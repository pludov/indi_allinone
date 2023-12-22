#ifndef JOURNALSTORE_H
#define JOURNALSTORE_H

#include <stdint.h>

#define SECTOR_HEAD_MARKER "\x00\xA1\xF9\xD8\xE0"
#define SECTOR_HEAD_MARKER_LENGTH 5
#define SECTOR_NEXT_MARKER "\x19\x80"
#define SECTOR_NEXT_MARKER_LENGTH 2 


class JournalEntry {
    friend class JournalStore;
    uint32_t lastFlashPos;
public:
    JournalEntry();
    virtual ~JournalEntry() = 0;

    virtual uint32_t get24BitsAddress() = 0;
    virtual int getLength() = 0;
    virtual void getData(uint8_t * t) = 0;

    virtual void setClean() = 0;

    virtual void loadDataFrom(const uint8_t * data, int length) = 0;
};

// This journal assume:
//  - All data fits into one sector
class JournalStore {
private:
    void reset();
    // Write as much as possible entries to the current sector
    // force: write from start of sector, even if no data is available (will write unless sector has no header)
    // Return false if nothing was written (no entry, or not enough space in the current sector)
    bool writeDirtyToCurrentSector(bool force);

    // Locate entries from current flashWritePtr value
    void findExistingEntries();
    
    // Locate entries from current flashWritePtr value, but only from the current sector
    // This assume sectorBuffer is filled with the current sector
    void findExistingEntriesFromCurrentSector();

    void readExistingEntries();

    void startCompaction();
protected:
    int sectorCount;
    int previousHeadSector = -1;
    int lastSectorErased;
    int sectorSize;
    int pageSize;

    uint8_t * sectorBuffer;

    // Offset of the first sector part of the journal
    uint32_t flashStartPtr = 0xffffffff;
    // Actual position of the next write in flash
    uint32_t flashWritePtr = 0;

    inline uint32_t flashLength() const {
        return sectorCount * sectorSize;
    }
    inline uint32_t usedLength() const {
        return (flashLength() + flashWritePtr - flashStartPtr) % flashLength();
    }

    // When renewing, all sectors are marked as to be erased.
    // This is prioritarized over new writes.
    uint16_t sectorLeftToErase = 0;

    // Read from a sector. Blocking
    virtual void readSector(int sectorNumber, uint8_t * sectorBuffer) = 0;
    // Write to a sector, from pageStart to pageEnd (inclusive). Can be asynchronous.
    // sectorData is for the full sector (points to page 0).
    virtual void writePage(int sectorNumber, int pageStart, int pageEnd, const uint8_t * sectorData) = 0;
    // Reset from sectorStart to sectorEnd (inclusive). Can be asynchronous
    virtual void resetSectors(int sectorStart, int sectorEnd) = 0;
    
    // Used when an entry is discovered at startup
    virtual JournalEntry * lookupEntry(uint32_t addr) = 0;

    // Mark all entries as dirty
    virtual void markAllDirty() = 0;

    virtual JournalEntry * getFirstDirtyEntry() const = 0;
    virtual JournalEntry * getNextDirtyEntry(const JournalEntry * from) const = 0;
public:
    JournalStore(int sectorCount, int sectorSize, int pageSize);
    virtual ~JournalStore();

    // While call readSector and initialize state
    void initialize();

    // This is to be called when the workqueue is complete (writePage, resetSectors)
    // This will iterate for dirty items and write them if required
    // If no work is generated, no need to call step again until dirty items are added
    void step();
};

#endif // JOURNALSTORE_H
