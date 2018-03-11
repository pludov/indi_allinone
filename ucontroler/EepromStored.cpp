/*
 * EperomStored.cpp
 *
 *  Created on: 7 déc. 2017
 *      Author: ludovic
 */

#include <Arduino.h>
#include <EEPROM.h>

#include "Scheduler.h"
#include "EepromStored.h"
#include "IndiVectorMemberStorage.h"
#include "CommonUtils.h"

#define DEBUG_FINE(...)

#define FLAG_WRITTEN  0

class EepromWriteCounter : public EepromStored {
	friend class EepromStored;
	uint16_t count;
protected:
	virtual void decodeEepromValue(void * buffer, uint8_t sze)
	{
		if (sze < 2) {
			return;
		}
		count = *((uint16_t*)buffer);
	}

	virtual void encodeEepromValue(void * buffer, uint8_t sze)
	{
		*((uint16_t*)buffer) = count;
	}

	virtual int getEepromSize() const
	{
		return 2;
	}

public:
	uint8_t highestCellWriteCount;

	EepromWriteCounter() : EepromStored(Addr(63,1))
	{
		count = 0;
		highestCellWriteCount = 0;
	}

	virtual ~EepromWriteCounter() {}

	void inc(int level) {
		if (level <= highestCellWriteCount) {
			return;
		}
		DEBUG_FINE(F("Max write cell count is now at "), highestCellWriteCount);
		if (count < 0xffff) {
			count ++;
		} else {
			DEBUG(F("rewrite counter overflow"));
		}
		highestCellWriteCount = level;
		write();
		if (highestCellWriteCount == 255) {
			// Reset all the flags
			for(EepromStored * item = first; item; item = item->next)
			{
				item->writeCount = 0;
			}
		}
	}

	uint16_t getCount() const {
		return count;
	}

};


EepromStored * EepromStored::first = nullptr;

static uint32_t initValue = 0x1F04EAE4;
static EepromWriteCounter * writeCounter = 0;
static uint32_t byteWriteCount = 0;

#define check(t, dplay) while(!(t)) {DEBUG(dplay);}

static void checkAddr(uint8_t v)
{
	check(v < 64, F("Invalid address"));
}

uint32_t EepromStored::Addr(uint8_t v1)
{
	checkAddr(v1);
	return v1;
}

uint32_t EepromStored::Addr(uint32_t v1, uint8_t v2)
{
	checkAddr(v2);
	int used = 1;
	while(v1 >> (8 * used)) {
		used ++;
	}
	check(used < 4, F("Addr overflow"));
	return v1 | (((uint32_t)v2) << (8*used));
}

uint32_t EepromStored::Addr(uint32_t v1, uint8_t v2, uint8_t v3)
{
	return Addr(Addr(v1, v2), v3);
}

uint32_t EepromStored::Addr(uint32_t v1, uint8_t v2, uint8_t v3, uint8_t v4)
{
	return Addr(Addr(Addr(v1, v2), v3), v4);
}



EepromStored::EepromStored(uint32_t naddr) :addr(naddr), writeCount(0){
	if (getAddrFromHead(naddr) != naddr) {
		while(1) {
			DEBUG(F("Invalid address: #"), naddr);
			delay(1000);
		}

	}
	this->eepromPos = 0;
	this->next = first;
	first = this;
}

EepromStored::~EepromStored() {
}



void EepromStored::eRead(uint16_t pos, void * ptr, int count)
{
	uint8_t * v = (uint8_t*)ptr;
	for(int i = 0; i < count; ++i)
	{
		v[i] = EEPROM.read(pos + i);
	}
}

uint8_t EepromStored::eRead8(uint16_t pos)
{
	return EEPROM.read(pos);
}

uint32_t EepromStored::eRead32(uint16_t pos)
{
	// Read LSB
	uint32_t val = ((uint32_t)eRead8(pos))
				| (((uint32_t)eRead8(pos + 1)) << 8)
				| (((uint32_t)eRead8(pos + 2)) << 16)
				| (((uint32_t)eRead8(pos + 3)) << 24);

//	DEBUG(F("R("), pos, F(") = "), val);
	return val;
}

bool EepromStored::eWrite(uint16_t pos, void * ptr, int count)
{
	uint8_t * v = (uint8_t*)ptr;
	bool rslt = false;
	for(int i = 0; i < count; ++i)
	{
		if (eWrite8(pos + i, v[i])) {
			rslt = true;
		}
	}
	return rslt;
}

bool EepromStored::eWrite32(uint16_t pos, uint32_t val)
{
//	DEBUG(F("Write("), pos, F(")="), val);
	// Write LSB
	bool rslt = false;
	if (eWrite8(pos++, val & 255)) {
		rslt = true;
	}
	val = val >> 8;
	if (eWrite8(pos++, val & 255)) {
		rslt = true;
	}
	val = val >> 8;
	if (eWrite8(pos++, val & 255)) {
		rslt = true;
	}
	val = val >> 8;
	if (eWrite8(pos, val & 255)) {
		rslt = true;
	}
	return rslt;
}

bool EepromStored::eWrite8(uint16_t pos, uint8_t val)
{
	uint8_t cur = EEPROM.read(pos);
	if (cur != val) {
		EEPROM.write(pos, val);
		byteWriteCount++;
		Scheduler::instance().yield();
		return true;
	}
	return false;
}

uint8_t EepromStored::getSizeFromHead(uint32_t head)
{
	uint8_t size = 0;
	for(int i = 0; i < 4; ++i) {
		if (head & (((uint32_t)1) << (8 * i + 7))) {
			size |= (1 << i);
		}
	}
	for(int i = 0; i < 4; ++i) {
		if (head & (((uint32_t)1) << (8 * i + 6))) {
			size |= (1 << (i + 4));
		}
	}
	size ++;
	return size;
}

uint32_t EepromStored::getAddrFromHead(uint32_t head)
{
	return head & 0x3f3f3f3f;
}

uint32_t EepromStored::packHead(uint32_t addr, uint8_t size)
{

	uint32_t head = addr;
	size--;
	for(int i = 0; i < 4; ++i) {
		head |= ((uint32_t)((size >> i)&1)) << (8 * i + 7);
	}
	for(int i = 0; i < 4; ++i) {
		head |= ((uint32_t)((size >> (4 + i))&1)) << (8 * i + 6);
	}
	DEBUG_FINE(F("PackHead of"), addr, ',', size);
	DEBUG_FINE(F("=>"), head);
	return head;
}

uint16_t EepromStored::findPos(uint32_t addr, uint8_t & sze)
{
	for(int i = 4; i < EEPROM.length() - 4;)
	{
		uint8_t c = EEPROM.read(i);
		if (c == 0) {
			++i;
			continue;
		}
		// We have an adress
		uint32_t head = eRead32(i);
		sze = getSizeFromHead(head);
		if (getAddrFromHead(head) == addr) {
			return i;
		}
		i += 4;
		i += sze;
	}
	return 0;
}

bool EepromStored::writeValueToEeprom()
{
	bool rslt = false;
	// Write the addr + the actual size
	if (eWrite32(eepromPos, packHead(addr, getEffectiveEepromSize()))) {
		rslt = true;
	}

	int sze = getEffectiveEepromSize();
	char buffer[sze + 4]; // better safe than sorry
	for(int i = 0; i < sze; ++i) {
		buffer[i] = 0 ;
	}
	encodeEepromValue(buffer, sze);
	if (eWrite(eepromPos + 4, buffer, sze)) {
		rslt = true;
	}

	return rslt;
}

void EepromStored::readValueFromEeprom(uint8_t sze)
{
	if (sze == 0) sze = getEffectiveEepromSize();

	char buffer[sze + 4];
	eRead(eepromPos + 4, buffer, sze);
	decodeEepromValue(buffer, sze);
}

uint8_t EepromStored::getEffectiveEepromSize() const
{
	int v = getEepromSize();
	if (v < 1) return 1;
	if (v > 250) return 250;
	return v;
}

static uint32_t eepromCrc() {
	long t = micros();
	const uint32_t crc_table[16] = {
			0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
			0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
			0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
			0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
	};

	uint32_t crc = ~0L;

	for (int index = 0 ; index < EEPROM.length()  ; ++index) {
		crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
		crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
		crc = ~crc;
	}
	long elapsed = micros() - t;
	DEBUG(F("Eeprom read in "), elapsed);
	return crc;
}



void EepromStored::init()
{
	bool needRewrite = false;
	int requiredSize = 0;
	writeCounter = new EepromWriteCounter();

	// FIXME: better seed rand !
	srand(eepromCrc());

	uint32_t vInit;
	if ((vInit = eRead32(0)) != initValue) {
		DEBUG(F("EepromStored is new"), vInit);
		needRewrite = true;
	} else {
		for(EepromStored * item = first; item; item = item->next)
		{
			uint8_t sze;
			uint16_t p = findPos(item->addr, sze);
			if (!p)
			{
				DEBUG(F("EepromStored item is new "), item->addr);
				// Need
				needRewrite = true;
			} else {
				DEBUG(F("Found item #"), item->addr, F(" at "), p);
				item->eepromPos = p;
				item->readValueFromEeprom(sze);
				if (sze != item->getEffectiveEepromSize()) {
					DEBUG(F("Item size updated for "), item->addr, F(" from "), sze);
					needRewrite = true;
				}
			}
		}
	}
	DEBUG(F("writeCounter value is "), writeCounter->getCount());
	if ((!needRewrite) && (writeCounter->count > 16)) {
		DEBUG(F("Time for a full rewrite"));
		needRewrite = true;
	}
	if (needRewrite) {
		fullRewrite();
	}
	DEBUG(F("Write during init:"), byteWriteCount);
}

static int getPadding(int padding, int itemCount, int & paddingLeft)
{
	int pad = (((long long)rand()) * 2 * padding) / ((long long)itemCount * RAND_MAX);

	if (pad > paddingLeft) {
		pad = paddingLeft;
	}

	paddingLeft -= pad;
	return pad;
}

void EepromStored::write()
{
	if (eepromPos == 0) return;
	if (writeValueToEeprom() && this != writeCounter && writeCounter) {
		writeCount ++;
		writeCounter->inc(writeCount);
	}
}

void EepromStored::fullRewrite() {
	// Restart write counter
	writeCounter->count = 0;

	int itemCount = 0;
	int requiredSpace = 4;
	for(EepromStored * item = first; item; item = item->next)
	{
		itemCount ++;
		requiredSpace += 4;
		requiredSpace += item->getEffectiveEepromSize();
	}

	int padding = EEPROM.length() - requiredSpace;
	int paddingLeft = padding;

	EepromStored * items[itemCount];
	int i = 0;
	for(EepromStored * item = first; item; item = item->next)
	{
		items[i++] = item;
	}

	DEBUG(F("EepromStored will rewrite: "), itemCount , F(" items, for "), requiredSpace, " bytes");
	DEBUG(F("Padding is "), padding);
	while(requiredSpace > EEPROM.length()) {
		DEBUG(F("EEPROM overflow\n"));
	}
	eWrite8(rand() % 4, 0);
	uint16_t pos = 4;
	while(i > 0) {
		// Choose random item
		int selected = rand() % i;
		EepromStored * item = items[selected];
		items[selected] = items[i-1];
		i--;


		// Pad before
		for(int pad = getPadding(padding, 2 * itemCount, paddingLeft); pad > 0; --pad)
		{
			eWrite8(pos++, 0);
		}

		uint8_t sze = item->getEffectiveEepromSize();
		DEBUG(F("EepromStored rewrite : #"), item->addr , F(" "), sze, F("b at "), pos);

		// Write item
		item->eepromPos = pos;
		item->writeValueToEeprom();
		pos += 4;
		pos += sze;

		// Pad after
		for(int pad = getPadding(padding, 2 * itemCount, paddingLeft); pad > 0; --pad)
		{
			eWrite8(pos++, 0);
		}
	}

	while(pos < EEPROM.length()) {
		eWrite8(pos++, 0);
	}

	DEBUG_FINE("Reset original value at 0");
	// Done, reset the marker
	eWrite32(0, initValue);
}

