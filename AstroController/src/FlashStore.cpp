#include <string.h>

extern "C" {
  #include <pico/bootrom.h>
  #include <hardware/sync.h>
  #include <hardware/flash.h>
};

#include "EepromStored.h"
#include "CommonUtils.h"

FlashStore * FlashStore::instance;

FlashStore::FlashStore(int sectorCount, bool def): 
    JournalStore(sectorCount, FLASH_SECTOR_SIZE, FLASH_PAGE_SIZE),
    Scheduled(F("FlashStore")),
    group(F("Flash status")),
    flashStatusVec(group, F("FLASH_STATUS"), F("Flash status")),
    flashSize(&flashStatusVec, F("FLASH_SIZE"), F("Flash size"),0 ,0x100000, 1),
    flashErrorCount(&flashStatusVec, F("FLASH_ERROR_COUNT"), F("Flash error count"),0 ,0xffff, 1),
    flashCompactVec(group, F("FLASH_COMPACT"), F("Flash compact"), VECTOR_WRITABLE|VECTOR_READABLE|VECTOR_SWITCH_MANY),
    flashCompact(&flashCompactVec, F("FLASH_COMPACT"), F("Flash compact"))
{

    firstSector = PICO_FLASH_SIZE_BYTES / FLASH_SECTOR_SIZE - sectorCount;
    first = nullptr;
    firstDirty = lastDirty = nullptr;

    flashErrorCount.setValue(0);
    flashSize.setValue(0);
    // TODO: Verify we run bynary_type=copy_to_ram
    this->priority = 1;
    if (def) {
        FlashStore::instance = this;
    }
    flashCompactVec.onRequested(VectorCallback(&FlashStore::onIndiCompactRequest, this));
}

void FlashStore::onIndiCompactRequest() {
    // Reset error count
    this->flashErrorCount.setValue(0);
    requestCompaction();
    this->schedule();
}

void FlashStore::readSector(int sectorNumber, uint8_t * sectorBuffer) {
    uint8_t * flashPtr ((uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + (firstSector + sectorNumber) * sectorSize));
    memcpy(sectorBuffer, flashPtr, sectorSize);
}

void FlashStore::writePage(int sectorNumber, int pageStart, int pageEnd, const uint8_t * sectorData) {

    this->flashOperation.schedule(
        [this, sectorNumber, pageStart, pageEnd, sectorData]() {
            flash_range_program((firstSector + sectorNumber) * sectorSize
                         + pageStart * pageSize,
                         sectorData + pageStart * pageSize,
                         (pageEnd - pageStart + 1) * pageSize);
            // Compare the memory to report errors
            uint8_t * flashPtr ((uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + (firstSector + sectorNumber) * sectorSize
                         + pageStart * pageSize));
            uint8_t * dataPtr = (uint8_t *)sectorData + pageStart * pageSize;
            bool ret =  true;
            for(int i = 0; i < (pageEnd - pageStart + 1) * pageSize; ++i) {
                if (flashPtr[i] != dataPtr[i]) {
                    DEBUG("Flash write error: ", sectorNumber, " at ", i, " = ", flashPtr[i], " != ", dataPtr[i]);
                    ret = false;
                }
            }
            return ret;
        },
        [this, sectorNumber, pageStart, pageEnd, sectorData](bool ret) {
            if (!ret) {
                this->flashErrorCount.setValue(this->flashErrorCount.getValue() + 1);
            }
            this->nextTick = UTime::now();
            this->tickExpectedDuration = US(500);
        },
        MS(10 * (pageEnd - pageStart + 1))
    );
}

void FlashStore::resetSectors(int sectorStart, int sectorEnd) {
    this->flashOperation.schedule(
        [this, sectorStart, sectorEnd]() {
            flash_range_erase((firstSector + sectorStart) * sectorSize,
                             (sectorEnd - sectorStart + 1) * sectorSize);
            // Verify the erase, all must be 0xff
            uint8_t * flashPtr ((uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + (firstSector + sectorStart) * sectorSize));
            bool ret = true;
            for(int i = 0; i < (sectorEnd - sectorStart + 1) * sectorSize; ++i) {
                if (flashPtr[i] != 0xff) {
                    DEBUG("Flash erase error: ", sectorStart, " at ", i, " = ", flashPtr[i]);
                    ret = false;
                    break;
                }
            }
            return ret;
        },
        [this, sectorStart, sectorEnd](bool ret) {
            if (!ret) {
                this->flashErrorCount.setValue(this->flashErrorCount.getValue() + 1);
            }
            this->nextTick = UTime::now();
            this->tickExpectedDuration = US(500);
        },
        MS(40 * (sectorEnd - sectorStart + 1))
    );
}

JournalEntry * FlashStore::lookupEntry(uint32_t addr) {
    for(EepromStored * e = first; e; e = e->next) {
        if (e->get24BitsAddress() == addr) {
            return e;
        }
    }

    this->flashErrorCount.setValue(this->flashErrorCount.getValue() + 1);
    return nullptr;
}

void FlashStore::onCorruptedSector(int sectorNumber) {
    DEBUG("Corrupted sector: ", sectorNumber);
    this->flashErrorCount.setValue(this->flashErrorCount.getValue() + 1);
}

void FlashStore::markAllDirty() {
    for(EepromStored * e = first; e; e = e->next) {
        e->setDirtyness(true);
    }
}

JournalEntry * FlashStore::getFirstDirtyEntry() const {
    return firstDirty;
}

JournalEntry * FlashStore::getNextDirtyEntry(const JournalEntry * from) const {
    return ((EepromStored *)from)->nextDirty;
}

void FlashStore::initialize() {
    int entryCount = 0;
    for(EepromStored * e = first; e; e = e->next) {
        ++entryCount;
    }
    DEBUG("Initialize storage with entries:", entryCount);
    JournalStore::initialize();
    EepromReadyListener::ready();
    this->flashSize.setValue(usedLength());
    this->nextTick = UTime::now();
    this->tickExpectedDuration = US(1300);
}

void FlashStore::tick() {
    if (flashOperation.isPending()) {
        return;
    }

    DEBUG("Flash store stepping");
    this->nextTick = UTime::never();
    JournalStore::step();
    this->flashSize.setValue(usedLength());
}

void FlashStore::schedule() {
    // Make sure write will occur.
    if (this->nextTick.isNever()) {
        // Take some time to aggregate multiple writes
        this->nextTick = UTime::now() + MS(100);
        this->tickExpectedDuration = US(1300);
    }
}