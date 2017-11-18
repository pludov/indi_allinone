#include <Arduino.h>
#include "ScheduledIndiProtocol.h"


// a 115200, on transmet un caractère (9 bits), en: 1000000 / (115200 / 9) us
#define CHAR_XMIT_DELAY 79

ScheduledIndiProtocol::ScheduledIndiProtocol(Stream * target)
        : IndiProtocol::IndiProtocol(target)
{
    this->priority = 100;
	this->nextTick = UTime::now();
}



void ScheduledIndiProtocol::tick()
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
