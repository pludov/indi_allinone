#ifdef ARDUINO
#include <Arduino.h>
#endif


#include "BinSerialReadBuffer.h"
#include "BinSerialProtocol.h"
#include "IndiProtocol.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"
#include "IndiTextVector.h"
#include "IndiNumberVector.h"
#include "CommonUtils.h"
#include "Symbol.h"
#include "Utils.h"

const VectorKind * kindsByUid[IndiMaxVectorKind + 1] = {
    &IndiTextVectorKind,
    &IndiNumberVectorKind
};

BinSerialReadBuffer::BinSerialReadBuffer(uint8_t * buffer, int size) : ReadBuffer::ReadBuffer(buffer, size)
{
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
            uint8_t typeUid = readUid();
            DEBUG(F(" [TYPE]:"), (int)typeUid);

            if (typeUid >IndiMaxVectorKind) {
                fail(F("Wrong vector kind"));
            }

            char name[64];
            readSymbol(name, 64);
            DEBUG(F(" [NAME]"), name);
            char label[64];
            readSymbol(label, 32);
            DEBUG(F(" [LABEL]"), label);
            uint8_t flag = readUint7();;
            DEBUG(F(" [FLAG]"), (int)flag);
            
            uint8_t clientUid = readUid();
            DEBUG(F(" [UID]"), (int)clientUid);

            const VectorKind & kind = *(kindsByUid[typeUid]);
            IndiVector * vec = kind.newVector(name, label);
            while(!isAtEnd()) {
                uint8_t subType = 0;
                if (kind.hasMemberSubtype()) {
                    subType = readUid();
                }
                DEBUG(F(" [Member] subtype="), (int)subType);

                readSymbol(name, 64);
                DEBUG(F("  [NAME]"), name);
                
                readSymbol(label, 64);
                DEBUG(F("  [LABEL]"), label);
                
                IndiVectorMember * member = kind.newMember(vec, name, label, subType);

                if (!member) {
                    fail(F("Wrong vector subtype"));
                }
                member->readValue(*this);
            }
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


void BinSerialReadBuffer::readString(char * buffer, int maxSize)
{
	readSymbol(buffer, maxSize);
}


uint8_t BinSerialReadBuffer::readUint7()
{
    uint8_t r = readOne();
    if (r > 127) {
        fail(F("Wrong uint7"));
    }
    return r;
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
        DEBUG(F("Failing float: "), buffer);
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

