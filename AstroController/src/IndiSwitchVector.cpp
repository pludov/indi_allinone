/*
 * IndiSwitchVector.cpp
 *
 *  Created on: 26 nov. 2017
 *      Author: ludovic
 */
#ifdef ARDUINO
#include <Arduino.h>
#endif
#include "Symbol.h"
#include "WriteBuffer.h"
#include "IndiDevice.h"
#include "IndiProtocol.h"
#include "IndiVector.h"
#include "IndiSwitchVector.h"
#include "IndiSwitchVectorMember.h"
#include "BinSerialProtocol.h"
#include "CommonUtils.h"

static Symbol __defSwitchVector() {
	return F("defSwitchVector");
}

static Symbol __newSwitchVector() {
	return F("newSwitchVector");
}

static Symbol __oneSwitch() {
	return F("oneSwitch");
}



const VectorKind IndiSwitchVectorKind {
	.defVectorText = __defSwitchVector(),
	.newVectorText = __newSwitchVector(),
	.oneMemberText = __oneSwitch(),
	.uid = IndiSwitchVectorKindUid,
	.flag = 0,
	.vectorFactory = &IndiSwitchVector::vectorFactory,
	.memberFactory = &IndiSwitchVector::memberFactory
};


IndiVector * IndiSwitchVector::vectorFactory(const Symbol & group, const Symbol & name, const Symbol & label)
{
	// FIXME: group name
	// FIXME: ISRule (oneOfMany for now)
	return new IndiSwitchVector(group, name, label, VECTOR_READABLE, false);
}


IndiVectorMember * IndiSwitchVector::memberFactory(IndiVector * vector, const Symbol & name, const Symbol & label, uint8_t subType)
{
	// Uses subtype as length
	return new IndiSwitchVectorMember((IndiSwitchVector*)vector, name, label);
}

IndiSwitchVector::IndiSwitchVector(const Symbol & group, const Symbol & name,const Symbol & label, uint8_t initialFlag, bool autoregister)
    :IndiVector(group, name, label, initialFlag, autoregister)
{
	activeOne = nullptr;
	first = nullptr;
	last = nullptr;
}

IndiSwitchVector::~IndiSwitchVector() {
}

// Imply that the vector is already dirty
void IndiSwitchVector::refreshActiveOne(IndiSwitchVectorMember * lastUpdated)
{
	if (independantMembers()) {
		return;
	}
	// ensure only one active.
	bool hasOne = false;
	bool hasMoreThanOne = false;
	IndiSwitchVectorMember * newActive;
	for(IndiVectorMember * cur = first; cur; cur = cur->next)
	{
		IndiSwitchVectorMember * curSwitch = (IndiSwitchVectorMember*)cur;
		if (curSwitch->getValue()) {
			if (hasOne) {
				hasMoreThanOne = true;
			} else {
				newActive = curSwitch;
				hasOne = true;
			}
		}
	}
	if (!hasOne) {
		// Take the first, not if it is lastUpdated
		if (first) {
			newActive = (IndiSwitchVectorMember*)first;
			if (newActive != nullptr && newActive == lastUpdated && newActive->next) {
				DEBUG(F("Switch rej"), newActive->label);
				newActive = (IndiSwitchVectorMember*)newActive->next;
			}
			// Don't bother signaling or rechecking.
			DEBUG(F("Switch def to: "), newActive->label);
			newActive->value = true;
		} else {
			DEBUG(F("Empty switch"));
			newActive = nullptr;
		}
	} else if (hasMoreThanOne) {
		if (lastUpdated != nullptr && lastUpdated->getValue()) {
			newActive = lastUpdated;
		}
		DEBUG(F("Choosed: "), newActive->label);
		// keep only newActive
		for(IndiVectorMember * cur = first; cur; cur = cur->next)
		{
			IndiSwitchVectorMember * curSwitch = (IndiSwitchVectorMember*)cur;
			if (curSwitch->getValue() && curSwitch != newActive) {
				curSwitch->value = false;
			}
		}
	}
	activeOne = newActive;
}

bool IndiSwitchVector::doUpdate(IndiVectorUpdateRequest & request)
{
	if (independantMembers()) {
		uint32_t previousActive = getValueMask();
		if (IndiVector::doUpdate(request)) {
			refreshActiveOne((IndiSwitchVectorMember*)request.members[0]);
			return getValueMask() != previousActive;
		}
		return false;
	} else {
		IndiVectorMember * previousActive = this->activeOne;
		if (IndiVector::doUpdate(request)) {
			refreshActiveOne((IndiSwitchVectorMember*)request.members[0]);
			return this->activeOne != previousActive;
		}
		return false;
	}
}

uint32_t IndiSwitchVector::getValueMask() const
{
	uint32_t rslt = 0;
	uint32_t i = 1;
	IndiVectorMember * cur = first;
	while(cur) {
		if (((IndiSwitchVectorMember*)cur)->getValue()) {
			rslt |= i;
		}
		i = i << 1;
		cur = cur->next;
	}
	return rslt;
}


const VectorKind & IndiSwitchVector::kind() const
{
	return IndiSwitchVectorKind;
}

