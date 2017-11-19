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


IndiVectorMember::IndiVectorMember(IndiVector * vector, 
	Symbol name, 
	Symbol label)
{
	this->vector = vector;
	next = 0;
	if (vector->last) {
		vector->last->next = this;
	} else {
		vector->first = this;
	}
	vector->last = this;
	this->name = name;
	this->label = label;
}

IndiVectorMember::~IndiVectorMember()
{}

void IndiVectorMember::notifyVectorUpdate(uint8_t commId) {
	vector->notifyUpdate(commId);
}