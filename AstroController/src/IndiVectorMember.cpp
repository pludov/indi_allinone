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
#include "IndiVector.h"
#include "IndiVectorMember.h"
#include "IndiVectorMemberStorage.h"


IndiVectorMember::IndiVectorMember(IndiVector * vector, 
	const Symbol & name,
	const Symbol & label) :
	name(name),
	label(label),
	storage(nullptr)
{
	this->vector = vector;
	next = 0;
	if (vector->last) {
		vector->last->next = this;
	} else {
		vector->first = this;
	}
	vector->last = this;
}

IndiVectorMember::~IndiVectorMember()
{}

void IndiVectorMember::notifyVectorUpdate(uint8_t commId) {
	vector->notifyUpdate(commId);
}

void IndiVectorMember::saveToStorage()
{
	if (storage) {
		storage->save();
	}
}

void IndiVectorMember::setStorage(IndiVectorMemberStorage * storage)
{
	this->storage = storage;
}
