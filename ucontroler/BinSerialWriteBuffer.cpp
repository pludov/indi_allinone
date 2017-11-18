#ifdef ARDUINO
#include <Arduino.h>
#else
#define F(a) (a)
#endif

#include "BinSerialProtocol.h"
#include "BinSerialWriteBuffer.h"
#include "IndiVector.h"


BinSerialWriteBuffer::BinSerialWriteBuffer(uint8_t * into, int size): WriteBuffer(into, size)
{}

void BinSerialWriteBuffer::writeStringChar(uint8_t c)
{
    if (c > 127) c = 127;
    appendUint7(c);
}

void BinSerialWriteBuffer::appendUid(uint8_t c)
{
    if (c >= PACKET_MAX_DATA) {
        append(PACKET_MAX_DATA);
        c-= PACKET_MAX_DATA;
    }
    append(c);
}

void BinSerialWriteBuffer::appendPacketControl(uint8_t v) {
    append(v);
}


bool BinSerialWriteBuffer::finish()
{
    appendPacketControl(PACKET_END);
    return WriteBuffer::finish();
}

void BinSerialWriteBuffer::appendSymbol(Symbol str, uint8_t suffix)
{
#ifdef ARDUINO
    PGM_P p = reinterpret_cast<PGM_P>(str);

	while (1) {
		unsigned char c = pgm_read_byte(p++);
		if (c == 0) break;
		writeStringChar(c);
	}
#else
    for(int u = 0; u < str.length(); ++u)
    {
        unsigned char c = str[u];
        writeStringChar(c);
    }
#endif
    if (suffix) {
		writeStringChar('_');
		writeStringChar('0' + suffix);
    }
    writeStringChar(0);
}

bool BinSerialWriteBuffer::supportUpdateValue() const {
	return true;
}

void BinSerialWriteBuffer::startWelcomePacket()
{
    appendPacketControl(PACKET_RESTARTED);
}

void BinSerialWriteBuffer::writeDeleteVectorPacket(const IndiVector & vec)
{
    appendPacketControl(PACKET_DELETE);
    appendUid(vec.uid);
}

void BinSerialWriteBuffer::startAnnounceVectorPacket(const IndiVector & vec)
{
    appendPacketControl(PACKET_ANNOUNCE);
    appendUid(vec.kind().uid);
}

void BinSerialWriteBuffer::endAnnounceVectorPacket(const IndiVector & vec)
{
}


void BinSerialWriteBuffer::startMutateVectorPacket(const IndiVector & vec)
{
    appendPacketControl(PACKET_MUTATE);
    appendUid(vec.kind().uid);
}

void BinSerialWriteBuffer::endMutateVectorPacket(const IndiVector & vec)
{
}

void BinSerialWriteBuffer::startUpdateValuesPacket(const IndiVector & vec)
{
    appendPacketControl(PACKET_UPDATE);
}

void BinSerialWriteBuffer::endUpdateValuesPacket(const IndiVector & vec)
{
}

void BinSerialWriteBuffer::writeVectorName(Symbol name, uint8_t suffix)
{
	appendSymbol(name, suffix);
}

void BinSerialWriteBuffer::writeVectorFlag(uint8_t fl)
{
    appendUint7(fl);
}

void BinSerialWriteBuffer::writeVectorLabel(Symbol name, uint8_t suffix)
{
	appendSymbol(name, suffix);
}

void BinSerialWriteBuffer::writeVectorUid(uint8_t uid)
{
	appendUid(uid);
}

void BinSerialWriteBuffer::startMember(const IndiVector & vec)
{
}

void BinSerialWriteBuffer::endMember(const IndiVector & vec)
{
}

void BinSerialWriteBuffer::writeVectorMemberSubtype(uint8_t subtype)
{
    appendUint7(subtype);
}

void BinSerialWriteBuffer::writeVectorMemberName(Symbol name, uint8_t suffix)
{
	appendSymbol(name, suffix);
}

void BinSerialWriteBuffer::writeVectorMemberLabel(Symbol name, uint8_t suffix)
{
	appendSymbol(name, suffix);
}

void BinSerialWriteBuffer::writeString(const char * str)
{
    while(*str) {
        uint8_t c = *(str++);
        writeStringChar(c);
    }
}

void BinSerialWriteBuffer::writeFloat(float value)
{
	char buffer[32];
	snprintf(buffer, 32, "%.2f", value);
	writeString(buffer);
}


void BinSerialWriteBuffer::writeInt(int32_t value)
{
    // Send a 35bit value
    for(int i = 0 ; i < 5; ++i)
    {
        appendUint7(value & 127);
        value = value >> 7;
    }
}
