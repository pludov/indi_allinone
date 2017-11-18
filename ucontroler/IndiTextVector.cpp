/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "WriteBuffer.h"
#include "IndiDevice.h"
#include "IndiProtocol.h"
#include "IndiVectorGroup.h"
#include "IndiVector.h"
#include "IndiTextVector.h"


const VectorKind IndiTextVectorKind {
	.defVectorText = F("defTextVector"),
	.newVectorText = F("newTextVector"),
	.oneMemberText = F("oneText"),
	.uid = 1
};

IndiTextVector::IndiTextVector(IndiVectorGroup * group, const __FlashStringHelper * name, const __FlashStringHelper * label)
    :IndiVector(group, name, label)
{
}

const VectorKind & IndiTextVector::kind() const {
	return IndiTextVectorKind;
}


bool IndiTextVector::hasMemberSubtype() const {
	return false;
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
