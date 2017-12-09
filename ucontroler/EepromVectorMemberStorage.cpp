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

void IndiVectorMemberStorage::remember(IndiFloatVectorMember * member, uint32_t addr)
{
	IndiVectorMemberStorage * v = new EepromFloatStorage(member, addr);
	member->setStorage(v);
}
