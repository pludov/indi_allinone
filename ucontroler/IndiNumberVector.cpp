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
#include "IndiVectorGroup.h"
#include "IndiVector.h"
#include "IndiNumberVector.h"
#include "BinSerialProtocol.h"
#include "CommonUtils.h"

const VectorKind IndiNumberVectorKind {
	.defVectorText = F("defNumberVector"),
	.newVectorText = F("newNumberVector"),
	.oneMemberText = F("oneNumber"),
	.uid = IndiNumberVectorKindUid
};

IndiNumberVector::IndiNumberVector(IndiVectorGroup * group,Symbol name,Symbol label)
    :IndiVector(group, name, label)
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

bool IndiNumberVector::hasMemberSubtype() const
{
	return true;
}

const VectorKind & IndiNumberVector::kind() const
{
	return IndiNumberVectorKind;
}
