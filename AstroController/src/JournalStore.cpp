#include <string.h>
#include "JournalStore.h"

#include <stdio.h>


#ifndef ARDUINO_ARCH_RP2040
#define DEBUG(...)
#else
#include <Arduino.h>
#include "CommonUtils.h"
#endif

// Each sector contains data.
// The first byte of a sectore indicates if the sector is head or data.
// This byte is accounted in the flashWritePtr counter.
#define SECTOR_HEAD 1
#define SECTOR_DATA 0

// Then the sector is assumed to be filled with len, addr, data or 0xff.

#define FLASH_PTR ((uint8_t *)(XIP_BASE + FLASH_START))

// This is an entry with invalid address.
static const uint8_t headMarker[SECTOR_HEAD_MARKER_LENGTH + 1] = SECTOR_HEAD_MARKER;
static const uint8_t continueMarker[SECTOR_NEXT_MARKER_LENGTH + 1] = SECTOR_NEXT_MARKER;

JournalStore::JournalStore(int sectorCount, int sectorSize, int pageSize) {
    this->sectorCount = sectorCount;
    this->sectorSize = sectorSize;
    this->pageSize = pageSize;
    this->sectorBuffer = new uint8_t[sectorSize];
    this->forceCompaction = false;
}

JournalStore::~JournalStore() {
    delete[] this->sectorBuffer;
}

void JournalStore::reset() {
    DEBUG("Resetting flash");
    flashStartPtr = 0;
    flashWritePtr = 0;
    previousHeadSector = -1;
}

void JournalStore::requestCompaction() {
    forceCompaction = true;
}

bool JournalStore::writeDirtyToCurrentSector(bool force) {
    int sectorWriteOffset = this->flashWritePtr % this->sectorSize;

    int startingOffset = force ? 0 : sectorWriteOffset;
    bool overflow = false;


    JournalEntry * c = getFirstDirtyEntry();
    while(c != nullptr && sectorWriteOffset < this->sectorSize - 1) {
        uint32_t addr = c->get24BitsAddress();
        int len = c->getLength();
        int entryTotalLength = 4 + len;
        if (sectorWriteOffset + entryTotalLength <= this->sectorSize - 1) {
            // This entry fits

            // Write the length & the address

            // FIXME: how can we make this atomic ?
            // This needs two write operations that don't include length
            this->sectorBuffer[sectorWriteOffset] = (addr >> 16) & 0xff;
            this->sectorBuffer[sectorWriteOffset + 1] = (addr >> 8) & 0xff;
            this->sectorBuffer[sectorWriteOffset + 2] = (addr) & 0xff;
            this->sectorBuffer[sectorWriteOffset + 3] = len;
            
            c->getData(this->sectorBuffer + sectorWriteOffset + 4);
            
            sectorWriteOffset += entryTotalLength;
            this->flashWritePtr += entryTotalLength;
            
            auto next = getNextDirtyEntry(c);
            c->setClean();
            c = next;
        } else {
            c = getNextDirtyEntry(c);
            overflow = true;
        }
    }

    // Close the sector as soon as something cannot fit
    if (overflow) {
        this->sectorBuffer[this->sectorSize - 1] = 0x00;
        sectorWriteOffset = this->sectorSize;
        auto sectorId = this->flashWritePtr / this->sectorSize;
        auto nextSectorId = (sectorId + 1) % this->sectorCount;
        this->flashWritePtr = nextSectorId * this->sectorSize;
    }

    if (sectorWriteOffset != startingOffset) {
        // Write the sector header
        writePage((this->flashWritePtr - 1) / this->sectorSize, 
                    startingOffset / this->pageSize, 
                    (sectorWriteOffset - 1) / this->pageSize, 
                    this->sectorBuffer);
        return true;
    } else {
        return false;
    }
}

void JournalStore::findExistingEntriesFromCurrentSector() {
    
    int offset = this->flashWritePtr % this->sectorSize;
    while(offset + 4 <= this->sectorSize) {
        DEBUG("Searching at offset ", offset);
        // Check the len is not -1
        int len = this->sectorBuffer[offset + 3];
        if (len == 0xff) {
            // This is the end of the sector's content
            break;
        }
        DEBUG("Found len ", len, " at offset ", offset);

        if (offset + 4 + len > this->sectorSize) {
            // bug in storage :-(
            // Stop for that sector
            DEBUG("Invalid sector content");
            onCorruptedSector(flashWritePtr / this->sectorSize);
            break;
        }
        // Get the address
        uint32_t addr = this->sectorBuffer[offset] << 16;
        addr |= this->sectorBuffer[offset + 1] << 8;
        addr |= this->sectorBuffer[offset + 2];

        DEBUG("Found ", addr, " at offset ", offset);
        auto entry = lookupEntry(addr);
        if (entry != nullptr) {
            entry->lastFlashPos = this->flashWritePtr;
        } else {
            DEBUG("Found unknown entry: %#x\n", addr);
        }

        offset += 4 + len;
        flashWritePtr += 4 + len;
    }
}

void JournalStore::startCompaction() {
    int sectorId = this->flashWritePtr / this->sectorSize;
    previousHeadSector = sectorId;
    sectorId = (sectorId + 1) % sectorCount;

    flashStartPtr = sectorId * this->sectorSize;
    flashWritePtr = flashStartPtr;
}

void JournalStore::findExistingEntries() {
    int firstSector = this->flashStartPtr / this->sectorSize;
    while(true) {
        findExistingEntriesFromCurrentSector();
        // Check if current sector is closed
        if (this->sectorBuffer[this->sectorSize - 1] != 0x00) {
            // This sector is not closed. We're done
            break;
        }

        int sectorId = this->flashWritePtr / this->sectorSize;

        int nextSector = (sectorId + 1) % sectorCount;
        if (nextSector == firstSector) {
            // We are done... The flash is full, no more empty sector.
            // Next step will start compaction
            startCompaction();
            break;
        }
        readSector(nextSector, sectorBuffer);
        flashWritePtr = nextSector * this->sectorSize;
        if (memcmp(sectorBuffer, continueMarker, SECTOR_NEXT_MARKER_LENGTH)) {
            // This is not a continue marker, we are done
            break;
        }
        flashWritePtr += SECTOR_NEXT_MARKER_LENGTH;
    }
}

void JournalStore::readExistingEntries() {
    // Count the existing entries (they have a lastFlashPos)
    int count = 0;
    for (auto e = getFirstDirtyEntry(); e != nullptr; e = getNextDirtyEntry(e)) {
        if (e->lastFlashPos != 0xffffffff) {
            DEBUG("Flash item ", e->get24BitsAddress(), " found at: ", e->lastFlashPos);
            count++;
        }
    }
    DEBUG("Found ", count, " entries");

    // Create an array of pointers to entries
    JournalEntry * entries[count];
    int i = 0;
    for (auto e = getFirstDirtyEntry(); e != nullptr; e = getNextDirtyEntry(e)) {
        if (e->lastFlashPos != 0xffffffff) {
            entries[i++] = e;
        }
    }

    // Sort them by lastFlashPos
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (entries[i]->lastFlashPos > entries[j]->lastFlashPos) {
                auto tmp = entries[i];
                entries[i] = entries[j];
                entries[j] = tmp;
            }
        }
    }

    int loadedSector = this->flashWritePtr / this->sectorSize;
    for(int i = 0; i < count; ++i) {
        auto e = entries[i];
        int sectorId = e->lastFlashPos / this->sectorSize;
        if (sectorId != loadedSector) {
            readSector(sectorId, sectorBuffer);
            loadedSector = sectorId;
        }
        int offset = e->lastFlashPos % this->sectorSize;
        int len = this->sectorBuffer[offset + 3];
        e->loadDataFrom(this->sectorBuffer + offset + 4, len);
        e->setClean();
    }

    // Now load the last sector
    if (loadedSector != this->flashWritePtr / this->sectorSize) {
        readSector(this->flashWritePtr / this->sectorSize, sectorBuffer);
    }
}

void JournalStore::initialize() {
    // This will init the flash if nothing is found
    lastSectorErased = -1;
    flashStartPtr = 0;
    while(flashStartPtr < this->flashLength()) {
        readSector(flashStartPtr / this->sectorSize, sectorBuffer);
        if (!memcmp(sectorBuffer, headMarker, SECTOR_HEAD_MARKER_LENGTH)) {
            break;
        }
        flashStartPtr += this->sectorSize;
    }

    markAllDirty();
    // clean lastFlashPos
    for (auto e = getFirstDirtyEntry(); e != nullptr; e = getNextDirtyEntry(e)) {
        e->lastFlashPos = 0xffffffff;
    }

    if (flashStartPtr >= this->flashLength()) {
        reset();
    } else {
        flashWritePtr = flashStartPtr;
        flashWritePtr += SECTOR_HEAD_MARKER_LENGTH;
        findExistingEntries();
        readExistingEntries();
    }
}

void JournalStore::step() {
    if (forceCompaction || flashWritePtr % this->sectorSize == 0) {
        
        // Advance to the next sector boundary
        flashWritePtr += (this->sectorSize - (flashWritePtr % this->sectorSize)) % this->sectorSize;

        // We are at the beginning of a sector
        // Check if we need to erase it
        int sectorId = this->flashWritePtr / this->sectorSize;

        if (lastSectorErased != sectorId) {
            DEBUG("Reset new sector");
            // We need to erase this sector
            resetSectors(sectorId, sectorId);
            lastSectorErased = sectorId;
            return;
        }
        // Now start the sector
        memset(this->sectorBuffer, 0xff, this->sectorSize);
        int firstSectorId = this->flashStartPtr / this->sectorSize;
        int prefixLen;
        if (sectorId == firstSectorId && !forceCompaction) {
            DEBUG("Start first sector");
            // This is a brand new sector
            // Write the marker
            memcpy(this->sectorBuffer, headMarker, SECTOR_HEAD_MARKER_LENGTH);
            prefixLen = SECTOR_HEAD_MARKER_LENGTH;
        } else if (forceCompaction || ((sectorId + 1) % this->sectorCount == firstSectorId)) {
            DEBUG("Start compaction sector");
            
            // Need to erase all & compact!
            memcpy(this->sectorBuffer, headMarker, SECTOR_HEAD_MARKER_LENGTH);
            prefixLen = SECTOR_HEAD_MARKER_LENGTH;
            // Do a full rewrite
            markAllDirty();
            // Ensure we erase the previous head right after
            previousHeadSector = firstSectorId;
            forceCompaction = false;
        } else {
            DEBUG("Start continue sector");
            
            memcpy(this->sectorBuffer, continueMarker, SECTOR_NEXT_MARKER_LENGTH);
            prefixLen = SECTOR_NEXT_MARKER_LENGTH;
        }

        this->flashWritePtr += prefixLen;
        writeDirtyToCurrentSector(true);
        return;
    }

    // Not at the start of a sector.
    if (previousHeadSector != -1) {
        DEBUG("Erasing previous head sector");
        // There was a reset, we need to erase previous head marker
        resetSectors(previousHeadSector, previousHeadSector);
        previousHeadSector = -1;
        return;
    }

    writeDirtyToCurrentSector(false);
}

JournalEntry::JournalEntry() {
}

JournalEntry::~JournalEntry() {
}