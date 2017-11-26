/*
 * IndiSwitchVectorMember.cpp
 *
 *  Created on: 26 nov. 2017
 *      Author: ludovic
 */
#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "WriteBuffer.h"
#include "IndiDevice.h"
#include "IndiProtocol.h"
#include "IndiVectorGroup.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"
#include "IndiSwitchVector.h"
#include "IndiSwitchVectorMember.h"
#include "Symbol.h"
#include "CommonUtils.h"

IndiSwitchVectorMember::IndiSwitchVectorMember(IndiSwitchVector * vector,
		const Symbol & name,
		const Symbol & label)
	:IndiVectorMember(vector, name, label)
{
	this->value = false;
	vector->refreshActiveOne(this);
}

IndiSwitchVectorMember::~IndiSwitchVectorMember() {
	// FIXME: support for dropping a member ? not soon
}


void IndiSwitchVectorMember::setValue(bool value)
{
    if (this->value == value) {
        return;
    }
	this->value = value;
	((IndiSwitchVector*)vector)->refreshActiveOne(this);
	notifyVectorUpdate(VECTOR_VALUE);
}


void IndiSwitchVectorMember::writeValue(WriteBuffer & into) const
{
	into.writeBool(this->value);
}

bool IndiSwitchVectorMember::readValue(ReadBuffer & from)
{
	bool prev = value;
	value = from.readBool();
	return value != prev;
}

void IndiSwitchVectorMember::skipUpdateValue(ReadBuffer & from) const
{
	from.skipBool();
}

// A convertion from ISState to bool is required.
// However, it is probably ok at byte level
void IndiSwitchVectorMember::writeUpdateValue(WriteBuffer & into, void * ptr) const
{
	// FIXME: size overflow ?
	into.writeBool(*(bool*)ptr);
}
