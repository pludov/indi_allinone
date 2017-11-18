#ifdef ARDUINO
#include <Arduino.h>
#endif


#include "BinSerialReadBuffer.h"
#include "BinSerialProtocol.h"
#include "IndiProtocol.h"
#include "IndiVector.h"
#include "IndiTextVector.h"
#include "IndiNumberVector.h"
#include "CommonUtils.h"
#include "Symbol.h"


const VectorKind * kindsByUid[IndiMaxVectorKind + 1] = {
    &IndiTextVectorKind,
    &IndiNumberVectorKind
};

BinSerialReadBuffer::BinSerialReadBuffer(uint8_t * buffer, int size)
{
    this->buffer = buffer;
    this->ptr = 0;
    this->left = size;
}

bool BinSerialReadBuffer::readAndApply(IndiDevice & applyTo, IndiProtocol & proto, BinSerialWriteBuffer & answer)
{
    volatile bool error = true;
    if (setjmp(parsePoint) == 0) {
        internalReadAndApply(applyTo, proto, answer);
        error = false;
    }
    return !error;
}

void BinSerialReadBuffer::internalReadAndApply(IndiDevice & applyTo, IndiProtocol &proto, BinSerialWriteBuffer & answer)
{
    uint8_t ctrl = readPacketControl();
    DEBUG(F("[PACKET START]"));
    switch(ctrl) {
        case PACKET_RESTARTED:
        {
            DEBUG(F("[RESTART]"));
#ifdef ARDUINO
            // Reset the whole protocol
            proto.reset();
            answer.appendPacketControl(PACKET_RESTARTED);
            answer.appendPacketControl(PACKET_END);
#endif
            break;
        }
        
#ifndef ARDUINO
        case PACKET_MESSAGE:
        {
            char buffer[128];
            int id = 0;
            while(!isAtEnd()) {
                uint8_t v = readUint7();
                buffer[id++] = v;
                if (id == 127) {
                    break;
                }
            }
            buffer[id] = 0; 
            DEBUG(F("DEBUG: "), buffer);
            break;
        }

        case PACKET_ANNOUNCE:
        {
            DEBUG(F("[ANNOUNCE]"));
            char str[32];
            readSymbol(str, 32);
            DEBUG(F("[NAME]"), str);
            readSymbol(str, 32);
            DEBUG(F("[LABEL]"), str);
            uint8_t flag = readUint7();;
            DEBUG(F("[FLAG]"), flag);
            uint8_t typeUid = readUid();
            DEBUG(F(" UID:"), typeUid);
            if (typeUid >IndiMaxVectorKind) {
                fail(F("Wrong vector kind"));
            }
            
            const VectorKind & kind = *(kindsByUid[typeUid]);
            // while(!isAtEnd()) {
            //     DEBUG(F("[Vector]"));
                
            // }
            break;
        }
#endif
        default:
            DEBUG("Wrong kind: ", (int)ctrl);
            fail(F("Wrong kind"));

    }

    if (readPacketControl() != PACKET_END) {
        fail(F("Packet too big"));
    }
    DEBUG(F("[PACKET END]"));
}

void BinSerialReadBuffer::fail(Symbol why)
{
    DEBUG(F("Packet parsing error: "), why);
    longjmp(parsePoint, 1);
}

uint8_t BinSerialReadBuffer::readOne()
{
    if (!left) fail(F("Malformed packet"));
    uint8_t c = *buffer;

    buffer++;
    left--;
    return c;
}

uint8_t BinSerialReadBuffer::readUint7()
{
    uint8_t r = readOne();
    if (r > 127) {
        fail(F("Wrong uint7"));
    }
    return r;
}
uint8_t BinSerialReadBuffer::peekOne()
{
    if (!left) fail(F("Malformed packet"));
    return *buffer;
}

uint8_t BinSerialReadBuffer::readStringChar()
{
    uint8_t c = readOne();
    if (c > PACKET_MAX_DATA) fail(F("Malformed packet"));
    return c;
}

uint8_t BinSerialReadBuffer::readUid()
{
    uint8_t v = readOne();
    if (v == PACKET_MAX_DATA) {
        v += readOne();
    }
    return v;
}

uint8_t BinSerialReadBuffer::readPacketControl()
{
    uint8_t v = readOne();
    if (v >= MIN_PACKET_START && v <= MAX_PACKET_START) {
        return v;
    }
    if (v == PACKET_END) {
        return v;
    }
    fail(F("Packet control not found"));
}

bool BinSerialReadBuffer::isAtEnd()
{
    uint8_t v = peekOne();
    return (v == PACKET_END);
}

void BinSerialReadBuffer::readSymbol(char * buffer, int maxLength)
{
    int i = 0;
    while(i < maxLength) {
        uint8_t v = readStringChar();
        buffer[i ++] = v;
        if (!v) return;
    }
    fail(F("Symbol overflow"));
}

float BinSerialReadBuffer::readFloat()
{
    char buffer[32];
    readSymbol(buffer, 32);
    float f;
    if (sscanf(buffer, "%f", &f) != 1) {
        DEBUG("Failing float: ", buffer);
        fail(F("Float format error"));
    }
    return f;
}

int32_t BinSerialReadBuffer::readInt()
{
    int32_t result = 0;
    for(int i = 0; i < 5; ++i)
    {
        int32_t v = readOne();
        if (v & 128) {
            fail(F("Invalid int"));
        }
        result |= (v << (7*i));
    }
    return result;
}

