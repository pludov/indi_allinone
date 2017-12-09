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
#include "IndiVectorMemberStorage.h"
#include "IndiVectorMember.h"

#include "CommonUtils.h"

IndiVector::IndiVector(const Symbol & group, const Symbol & name, const Symbol & label, uint8_t initialFlag, bool autoregister)
	: group(group),
	  name(name),
	  label(label),
	  requestCallback()
{
	this->first = 0;
	this->last = 0;
	this->flag = initialFlag;
	this->uid = VECNONE;
	if (autoregister) {
		IndiDevice::instance().add(this);
	}
}

// Assume the vector was unregistered from its device
IndiVector::~IndiVector()
{
	IndiVectorMember * cur = first;
	while(cur) {
		IndiVectorMember * next = cur->next;
		delete cur;
		cur = next;
	}
}

void IndiVector::onRequested(const VectorCallback & function)
{
	this->requestCallback = function;
}

int IndiVector::getMemberCount() const
{
	int i = 0;
	for(IndiVectorMember * child = first; child; child = child->next) {
		i++;
	}
	return i;
}

void IndiVector::notifyUpdate(uint8_t which)
{
	// Don't bother with unregistered vectors
	if (uid == VECNONE) {
		return;
	}
	IndiDevice & device = IndiDevice::instance();
	notifStatus[which] = 0;

	for(IndiProtocol * dw = device.firstWriter; dw; dw = dw->next)
	{
		DEBUG(F("dirtied: "), (int)which);
		dw->dirtied(this);
	}
}

void IndiVector::resetClient(uint8_t clientId)
{
	for(int commId = 0; commId < VECTOR_COMM_COUNT; ++commId) {
		notifStatus[commId] &= ~(((uint8_t)1)<<clientId);
	}
}

bool IndiVector::isDirty(uint8_t clientId, uint8_t commId)
{
	// Dirty means bit is set to 0
	return (notifStatus[commId] & (1 << clientId)) == 0;
}

bool IndiVector::cleanDirty(uint8_t clientId, uint8_t commId)
{
	// Set bit to 1
	uint8_t mask = 1 << clientId;
	uint8_t val = notifStatus[commId];
	if (!(val & mask)) {
		notifStatus[commId] |= mask;
		return true;
	}
	return false;
}

bool IndiVector::setDirty(uint8_t clientId, uint8_t commId)
{
	// Set bit to 1
	uint8_t mask = 1 << clientId;
	for(int i = 0; i <= commId; ++i) {
		if (!(notifStatus[i] & mask)) {
			return false;
		}
	}
	notifStatus[commId] &= ~mask;

	return true;
}


void IndiVector::set(uint8_t flagToChange, bool status)
{
	auto oldFlag = flag;
	if (status) {
		flag |= flagToChange;
	} else {
		flag &= ~flagToChange;
	}
	if (flag != oldFlag) {
		if (flagToChange == VECTOR_HIDDEN) {
			notifyUpdate(VECTOR_ANNOUNCED);
		} else if (flagToChange == VECTOR_BUSY) {
			notifyUpdate(VECTOR_VALUE);
		} else {
			notifyUpdate(VECTOR_MUTATION);
		}
	}
}

/*void IndiVector::dumpMembers(WriteBuffer & into)
{
	for(IndiVectorMember * cur = first; cur; cur=cur->next)
	{
		into.append('\t');
		cur->dump(into, nameSuffix);
		into.append('\n');
	}
}*/


void IndiVector::sendDefinition(WriteBuffer & into)
{
	into.writeVectorGroup(group);
	into.writeVectorName(name);
	into.writeVectorLabel(label);
	into.writeVectorFlag(flag);
	into.writeVectorUid(uid);
	for(IndiVectorMember * cur = first; cur; cur=cur->next)
	{
		into.startMember(*this);
		if (kind().hasMemberSubtype()) {
			into.writeVectorMemberSubtype(cur->getSubtype());
		}
		into.writeVectorMemberName(cur->name);
		into.writeVectorMemberLabel(cur->label);
		
		cur->writeValue(into);
		into.endMember(*this);
	}
}

void IndiVector::sendAnnounce(WriteBuffer & into)
{
	if (hidden()) {
		into.writeDeleteVectorPacket(*this);
	} else {
		into.startAnnounceVectorPacket(*this);
		sendDefinition(into);
		into.endAnnounceVectorPacket(*this);
	}
}

void IndiVector::sendMutation(WriteBuffer & into)
{
	if (hidden()) {
		// Mutation when hidden should not be sent.
		return;
	}
	into.startMutateVectorPacket(*this);
	sendDefinition(into);
	into.endMutateVectorPacket(*this);
}

void IndiVector::sendValue(WriteBuffer & into)
{
	DEBUG(F("REQUESTED value update"));
	
	if (!into.supportUpdateValue()) {
		sendMutation(into);
		return;
	}
	if (hidden()) {
		// Mutation when hidden should not be sent.
		return;
	}
	DEBUG(F("SENDING value update"));
	into.startUpdateValuesPacket(*this);
	into.writeVectorUid(uid);
	into.writeVectorFlag(flag & VECTOR_BUSY);
	for(IndiVectorMember * cur = first; cur; cur = cur->next)
	{
		cur->writeValue(into);
	}
	into.endUpdateValuesPacket(*this);
}


bool IndiVector::doUpdate(IndiVectorUpdateRequest & request)
{
	int changeCount = 0;
	for(int i = 0; i < request.updatedMemberCount; ++i) {
		request.seekAt(i);
		if (request.members[i]->readValue(*request.readBuffer)) {
			request.members[i]->saveToStorage();
			changeCount ++;
		}
	}
	if (changeCount) {
		this->notifyUpdate(VECTOR_VALUE);
	}

	return changeCount > 0;
}

// not for Arduino
/*void IndiVector::readMembers(ReadBuffer & from)
{
	// Drop existing members
	for(IndiVectorMember * cur = first; cur; )
	{
		IndiVectorMember * was = cur;
		delete(cur);
		cur = was->next;
	}
	first = nullptr;
	last = nullptr;

	while(!from.atEnd())
	{
		uint8_t childType = 0;
		if (hasMemberSubtype()) {
			childType = from.readU7();
		}
		Symbol name, label;
		name = from.readSymbol();
		label = from.readSymbol();
		VectorMember * vm = newMember(this, name, label, childType);
		
		vm.readAttributes(from);
		vm.readValue(from);
	}
}

void IndiVector::readValue(WriteBuffer & from)
{
	// Will be handled separately for switch, ...
	for(IndiVectorMember * cur = first; cur; cur = cur->next)
	{
		cur->readValue(from);
	}
}*/

IndiVectorUpdateRequest::IndiVectorUpdateRequest(ReadBuffer * rb, IndiVectorMember * * members, uint16_t * offsets)
{
	this->updatedMemberCount = 0;
	this->readBuffer = rb;
	this->members = members;
	this->offsets = offsets;
	this->endOffset = 0;
}

void IndiVectorUpdateRequest::addItem(IndiVectorMember * member, uint16_t offset)
{
	members[updatedMemberCount] = member;
	offsets[updatedMemberCount] = offset;
	updatedMemberCount++;
}

void IndiVectorUpdateRequest::seekAt(int itemId)
{
	DEBUG(F("Seek back for"), itemId, F(" at "), this->offsets[itemId]);
	this->readBuffer->seekAt(this->offsets[itemId]);
}

void IndiVectorUpdateRequest::markEnd()
{
	this->endOffset = this->readBuffer->getCurrentPos();
}

void IndiVectorUpdateRequest::unseek()
{
	this->readBuffer->seekAt(endOffset);
}
