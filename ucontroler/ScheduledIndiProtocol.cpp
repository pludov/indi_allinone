#include <Arduino.h>
#include "ScheduledIndiProtocol.h"
#include "BinSerialReadBuffer.h"
#include "BinSerialWriteBuffer.h"
#include "BinSerialProtocol.h"

#include "IndiDevice.h"

// a 115200, on transmet un caractère (9 bits), en: 1000000 / (115200 / 9) us
#define CHAR_XMIT_DELAY 79

ScheduledIndiProtocol::ScheduledIndiProtocol(Stream * target)
        : IndiProtocol::IndiProtocol()
{
	this->serial = target;
    this->priority = 100;
	this->nextTick = UTime::now();
}

void ScheduledIndiProtocol::onIncomingPacketReady()
{
	if (ackPacketSize == 0) {
		this->nextTick = UTime::now();
	}
	// Else wait until ackPacketBuffer is free
}

void ScheduledIndiProtocol::onAckPacketBufferEmpty()
{
	if (incomingPacketReady) {
		this->nextTick = UTime::now();
	}
}

void ScheduledIndiProtocol::handleIncomingPacket(uint8_t * packet, int packetSize, BinSerialWriteBuffer & answer)
{
	BinSerialReadBuffer bsrb(packet, packetSize);
	if (!bsrb.readAndApply(IndiDevice::instance(), *this, answer)) {
		reset();
	}
}

void ScheduledIndiProtocol::tick()
{
	if (incomingPacketReady && ackPacketSize == 0) {
		int prevIncomingPacketSize = incomingPacketSize;
		incomingPacketReady = 0;
		incomingPacketSize = 0;
		// We can reply to the packet
		// FIXME: which protocol here ?
		BinSerialWriteBuffer bswb(ackPacket, ACK_PACKET_MAX_SIZE);
		handleIncomingPacket(incomingPacket, prevIncomingPacketSize, bswb);
		if (!bswb.isEmpty()) {
			ackPacketSize = bswb.size();
		}
	}

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


void ScheduledIndiProtocol::idle()
{
	// Read if available
	while ((!incomingPacketReady) && serial->available()) {
		uint8_t v = serial->read();
		received(v);
	}

}