/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "WriteBuffer.h"
#include "IndiDevice.h"
#include "IndiProtocol.h"
#include "IndiVectorGroup.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"


IndiVector::IndiVector(IndiVectorGroup * group, const __FlashStringHelper * name, const __FlashStringHelper * label)
{
	this->group = group;
	this->name = name;
	this->label = label;
	this->first = 0;
	this->last = 0;
	this->nameSuffix = 0;
	this->flag = 0;
	IndiDevice::instance().add(this);
}

void IndiVector::notifyUpdate(uint8_t which)
{
	IndiDevice & device = IndiDevice::instance();
	notifStatus[which] = 0;

	for(IndiProtocol * dw = device.firstWriter; dw; dw = dw->next)
	{
		Serial.println("dirtied");
		dw->dirtied(this);
	}
}

bool IndiVector::isDirty(uint8_t clientId, uint8_t commId)
{
	// Dirty means bit is set to 0
	return (notifStatus[commId] & (1 << clientId)) == 0;
}

bool IndiVector::cleanDirty(uint8_t clientId, uint8_t commId)
{
	// Set bit to 1
	uint8_t mask = 1 << clientId;
	uint8_t val = notifStatus[commId];
	if (!(val & mask)) {
		val |= mask;
		return true;
	}
	return false;
}


void IndiVector::set(uint8_t flag, bool status)
{
	uint8_t newFlag = (this->flag & ~flag);
	if (status) newFlag |= flag;
	if (this->flag != newFlag) {
		notifyUpdate(VECTOR_MUTATION);
	}
}

void IndiVector::dumpMembers(WriteBuffer & into)
{
	for(IndiVectorMember * cur = first; cur; cur=cur->next)
	{
		into.append('\t');
		cur->dump(into, nameSuffix);
		into.append('\n');
	}
}
