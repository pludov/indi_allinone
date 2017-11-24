/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
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
#include "IndiNumberVector.h"
#include "IndiIntVectorMember.h"
#include "CommonUtils.h"
#include "Symbol.h"

IndiIntVectorMember::IndiIntVectorMember(IndiNumberVector * vector, 
	Symbol name, 
	Symbol label,
	int32_t min,
    int32_t max,
	int32_t step)
    :IndiNumberVectorMember(vector, name, label, min, max, step)
{
	this->value = 0;
}

void IndiIntVectorMember::setValue(int32_t newValue)
{
	if (value == newValue) return;
	value = newValue;
	notifyVectorUpdate(VECTOR_VALUE);
}

void IndiIntVectorMember::writeValue(WriteBuffer & into) const
{
	into.writeInt(value);
}

void IndiIntVectorMember::readValue(ReadBuffer & from)
{
	value = from.readInt();
}


/*
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
}*/
