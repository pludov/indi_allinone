/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <string.h>

#include "WriteBuffer.h"
#include "IndiDevice.h"
#include "IndiProtocol.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"
#include "Symbol.h"
#include "CommonUtils.h"

IndiTextVectorMember::IndiTextVectorMember(IndiTextVector * vector,
	const Symbol & name,
	const Symbol & label,
    uint8_t maxSize)
    :IndiVectorMember(vector, name, label)
{
    this->value = (char*)malloc(((int)maxSize) + 1);
    this->value[0] = 0;
    this->maxSize = maxSize;
}

IndiTextVectorMember::~IndiTextVectorMember()
{
	free(this->value);
}

void IndiTextVectorMember::setValue(const char * value)
{
    if (!strcmp(this->value, value)) {
        return;
    }
	strcpy(this->value, value);
	notifyVectorUpdate(VECTOR_VALUE);
}

void IndiTextVectorMember::setValueFrom(const char * value, int sze)
{
	int realLen = strnlen(value, sze);
	if (realLen > maxSize) {
		realLen = maxSize;
	}
	if (!strncmp(this->value, value, realLen)) {
		return;
	}
	strncpy(this->value, value, realLen);
	this->value[realLen] = 0;
	notifyVectorUpdate(VECTOR_VALUE);
}

void IndiTextVectorMember::writeValue(WriteBuffer & into) const
{
	into.writeString(this->value, maxSize);
}

bool IndiTextVectorMember::readValue(ReadBuffer & from)
{
	return from.readString(value, maxSize);
}

void IndiTextVectorMember::skipUpdateValue(ReadBuffer & from) const
{
	from.skipString(maxSize);
}

void IndiTextVectorMember::writeUpdateValue(WriteBuffer & into, void * ptr) const
{
	const char * write = *(char**)ptr;
	into.writeString(write, maxSize);
}

/*
void IndiTextVectorMember::dump(WriteBuffer & into, int8_t nameSuffix)
{
	into.append(F("<defText name=\""));
	into.appendXmlEscaped(name);
	if (nameSuffix) {
		into.append('_');
		into.append(nameSuffix);
	}
	into.append(F("\" label=\""));
    into.appendXmlEscaped(label);
    into.append(F("\">"));
	into.appendXmlEscaped(value);
	into.append(F("</defText>"));
}*/
