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
#include "IndiFloatVectorMember.h"
#include "Symbol.h"
#include "CommonUtils.h"


IndiFloatVectorMember::IndiFloatVectorMember(IndiNumberVector * vector, 
	Symbol name, 
	Symbol label,
	double min,
    double max,
	double step)
    :IndiNumberVectorMember(vector, name, label, min, max, step)
{
	this->value = 0;
}

void IndiFloatVectorMember::setValue(double newValue)
{
	if (value == newValue) return;
	value = newValue;
	notifyVectorUpdate(VECTOR_VALUE);
}

void IndiFloatVectorMember::writeValue(WriteBuffer & into) const
{
	into.writeFloat(value);
}

void IndiFloatVectorMember::readValue(ReadBuffer & from)
{
	value = from.readFloat();
}

/*
void IndiFloatVectorMember::dump(WriteBuffer & into, int8_t nameSuffix)
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
	snprintf(buffer, 32, "%.2f", value);
	into.appendXmlEscaped(buffer);
	into.append(F("</defNumber>"));
}*/
