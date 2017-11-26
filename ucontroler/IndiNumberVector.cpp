/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#ifdef ARDUINO
#include <Arduino.h>
#endif
#include "Symbol.h"
#include "WriteBuffer.h"
#include "IndiDevice.h"
#include "IndiProtocol.h"
#include "IndiVector.h"
#include "IndiNumberVector.h"
#include "IndiIntVectorMember.h"
#include "IndiFloatVectorMember.h"
#include "BinSerialProtocol.h"
#include "CommonUtils.h"

const VectorKind IndiNumberVectorKind {
	.defVectorText = F("defNumberVector"),
	.newVectorText = F("newNumberVector"),
	.oneMemberText = F("oneNumber"),
	.uid = IndiNumberVectorKindUid,
	.flag = VECTORKIND_NEED_MEMBER_SUBTYPE,
	.vectorFactory = &IndiNumberVector::vectorFactory,
	.memberFactory = &IndiNumberVector::memberFactory
};

IndiVector * IndiNumberVector::vectorFactory(const Symbol & group, const Symbol & name, const Symbol & label)
{
	return new IndiNumberVector(group, name, label, VECTOR_READABLE, false);
}

IndiVectorMember * IndiNumberVector::memberFactory(IndiVector * vector, const Symbol & name, const Symbol & label, uint8_t subType)
{
	// Uses subtype as length
	switch(subType) {
	case IndiIntVectorMember::subType:
		// FIXME: min/max
		return new IndiIntVectorMember((IndiNumberVector*)vector, name, label, 0, 0, 0);
	case IndiFloatVectorMember::subType:
		return new IndiFloatVectorMember((IndiNumberVector*)vector, name, label, 0, 0, 0);
	}
	return 0;
}

IndiNumberVector::IndiNumberVector(const Symbol & group, const Symbol & name, const Symbol & label, uint8_t initialFlag, bool autoregister)
    :IndiVector(group, name, label, initialFlag, autoregister)
{
}

/*
void IndiNumberVector::sendAnnounce(WriteBuffer & into)
{
	if (isHidden()) {
		into.writeDeleteVectorPacket(*this);
	} else {
		into.startAnnounceVectorPacket(*this, )
	}
	into.append(F("<defNumberVector name=\""));
	into.appendXmlEscaped(name);
	if (nameSuffix) {
		into.append('_');
		into.append(nameSuffix);
	}
	into.append(F("\" label=\""));
	into.appendXmlEscaped(label);
	into.append(F("\" group=\""));
	group->dumpXmlEncoded(into);
    into.append(F("\" state=\"Idle\" perm=\"ro\">\n"));
    dumpMembers(into);
	into.append(F("</defNumberVector>\n"));
}*/

const VectorKind & IndiNumberVector::kind() const
{
	return IndiNumberVectorKind;
}
