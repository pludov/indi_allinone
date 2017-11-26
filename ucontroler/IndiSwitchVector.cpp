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
#include "IndiVectorGroup.h"
#include "IndiVector.h"
#include "IndiSwitchVector.h"
#include "IndiSwitchVectorMember.h"
#include "BinSerialProtocol.h"
#include "CommonUtils.h"


const VectorKind IndiSwitchVectorKind {
	.defVectorText = F("defSwitchVector"),
	.newVectorText = F("newSwitchVector"),
	.oneMemberText = F("oneSwitch"),
	.uid = IndiSwitchVectorKindUid,
	.flag = 0,
	.vectorFactory = &IndiSwitchVector::vectorFactory,
	.memberFactory = &IndiSwitchVector::memberFactory
};


IndiVector * IndiSwitchVector::vectorFactory(Symbol name, Symbol label)
{
	// FIXME: group name
	// FIXME: ISRule (oneOfMany for now)
	return new IndiSwitchVector(new IndiVectorGroup(F("plop")), name, label, VECTOR_READABLE, false);
}


IndiVectorMember * IndiSwitchVector::memberFactory(IndiVector * vector, Symbol name, Symbol label, uint8_t subType)
{
	// Uses subtype as length
	return new IndiSwitchVectorMember((IndiSwitchVector*)vector, name, label);
}

IndiSwitchVector::IndiSwitchVector(IndiVectorGroup * group,Symbol name,Symbol label, uint8_t initialFlag, bool autoregister)
    :IndiVector(group, name, label, initialFlag, autoregister)
{
	activeOne = nullptr;
}

IndiSwitchVector::~IndiSwitchVector() {
}

// Imply that the vector is already dirty
void IndiSwitchVector::refreshActiveOne(IndiSwitchVectorMember * lastUpdated)
{
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
	if (IndiVector::doUpdate(request)) {
		refreshActiveOne((IndiSwitchVectorMember*)request.members[0]);
		return true;
	}
	return false;
}


const VectorKind & IndiSwitchVector::kind() const
{
	return IndiSwitchVectorKind;
}

