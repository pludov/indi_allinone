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
#include "IndiVectorMember.h"

// a 115200, on transmet un caractère (9 bits), en: 1000000 / (115200 / 9) us
#define CHAR_XMIT_DELAY 79

#define NOTIF_PACKET_MAX_SIZE 2048

IndiProtocol::IndiProtocol(Stream * target)
{
	IndiDevice & device = IndiDevice::instance();
	
	this->serial = target;
	this->notifPacket = (char*)malloc(NOTIF_PACKET_MAX_SIZE);
	this->writeBuffer = 0;
	this->writeBufferLeft = 0;
	this->priority = 100;
	this->nextTick = UTime::now();

	this->next = device.firstWriter;
	this->clientId = this->next ? this->next->clientId + 1 : 0;
	device.firstWriter = this;

	if (device.variableCount) {
		this->nextDirtyVector = (uint8_t*)malloc(device.variableCount);
		for(int i = 0; i < device.variableCount -1; ++i) {
			this->nextDirtyVector[i] = i + 1;
		}
		this->nextDirtyVector[device.variableCount - 1] = VECNONE;
		this->firstDirtyVector = 0;
		this->lastDirtyVector = device.variableCount - 1;
	} else {
		this->firstDirtyVector = 0;
		this->lastDirtyVector = 0;
		this->nextDirtyVector = 0;
	}
}

void IndiProtocol::dirtied(IndiVector* vector)
{
	uint8_t which = vector->uid;
	if (which == VECNONE) return;

	// Si il est dans la liste:
	//  - soit c'est le dernier
	//  - soit il a un suivant
	if (this->lastDirtyVector == which || this->nextDirtyVector[which] != VECNONE) {
		return;
	}

	// C'est notre nouveau dernier
	if (this->lastDirtyVector != VECNONE) {
		this->nextDirtyVector[this->lastDirtyVector] = which;
		this->lastDirtyVector = which;
	} else {
		this->firstDirtyVector = which;
		this->lastDirtyVector = which;
	}
}


struct DirtyVector {
	IndiVector * vector;
	uint8_t dirtyFlags;
};


void IndiProtocol::popDirty(DirtyVector & result)
{
	uint8_t vectorId = this->firstDirtyVector;
	result.dirtyFlags = 0;
	if (vectorId != VECNONE) {
		IndiVector * v = IndiDevice::instance().list[vectorId];
		result.vector = v;
		for(int i = 0; i < VECTOR_COMM_COUNT; ++i) {
			if (v->cleanDirty(clientId, i)) {
				result.dirtyFlags |= (1 << i);
			}
		}
		
		this->firstDirtyVector = this->nextDirtyVector[vectorId];
		this->nextDirtyVector[vectorId] = VECNONE;
		if (this->firstDirtyVector == VECNONE) {
			this->lastDirtyVector = VECNONE;
		}
	} else {
		result.vector = 0;
	}
}

void IndiProtocol::fillBuffer()
{
	// Il y a de la place, on n'a rien à dire...
	// PArcours la liste des variables (ouch, on peut mieux faire là...)
	DirtyVector toSend;
	popDirty(toSend);
	if (!toSend.vector) {
		return;
	}

	// FIXME: depending on dirtyFlags, ...

	WriteBuffer wf(notifPacket, NOTIF_PACKET_MAX_SIZE);

	toSend.vector->dump(wf);
	if (wf.finish()) {
		writeBuffer = notifPacket;
		writeBufferLeft = wf.size();
		return;
	} else {
		// WTF ? on peut rien faire... on oublie
	}
}

void IndiProtocol::tick()
{
	int spaceAvailable = serial->availableForWrite();
	if (writeBufferLeft == 0 && spaceAvailable > 8) {
		fillBuffer();
	}

	if (writeBufferLeft > 0) {
		int nbCarToWait;
		if (spaceAvailable > 0) {
			// On attend que les caractères aient été écrits.
			nbCarToWait = spaceAvailable;
			if (nbCarToWait > writeBufferLeft) {
				nbCarToWait = writeBufferLeft;
			}
			serial->write(writeBuffer, nbCarToWait);
			writeBuffer += nbCarToWait;
			writeBufferLeft -= nbCarToWait;
		} else {
			nbCarToWait = 1;
		}
		this->nextTick = UTime::now() + nbCarToWait * CHAR_XMIT_DELAY;
		return;
	} else {
		// On ne s'excite pas tout de suite, il n'y a pas de place ou rien à dire
		this->nextTick = UTime::now() + 8 * CHAR_XMIT_DELAY;
		return;
	}
}
