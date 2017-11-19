#ifdef ARDUINO
#include <Arduino.h>
#endif


#include "ReadBuffer.h"
#include "IndiProtocol.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"
#include "IndiTextVector.h"
#include "IndiNumberVector.h"
#include "CommonUtils.h"
#include "Symbol.h"
#include "Utils.h"


ReadBuffer::ReadBuffer(uint8_t * buffer, int size)
{
    this->buffer = buffer;
    this->ptr = 0;
    this->left = size;
}

bool ReadBuffer::readAndApply(IndiDevice & applyTo, IndiProtocol & proto, BinSerialWriteBuffer & answer)
{
    volatile bool error = true;
    if (setjmp(parsePoint) == 0) {
#ifndef ARDUINO
        int toDisplayOffset = ptr;
        while(toDisplayOffset < left) {
            int thisLoop = left - toDisplayOffset;
            if (thisLoop  > 30) thisLoop = 30;
    
            char display[3 * thisLoop + 1];
            char * dp = display;
            for(int i = 0; i < thisLoop; ++i)
            {
                uint8_t c = buffer[toDisplayOffset + i];
                *(dp++) = Utils::hex(c >> 4);
                *(dp++) = Utils::hex(c & 15);
                *(dp++) = ' ';
            }
            *dp = '\0';
            DEBUG(F("Received (hex): "), display);
        
            dp = display;
            for(int i = 0; i < thisLoop; ++i)
            {
                uint8_t c = buffer[toDisplayOffset + i];
                if (c > 27 && c < 128) {
                    *(dp++) = c;
                } else {
                    *(dp++) = ' ';
                }
                *(dp++) = ' ';
                *(dp++) = ' ';
            }
            *dp = '\0';
            DEBUG(F("Received (asc): "), display);
    
            toDisplayOffset += thisLoop;
        }
#endif

        internalReadAndApply(applyTo, proto, answer);
        error = false;
    }
    return !error;
}

void ReadBuffer::fail(Symbol why)
{
    DEBUG(F("Packet parsing error: "), why);
    longjmp(parsePoint, 1);
}

uint8_t ReadBuffer::readOne()
{
    if (!left) fail(F("Malformed packet"));
    uint8_t c = *buffer;

    buffer++;
    left--;
    return c;
}

uint8_t ReadBuffer::peekOne()
{
    if (!left) fail(F("Malformed packet"));
    return *buffer;
}

