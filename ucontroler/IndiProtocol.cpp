/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <string.h>

#include "WriteBuffer.h"
#include "XmlWriteBuffer.h"
#include "BinSerialWriteBuffer.h"
#include "IndiDevice.h"
#include "IndiProtocol.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"

#include "CommonUtils.h"

#include "BinSerialProtocol.h"


IndiProtocol::IndiProtocol()
{
	IndiDevice & device = IndiDevice::instance();
	
	this->notifPacket = (uint8_t*)malloc(NOTIF_PACKET_MAX_SIZE);
	this->writeBuffer = 0;
	this->writeBufferLeft = 0;
	this->incomingPacket = (uint8_t*)malloc(NOTIF_PACKET_MAX_SIZE);
	this->ackPacket = (uint8_t*)malloc(ACK_PACKET_MAX_SIZE);
	this->incomingPacketSize = 0;
	this->incomingPacketReady = 0;
	this->next = device.firstWriter;
	this->clientId = this->next ? this->next->clientId + 1 : 0;
	device.firstWriter = this;
	if (device.variableCount) {
		this->nextDirtyVector = (uint8_t*)malloc(device.variableCount);
	}
	reset();
}

void IndiProtocol::reset()
{
	IndiDevice & device = IndiDevice::instance();

	this->writeBuffer = 0;
	this->writeBufferLeft = 0;
	this->incomingPacketSize = 0;
	this->incomingPacketReady = 0;
	this->ackPacketSize = 0;
	this->requestPacketSize = 0;
	this->welcomed = 0;

	if (device.variableCount) {
		for(int i = 0; i < device.variableCount; ++i) {
			device.list[i]->resetClient(this->clientId);
		}
		for(int i = 0; i < device.variableCount - 1; ++i) {
			this->nextDirtyVector[i] = i + 1;
		}
		this->nextDirtyVector[device.variableCount - 1] = VECNONE;
		this->firstDirtyVector = 0;
		this->lastDirtyVector = device.variableCount - 1;
	} else {
		this->firstDirtyVector = VECNONE;
		this->lastDirtyVector = VECNONE;
		this->nextDirtyVector = 0;
	}
}

void IndiProtocol::received(uint8_t v)
{
	if (incomingPacketReady) {
		DEBUG(F("Discarding received while ready"));
		return;
	}
	incomingPacket[incomingPacketSize] = v;
	if (v >= MIN_PACKET_START && v <= MAX_PACKET_START) {
		if (incomingPacketSize != 0) {
			DEBUG(F("Packet interrupted after "), incomingPacketSize);
			incomingPacket[0] = v;
		}
		incomingPacketSize = 1;
	} else if (v == PACKET_END) {
		if (!incomingPacketSize) {
			DEBUG(F("End of unknown packet after "), incomingPacketSize);
			incomingPacketSize = 0;
		} else {
			incomingPacketSize++;
			DEBUG(F("Receveid packt of size "), incomingPacketSize);
			
			incomingPacketReady = true;
			onIncomingPacketReady();
		}
	} else {
		if (incomingPacketSize) {
			incomingPacketSize++;
			if (incomingPacketSize >= NOTIF_PACKET_MAX_SIZE) {
				DEBUG(F("Packet overflow"));
				incomingPacketSize = 0;
			}
		} else {
			if (v < 32 || v > 128) {
				DEBUG(F("Discard: #"), (int)v);
			} else {
				DEBUG(F("Discard:"), (char)v);
			}
		}
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
	if (ackPacketSize > 0) {
		// An ack is ready. Push it in the notif buffer
		memcpy(notifPacket, ackPacket, ackPacketSize);
		writeBufferLeft = ackPacketSize;
		writeBuffer = notifPacket;
		ackPacketSize = 0;
		onAckPacketBufferEmpty();
		return;
	}

	if (requestPacketSize > 0) {
		// An ack is ready. Push it in the notif buffer
		memcpy(notifPacket, requestPacket, requestPacketSize);
		writeBufferLeft = requestPacketSize;
		writeBuffer = notifPacket;
		requestPacketSize = 0;
		onRequestPacketBufferEmpty();
	}

	if (!welcomed) {
		welcomed = true;
		BinSerialWriteBuffer welcom(notifPacket, NOTIF_PACKET_MAX_SIZE);
		welcom.startWelcomePacket();
		if (!welcom.isEmpty()) {
			welcom.finish();
			
			writeBuffer = notifPacket;
			writeBufferLeft = welcom.size();
			return;
		}
	}
	// Il y a de la place, on n'a rien à dire...
	// PArcours la liste des variables (ouch, on peut mieux faire là...)
	do {
		DirtyVector toSend;

		popDirty(toSend);
		if (!toSend.vector) {
			return;
		}

		BinSerialWriteBuffer wf(notifPacket, NOTIF_PACKET_MAX_SIZE);
		if (toSend.dirtyFlags & (1 << VECTOR_ANNOUNCED)) {
			toSend.vector->sendAnnounce(wf);
		} else if (toSend.dirtyFlags & (1 << VECTOR_MUTATION)) {
			toSend.vector->sendMutation(wf);
		} else {
			toSend.vector->sendValue(wf);
		}
		if (wf.isEmpty()) {
			// Next.
			DEBUG(F("Dirty vector generated empty notif"));
			continue;
		}

		if (wf.finish()) {
			writeBuffer = notifPacket;
			writeBufferLeft = wf.size();
			return;
		} else {
			// WTF ? on peut rien faire... on oublie ?
		}
	}while(true);
}
