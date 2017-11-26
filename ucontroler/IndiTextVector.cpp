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
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"
#include "BinSerialProtocol.h"
#include "CommonUtils.h"

const VectorKind IndiTextVectorKind {
	.defVectorText = F("defTextVector"),
	.newVectorText = F("newTextVector"),
	.oneMemberText = F("oneText"),
	.uid = IndiTextVectorKindUid,
	.flag = VECTORKIND_NEED_MEMBER_SUBTYPE,
	.vectorFactory = &IndiTextVector::vectorFactory,
	.memberFactory = &IndiTextVector::memberFactory
};

IndiVector * IndiTextVector::vectorFactory(const Symbol & group, const Symbol & name, const Symbol & label)
{
	return new IndiTextVector(group, name, label, VECTOR_READABLE, false);
}

IndiVectorMember * IndiTextVector::memberFactory(IndiVector * vector, const Symbol & name, const Symbol & label, uint8_t subType)
{
	// Uses subtype as length
	return new IndiTextVectorMember((IndiTextVector*)vector, name, label, subType);
}

IndiTextVector::IndiTextVector(const Symbol & group, const Symbol & name, const Symbol & label, uint8_t initialFlag, bool autoregister)
    :IndiVector(group, name, label, initialFlag, autoregister)
{
}

const VectorKind & IndiTextVector::kind() const {
	return IndiTextVectorKind;
}

/*void IndiTextVector::dump(WriteBuffer & into)
{
	into.append(F("<defTextVector name=\""));
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
    into.append(F("</defTextVector>\n"));
}
*/
