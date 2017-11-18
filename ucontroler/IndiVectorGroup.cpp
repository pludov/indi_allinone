/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "IndiVectorGroup.h"
#include "WriteBuffer.h"
#include "Utils.h"

IndiVectorGroup::IndiVectorGroup(const __FlashStringHelper * name, int8_t suffix)
{
    this->suffix = suffix;
	this->name = name;
}

/*void IndiVectorGroup::dumpXmlEncoded(WriteBuffer & into) const
{
    into.appendXmlEscaped(name);
    if (suffix) {
        into.append('_');
        into.append(Utils::hex(suffix));
    }
}
*/