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
#include "IndiVectorGroup.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"
#include "Symbol.h"
#include "CommonUtils.h"

IndiTextVectorMember::IndiTextVectorMember(IndiTextVector * vector,
	Symbol name,
    Symbol label,
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


void IndiTextVectorMember::writeValue(WriteBuffer & into) const
{
	into.writeString(this->value);
}

void IndiTextVectorMember::readValue(ReadBuffer & from)
{
	from.readString(value, maxSize);
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
