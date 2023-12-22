/*
 * EperomStored.h
 *
 *  Created on: 7 déc. 2017
 *      Author: ludovic
 */

#ifndef EEPROMSTORED_H_
#define EEPROMSTORED_H_

#include <stdint.h>

/**
 * Represent something that is stored within eeprom
 *
 * During init, the eeprom may be reallocated
 *
 * addrs are in the range : 1-127.1-127.1-127.1-127
 * 4 bits for size leads to 1=>16 chars
 *
 */

#ifdef ARDUINO_ARCH_RP2040

#include "JournalStore.h"
#include "Scheduled.h"

class EepromStored;

// This is a singleton class to address the store
class FlashStore: protected JournalStore, protected Scheduled {
	friend class EepromStored;
	
	int firstSector;


	bool pendingOperation;
	int pendingSectorStart;
	int pendingSectorEnd;
	int pendingPageStart;
	int pendingPageEnd;
	// This decide reset vs write
	const uint8_t * pendingSectorData;

	bool hasPendingOperation();
	void doPendingOperation();


	void schedule();
public:
	FlashStore(int sectorCount, bool def = true);

	void initialize();

protected:
	EepromStored * first;
	EepromStored * firstDirty, * lastDirty;

    virtual void readSector(int sectorNumber, uint8_t * sectorBuffer);
    // Write to a sector, from pageStart to pageEnd (inclusive). Can be asynchronous.
    // sectorData is for the full sector (points to page 0).
    virtual void writePage(int sectorNumber, int pageStart, int pageEnd, const uint8_t * sectorData);
    // Reset from sectorStart to sectorEnd (inclusive). Can be asynchronous
    virtual void resetSectors(int sectorStart, int sectorEnd);
    
    // Used when an entry is discovered at startup
    virtual JournalEntry * lookupEntry(uint32_t addr);

    // Mark all entries as dirty
    virtual void markAllDirty();

    virtual JournalEntry * getFirstDirtyEntry() const;
    virtual JournalEntry * getNextDirtyEntry(const JournalEntry * from) const;


	virtual void tick();
public:
	static FlashStore * instance;

};


class EepromStored: private JournalEntry {
	friend class FlashStore;
	FlashStore * root;
	EepromStored * nextDirty, * prevDirty;
	EepromStored * next;
	const uint32_t addr;
	bool dirty;

	void setDirtyness(bool dirty);
    virtual uint32_t get24BitsAddress();
    virtual int getLength();
    virtual void getData(uint8_t * t);

    virtual void setClean();

    virtual void loadDataFrom(const uint8_t * data, int length);


protected:
	virtual void decodeEepromValue(void * buffer, uint8_t sze) = 0;
	virtual void encodeEepromValue(void * buffer, uint8_t sze) = 0;

	// Size required in byte
	virtual int getEepromSize() const = 0;

	void write();
public:
	EepromStored(uint32_t addr);
	virtual ~EepromStored();

	// Addr are in the range 0..63
	static uint32_t Addr(uint8_t v);
	static uint32_t Addr(uint32_t v1, uint8_t v2);
	static uint32_t Addr(uint32_t v1, uint8_t v2, uint8_t v3);
	static uint32_t Addr(uint32_t v1, uint8_t v2, uint8_t v3, uint8_t v4);
};


#else

class EepromWriteCounter;

class EepromStored {
	friend class EepromWriteCounter;
private:
	static bool initDone;
	EepromStored * next;
	static EepromStored * first;
	const uint32_t addr;

#ifdef ARDUINO_ARCH_RP2040
	// Offset from the end of the eeprom.
	// 0 means not initialized
	bool dirty;
	static EepromStored * firstDirty, * lastDirty;
	EepromStored * nextDirty, * prevDirty;

	static void loop();
	static void compact();
	static void dispatchValue(uint32_t addr, uint8_t * buffer, uint8_t sze);
	// 0 means not busy
	// 1 about to write data
	// 2 about to erase data (longer)
	static int busyLevel();
#else
	uint16_t eepromPos;
	uint8_t writeCount;

	uint8_t getEffectiveEepromSize() const;

	bool writeValueToEeprom();
	void readValueFromEeprom(uint8_t sze = 0);

	// size is 1->16
	static uint8_t getSizeFromHead(uint32_t addr);
	static uint32_t getAddrFromHead(uint32_t head);
	static uint32_t packHead(uint32_t addr, uint8_t size);

	static uint32_t eRead32(uint16_t pos);
	static uint8_t eRead8(uint16_t pos);
	static void eRead(uint16_t pos, void * ptr, int count);

	static bool eWrite32(uint16_t pos, uint32_t val);
	static bool eWrite8(uint16_t pos, uint8_t val);
	static bool eWrite(uint16_t pos, void * val, int count);

	// 0 means not found
	static uint16_t findPos(uint32_t addr, uint8_t & sze);

	static void fullRewrite();
#endif
protected:
	static void rewrite();

	virtual void decodeEepromValue(void * buffer, uint8_t sze) = 0;
	virtual void encodeEepromValue(void * buffer, uint8_t sze) = 0;

	// Size required in byte
	virtual int getEepromSize() const = 0;

	void write();
public:
	EepromStored(uint32_t addr);
	virtual ~EepromStored();

	static void init();

	static bool eepromReady() {
		return initDone;
	}

	// Addr are in the range 0..63
	static uint32_t Addr(uint8_t v);
	static uint32_t Addr(uint32_t v1, uint8_t v2);
	static uint32_t Addr(uint32_t v1, uint8_t v2, uint8_t v3);
	static uint32_t Addr(uint32_t v1, uint8_t v2, uint8_t v3, uint8_t v4);
};


#endif

class EepromCallback {
	typedef void (FuncType)(void*);
	FuncType * func;
	void *thiz;
public:
	EepromCallback() {
		thiz = nullptr;
		func = nullptr;
	}

	EepromCallback(const EepromCallback & copy) {
		thiz = copy.thiz;
		func = copy.func;
	}

	template<typename O>
	EepromCallback(void (O::*func)(), O * thiz) {
		this->thiz = (void*)thiz;
		this->func = (FuncType*)func;
	}

	bool isSet() const {
		return func;
	}

	void call() const{
		(*func)(thiz);
	}
};

class EepromReadyListener {
	friend class EepromStored;
	friend class FlashStore;
private:
	static EepromReadyListener * first;
	static bool readyStatus;
	static void ready();
	EepromReadyListener * next;
	EepromCallback callback;
public:
	EepromReadyListener(const EepromCallback & cb);
	~EepromReadyListener();
	void setCallback(const EepromCallback & cb);
	static bool isReady();
};

#endif /* EEPROMSTORED_H_ */
