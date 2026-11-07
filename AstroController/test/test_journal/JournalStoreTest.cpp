#include <unity.h>
#include <string.h>
#include <list>
#include <algorithm>
#include <map>
#include <string>
#include "JournalStore.h"
#include "JournalStore.cpp"

enum SectorActionType {
    READ_SECTOR,
    WRITE_PAGE,
    RESET_SECTORS
};

class SectorAction {
public:
    SectorActionType type;
    int firstSectorNumber;
    int lastSectorNumber;
    int pageStart;
    int pageEnd;
    const uint8_t * sectorData;

    int sectorSize, pageSize;

    bool containsData(const uint8_t * data, int dataLen, int at = -1) const {
        if (sectorData == NULL) {
            return false;
        }
        for(int i = 0; i < dataLen; i++) {
            printf (" %02x ", data[i]);
            if (i % 16 == 15) {
                printf("\n");
            }
        }
        printf("\n");

        for(int i = 0; i < (pageEnd + 1) * pageSize; i++) {
            printf (" %02x ", sectorData[i]);
            if (i % 16 == 15) {
                printf("\n");
            }
        }
        printf("\n");

        for(int offset = 0; offset <= (pageEnd + 1) * pageSize - dataLen; offset++) {
            if (at != -1 && offset != at) {
                continue;
            }
            if (memcmp(sectorData + offset, data, dataLen) == 0) {
                printf("found %d at %d\n", dataLen, offset);
                return true;
            }
        }
        return false;
    }
};

class TestJournalEntry : public JournalEntry {
public:
    uint32_t addr;
    int length;
    uint8_t * data;
    bool clean = false;


    TestJournalEntry(uint32_t addr, int length, const uint8_t * data): JournalEntry() {
        this->addr = addr;
        this->length = length;
        this->data = new uint8_t[length];
        memcpy(this->data, data, length);
    }

    virtual ~TestJournalEntry() {
        delete[] data;
    }

    int initCount = 0;
    int initLength = 0;
    uint8_t * initData = nullptr;

    bool wasInitialized() const {
        return initData != nullptr;
    }

    virtual uint32_t get24BitsAddress() {
        return addr;
    }

    virtual int getLength() {
        return length;
    }

    virtual void getData(uint8_t * t) {
        memcpy(t, data, length);
    }

    virtual void setClean() {
        clean = true;
    }

    virtual void loadDataFrom(const uint8_t * data, int length) {
        if (initData != nullptr) {
            delete[] initData;
        }
        initCount++;
        initLength = length;
        initData = new uint8_t[length];
        memcpy(initData, data, length);
    }
};

class TestJournalStore : public JournalStore {
public:
    std::list<TestJournalEntry*> allEntries;
    
    int getFlashStartPtr() const {
        return flashStartPtr;
    }
    int getFlashWritePtr() const {
        return flashWritePtr;
    }

    TestJournalStore(int sectorCount, int sectorSize, int pageSize) : JournalStore(sectorCount, sectorSize, pageSize) {
    }

    virtual ~TestJournalStore() {
        // Free all sectorData
        for (auto it = sectorData.begin(); it != sectorData.end(); it++) {
            delete[] it->second;
        }
    }

    void setSectorData(int sectorNumber, const uint8_t * data, int dataSize) {
        auto it = sectorData.find(sectorNumber);
        if (it != sectorData.end()) {
            delete[] it->second;
        }
        sectorData[sectorNumber] = new uint8_t[sectorSize];
        memset(sectorData[sectorNumber], -1, sectorSize);
        memcpy(sectorData[sectorNumber], data, dataSize);
    }

    int readSectorCalled = 0;
    int readSectorSectorNumber = 0;

    std::map<int, uint8_t *> sectorData;

    virtual void readSector(int sectorNumber, uint8_t * sectorBuffer) {
        readSectorCalled++;
        readSectorSectorNumber = sectorNumber;
        auto it = sectorData.find(sectorNumber);
        if (it != sectorData.end()) {
            memcpy(sectorBuffer, it->second, sectorSize);
        } else {
            memset(sectorBuffer, -1, sectorSize);
        }
    }


    std::list<SectorAction> sectorActions;

    virtual void writePage(int sectorNumber, int pageStart, int pageEnd, const uint8_t * sectorData) {
        sectorActions.push_back(SectorAction{
            WRITE_PAGE,
            sectorNumber, sectorNumber,
            pageStart, pageEnd, sectorData,
            this->sectorSize, this->pageSize
        });
    }

    virtual void resetSectors(int sectorStart, int sectorEnd) {
        sectorActions.push_back(SectorAction{
            RESET_SECTORS,
            sectorStart, sectorEnd, 0, 0, NULL,
            this->sectorSize, this->pageSize
        });
    }
    
    virtual JournalEntry * lookupEntry(uint32_t addr) {
        auto it = std::find_if(this->allEntries.begin(), this->allEntries.end(), [addr](const TestJournalEntry * e) {
            return e->addr == addr;
        });
        if (it != this->allEntries.end()) {
            return (*it);
        } else {
            return nullptr;
        }
    }

    virtual JournalEntry * getFirstDirtyEntry() const {
        auto it = this->allEntries.begin();
        while(it != this->allEntries.end()) {
            if (!(*it)->clean) {
                return *it;
            }
            it++;
        }
        return nullptr;
    }

    virtual JournalEntry * getNextDirtyEntry(const JournalEntry * c) const {
        auto it = std::find(this->allEntries.begin(), this->allEntries.end(), c);
        if (it == this->allEntries.end()) {
            return nullptr;
        }
        it++;
        while(it != this->allEntries.end()) {
            if (!(*it)->clean) {
                return *it;
            }
            it++;
        }
        return nullptr;
    }

    virtual uint32_t getEntryAddress(const JournalEntry * c) const {
        return ((TestJournalEntry*)c)->addr;
    }

    virtual int getEntryLength(const JournalEntry * c) const {
        return ((TestJournalEntry*)c)->length;
    }

    virtual void markAllDirty() {
        for (auto it = this->allEntries.begin(); it != this->allEntries.end(); it++) {
            (*it)->clean = false;
        }        
    }
};

void setup() {
}

void teardown() {
}

void test_init_from_empty() {
    int totalSectorCount = 4;
    TestJournalStore js(totalSectorCount, 512, 64);
    auto entry = new TestJournalEntry(
            0x123456, 
            5, 
            (const uint8_t*)"\x01\x02\x03\x04\x05");
    
    auto secondEntry = new TestJournalEntry(
            0xBABEBD, 
            4, 
            (const uint8_t*)"\xe0\x0e\xb0\x0b");

    js.allEntries.push_back(entry);
    js.allEntries.push_back(secondEntry);
    
    js.initialize();
    TEST_ASSERT_EQUAL(0, js.sectorActions.size());

    js.step();
    TEST_ASSERT_EQUAL(1, js.sectorActions.size());
    TEST_ASSERT_EQUAL(RESET_SECTORS, js.sectorActions.front().type);
    TEST_ASSERT_EQUAL(0, js.sectorActions.front().firstSectorNumber);
    TEST_ASSERT_EQUAL(0, js.sectorActions.front().lastSectorNumber);
    
    js.sectorActions.pop_front();
    js.step();
    TEST_ASSERT_EQUAL(1, js.sectorActions.size());
    TEST_ASSERT_EQUAL(WRITE_PAGE, js.sectorActions.front().type);
    TEST_ASSERT_EQUAL(0, js.sectorActions.front().firstSectorNumber);
    TEST_ASSERT_EQUAL(0, js.sectorActions.front().pageEnd);
    TEST_ASSERT(js.sectorActions.front().containsData((const uint8_t*)"\x01\x02\x03\x04\x05", 5));
    TEST_ASSERT(js.sectorActions.front().containsData((const uint8_t*)"\x12\x34\x56\x05", 4));
    TEST_ASSERT(!js.sectorActions.front().containsData((const uint8_t*)"\x12\x34\x56\x05\x01\x02\x03\x04\x15", 9));

    TEST_ASSERT(js.sectorActions.front().containsData(secondEntry->data, secondEntry->length));

    js.sectorActions.pop_front();
    js.step();
    TEST_ASSERT_EQUAL_MESSAGE(0, js.sectorActions.size(), "No action after init");
    

    int currSector = 0;
    int startSector = 0;
    
    // Now change the entry, check it is written
    for(int i = 0; i < 0xC0; ++i) {
        int sectorEraseExpected = -1;

        js.sectorActions.clear();

        entry->data[0] = 0x10 + i;
        entry->clean = false;
        js.step();

        uint8_t v = (0x10 + i) & 0xff;
        const uint8_t expectedData [] = {v, 0x02, 0x03, 0x04, 0x05};


        if (i == 0x36 || i == 0x6E) {
            // New page, means closing the previous one,
            TEST_ASSERT_EQUAL(1, js.sectorActions.size());
            TEST_ASSERT_EQUAL(WRITE_PAGE, js.sectorActions.front().type);
            TEST_ASSERT_EQUAL_MESSAGE(currSector, js.sectorActions.front().firstSectorNumber, "Close sector 0");
            TEST_ASSERT_EQUAL(7, js.sectorActions.front().pageEnd);
            TEST_ASSERT_EQUAL_MESSAGE(js.sectorActions.front().sectorData[511], 0, "Sector is closing");
            
            currSector++;
            
            // Reset the next sector
            js.sectorActions.clear();
            js.step();
            TEST_ASSERT_EQUAL(1, js.sectorActions.size());
            TEST_ASSERT_EQUAL(RESET_SECTORS, js.sectorActions.front().type);
            TEST_ASSERT_EQUAL(currSector, js.sectorActions.front().firstSectorNumber);

            // Then write the new page
            js.sectorActions.clear();
            js.step();
            TEST_ASSERT_EQUAL(1, js.sectorActions.size());
            TEST_ASSERT_EQUAL(WRITE_PAGE, js.sectorActions.front().type);
            TEST_ASSERT_EQUAL(currSector, js.sectorActions.front().firstSectorNumber);
            // Verify the header is correct
            TEST_ASSERT(js.sectorActions.front().containsData((const uint8_t*)SECTOR_NEXT_MARKER, SECTOR_NEXT_MARKER_LENGTH, 0));
        } else if (i == 0xA6) {
            // There will be a compaction
            // Current sector is closed
            TEST_ASSERT_EQUAL(1, js.sectorActions.size());
            TEST_ASSERT_EQUAL(WRITE_PAGE, js.sectorActions.front().type);
            TEST_ASSERT_EQUAL_MESSAGE(currSector, js.sectorActions.front().firstSectorNumber, "Close sector 0");
            TEST_ASSERT_EQUAL(7, js.sectorActions.front().pageEnd);
            TEST_ASSERT_EQUAL_MESSAGE(js.sectorActions.front().sectorData[511], 0, "Sector is closing");
            currSector++;
            
            // Reset the next sector
            js.sectorActions.clear();
            js.step();
            TEST_ASSERT_EQUAL(1, js.sectorActions.size());
            TEST_ASSERT_EQUAL(RESET_SECTORS, js.sectorActions.front().type);
            TEST_ASSERT_EQUAL(currSector, js.sectorActions.front().firstSectorNumber);

            // Then write the new full page
            js.sectorActions.clear();
            js.step();
            TEST_ASSERT_EQUAL(1, js.sectorActions.size());
            TEST_ASSERT_EQUAL(WRITE_PAGE, js.sectorActions.front().type);
            TEST_ASSERT_EQUAL(currSector, js.sectorActions.front().firstSectorNumber);
            // Verify the header is correct
            TEST_ASSERT(js.sectorActions.front().containsData((const uint8_t*)SECTOR_HEAD_MARKER, SECTOR_HEAD_MARKER_LENGTH, 0));
            // Verify the second entry is also there
            TEST_ASSERT_MESSAGE(js.sectorActions.front().containsData(secondEntry->data, secondEntry->length), "Second entry is present on compaction");

            // There will be an erase right after the write
            sectorEraseExpected = startSector;
            startSector = currSector;
        }

        TEST_ASSERT_EQUAL(1, js.sectorActions.size());
        TEST_ASSERT_EQUAL(WRITE_PAGE, js.sectorActions.front().type);
        TEST_ASSERT_EQUAL(currSector, js.sectorActions.front().firstSectorNumber);
        TEST_ASSERT_MESSAGE(js.sectorActions.front().containsData(expectedData, 5), (std::string("Data is written at ") + std::to_string(i)).c_str());

        if (sectorEraseExpected != -1) {
            js.sectorActions.clear();
            js.step();
            TEST_ASSERT_EQUAL(1, js.sectorActions.size());
            TEST_ASSERT_EQUAL(RESET_SECTORS, js.sectorActions.front().type);
            TEST_ASSERT_EQUAL(sectorEraseExpected, js.sectorActions.front().firstSectorNumber);
        }
    }
}

#define EXISTING_ENTRY_DATA "\xbb\xcc\xdd\xee"
void test_add_new_entry_to_existing() {
    int totalSectorCount = 32;
    TestJournalStore js(totalSectorCount, 4096, 256);
    

    js.setSectorData(0, (uint8_t*)(
                    SECTOR_HEAD_MARKER
                    // A previous value
                    "\xA4\xB4\xC4\x04"
                    "\x0e\x0e\x0e\x0e"
                    // A new value
                    "\xA4\xB4\xC4\x04"
                    EXISTING_ENTRY_DATA)
                    ,
                    SECTOR_HEAD_MARKER_LENGTH + 4 + 4);

    auto entry = new TestJournalEntry(
            0x123456, 
            5, 
            (const uint8_t*)"\x01\x02\x03\x04\x05");

    js.allEntries.push_back(entry);

    auto existingEntry = new TestJournalEntry(
            0xA4B4C4, 
            4, 
            (const uint8_t*)EXISTING_ENTRY_DATA);

    js.allEntries.push_back(existingEntry);

    js.initialize();
    TEST_ASSERT_EQUAL_MESSAGE(0, js.sectorActions.size(), "No action on initialize");
    TEST_ASSERT_EQUAL_MESSAGE(0, js.getFlashStartPtr(), "Flash start ptr is 0");
    TEST_ASSERT_EQUAL_MESSAGE(13, js.getFlashWritePtr(), "Flash write ptr is after datas");
    TEST_ASSERT_EQUAL_MESSAGE(false, entry->wasInitialized(), "Entry was not initialized");
    TEST_ASSERT_EQUAL_MESSAGE(true, existingEntry->wasInitialized(), "Existing entry was initialized");
    TEST_ASSERT_EQUAL_MESSAGE(1, existingEntry->initCount, "Existing entry was initialized once");
    
    js.step();

    TEST_ASSERT_EQUAL_MESSAGE(1, js.sectorActions.size(), "Write of the missing entry");
    TEST_ASSERT_EQUAL_MESSAGE(13 + 9, js.getFlashWritePtr(), "Flash write ptr is after datas");
    TEST_ASSERT_EQUAL_MESSAGE(WRITE_PAGE, js.sectorActions.front().type, "Write of the missing entry");
    TEST_ASSERT_EQUAL_MESSAGE(0, js.sectorActions.front().firstSectorNumber, "Write of the missing entry");
    TEST_ASSERT_EQUAL_MESSAGE(0, js.sectorActions.front().pageEnd, "Write of the missing entry");
    TEST_ASSERT(js.sectorActions.front().containsData((const uint8_t*)"\x01\x02\x03\x04\x05", 5));
    TEST_ASSERT(js.sectorActions.front().containsData((const uint8_t*)"\x12\x34\x56\x05", 4));
    TEST_ASSERT(!js.sectorActions.front().containsData((const uint8_t*)"\x12\x34\x56\x05\x01\x02\x03\x04\x15", 9));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_init_from_empty);
    RUN_TEST(test_add_new_entry_to_existing);

    return UNITY_END();
}