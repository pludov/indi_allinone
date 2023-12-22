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
    flashErrorCount(&flashStatusVec, F("FLASH_ERROR_COUNT"), F("Flash error count"),0 ,0xffff, 1)
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
}

void FlashStore::readSector(int sectorNumber, uint8_t * sectorBuffer) {
    uint8_t * flashPtr ((uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + (firstSector + sectorNumber) * sectorSize));
    memcpy(sectorBuffer, flashPtr, sectorSize);
}

bool FlashStore::hasPendingOperation() {
    return pendingOperation;
}

void FlashStore::doPendingOperation() {
//    uint32_t ints = save_and_disable_interrupts();
    if (pendingSectorData) {
        flash_range_program((firstSector + pendingSectorStart) * sectorSize
                         + pendingPageStart * pageSize,
                         pendingSectorData + pendingPageStart * pageSize,
                         (pendingPageEnd - pendingPageStart + 1) * pageSize);
        // Compare the memory to report errors
        uint8_t * flashPtr ((uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + (firstSector + pendingSectorStart) * sectorSize
                         + pendingPageStart * pageSize));
        uint8_t * dataPtr = (uint8_t *)pendingSectorData + pendingPageStart * pageSize;
        for(int i = 0; i < (pendingPageEnd - pendingPageStart + 1) * pageSize; ++i) {
            if (flashPtr[i] != dataPtr[i]) {
                DEBUG("Flash write error: ", pendingSectorStart, " at ", i, " = ", flashPtr[i], " != ", dataPtr[i]);
                flashErrorCount.setValue(flashErrorCount.getValue() + 1);
            }
        }
    } else {
        flash_range_erase((firstSector + pendingSectorStart) * sectorSize,
                             (pendingSectorEnd - pendingSectorStart + 1) * sectorSize);
        // Verify the erase, all must be 0xff
        uint8_t * flashPtr ((uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + (firstSector + pendingSectorStart) * sectorSize));
        for(int i = 0; i < (pendingSectorEnd - pendingSectorStart + 1) * sectorSize; ++i) {
            if (flashPtr[i] != 0xff) {
                DEBUG("Flash erase error: ", pendingSectorStart, " at ", i, " = ", flashPtr[i]);
                flashErrorCount.setValue(flashErrorCount.getValue() + 1);
                break;
            }
        }
    }
//    restore_interrupts (ints);
    pendingOperation = false;
}

void FlashStore::writePage(int sectorNumber, int pageStart, int pageEnd, const uint8_t * sectorData) {
    pendingSectorStart = sectorNumber;
    pendingSectorEnd = sectorNumber;
    pendingPageStart = pageStart;
    pendingPageEnd = pageEnd;
    pendingSectorData = sectorData;
    pendingOperation = true;
}

void FlashStore::resetSectors(int sectorStart, int sectorEnd) {
    pendingSectorStart = sectorStart;
    pendingSectorEnd = sectorEnd;
    pendingPageStart = 0;
    pendingPageEnd = 0;
    pendingSectorData = nullptr;
    pendingOperation = true;
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
    if (hasPendingOperation()) {
        DEBUG("Doing pending operations");
        doPendingOperation();
        DEBUG("Done with pending operations");
        // Will recall ASAP
        this->nextTick = UTime::now();
        this->tickExpectedDuration = US(1300);
    } else {
        DEBUG("Flash store stepping");
        JournalStore::step();
        this->flashSize.setValue(usedLength());
        DEBUG("Flash store done: ", hasPendingOperation());
        if (hasPendingOperation()) {
            // some work was scheduled, do it
            this->nextTick = UTime::now() + MS(1);
            this->tickExpectedDuration = MS(40);
        } else {
            DEBUG("Flash idle. Write ptr: ", (this->flashWritePtr - this->flashStartPtr) % this->flashLength());
            this->nextTick = UTime::never();
        }
    }
}

void FlashStore::schedule() {
    // Make sure write will occur.
    if (this->nextTick.isNever()) {
        // Take some time to aggregate multiple writes
        this->nextTick = UTime::now() + MS(100);
        this->tickExpectedDuration = US(1300);
    }
}