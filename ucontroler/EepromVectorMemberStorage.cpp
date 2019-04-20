/*
 * EepromVectorMemberStorage.cpp
 *
 *  Created on: 9 déc. 2017
 *      Author: ludovic
 */
#include <Arduino.h>


#include "EepromStored.h"
#include "IndiVectorMember.h"
#include "IndiVectorMemberStorage.h"

#include "IndiFloatVectorMember.h"
#include "IndiIntVectorMember.h"
#include "IndiTextVectorMember.h"


class EepromFloatStorage: public EepromStored, public IndiVectorMemberStorage {
	IndiFloatVectorMember * member;
public:
	EepromFloatStorage(IndiFloatVectorMember * member, uint32_t addr):
			EepromStored::EepromStored(addr),
			member(member)
	{

	}

	virtual ~EepromFloatStorage() {}

	virtual void decodeEepromValue(void * buffer, uint8_t sze) {
		if (sze < sizeof(double)) {
			return;
		}
		double v = *((double*)buffer);
		member->setValue(v);
	}

	virtual void encodeEepromValue(void * buffer, uint8_t sze) {
		if (sze < sizeof(double)) {
			return;
		}
		*((double*)buffer) = member->getDoubleValue();
	}

	virtual int getEepromSize() const {
		return sizeof(double);
	}

	virtual void save() {
		write();
	}
};

class EepromIntStorage: public EepromStored, public IndiVectorMemberStorage {
	IndiIntVectorMember * member;
public:
	EepromIntStorage(IndiIntVectorMember * member, uint32_t addr):
			EepromStored::EepromStored(addr),
			member(member)
	{

	}

	virtual ~EepromIntStorage() {}

	virtual void decodeEepromValue(void * buffer, uint8_t sze) {
		if (sze < sizeof(int32_t)) {
			return;
		}
		int32_t v = *((int32_t*)buffer);
		member->setValue(v);
	}

	virtual void encodeEepromValue(void * buffer, uint8_t sze) {
		if (sze < sizeof(int32_t)) {
			return;
		}
		*((int32_t*)buffer) = member->getValue();
	}

	virtual int getEepromSize() const {
		return sizeof(int32_t);
	}

	virtual void save() {
		write();
	}
};

class EepromTextStorage: public EepromStored, public IndiVectorMemberStorage {
	IndiTextVectorMember * member;
public:
	EepromTextStorage(IndiTextVectorMember * member, uint32_t addr):
			EepromStored::EepromStored(addr),
			member(member)
	{

	}

	virtual ~EepromTextStorage() {}

	virtual void decodeEepromValue(void * buffer, uint8_t sze) {
		member->setValueFrom((char*)buffer, sze);
	}

	virtual void encodeEepromValue(void * buffer, uint8_t sze) {
		strncpy((char*)buffer, member->getTextValue(), sze);
	}

	virtual int getEepromSize() const {
		return member->getMaxSize();
	}

	virtual void save() {
		write();
	}
};


void IndiVectorMemberStorage::remember(IndiFloatVectorMember * member, uint32_t addr)
{
	IndiVectorMemberStorage * v = new EepromFloatStorage(member, addr);
	member->setStorage(v);
}

void IndiVectorMemberStorage::remember(IndiIntVectorMember * member, uint32_t addr)
{
	IndiVectorMemberStorage * v = new EepromIntStorage(member, addr);
	member->setStorage(v);
}

void IndiVectorMemberStorage::remember(IndiTextVectorMember * member, uint32_t addr)
{
	IndiVectorMemberStorage * v = new EepromTextStorage(member, addr);
	member->setStorage(v);
}
