/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "IndiVectorGroup.h"


IndiVectorGroup::IndiVectorGroup(const __FlashStringHelper * name)
{
	this->name = name;
}
