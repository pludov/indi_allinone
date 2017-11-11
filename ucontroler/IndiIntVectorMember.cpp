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
#include "IndiNumberVector.h"
#include "IndiIntVectorMember.h"


IndiIntVectorMember::IndiIntVectorMember(IndiNumberVector * vector, 
	const __FlashStringHelper * name, 
	const __FlashStringHelper * label,
	int min,
    int max)
    :IndiVectorMember(vector, name, label)
{
	this->min = min;
	this->max = max;
	this->value = 0;
}

void IndiIntVectorMember::setValue(int newValue)
{
	if (value == newValue) return;
	value = newValue;
	vector->notifyUpdate(VECTOR_VALUE);
}

void IndiIntVectorMember::dump(WriteBuffer & into, int8_t nameSuffix)
{
	into.append(F("<defNumber name=\""));
	into.appendXmlEscaped(name);
	if (nameSuffix) {
		into.append('_');
		into.append(nameSuffix);
	}
	into.append(F("\" label=\""));
	into.appendXmlEscaped(label);
	into.append(F("\" format=\"%.0f\">"));
	char buffer[32];
	snprintf(buffer, 32, "%d", value);
	into.appendXmlEscaped(buffer);
	into.append(F("</defNumber>"));
}