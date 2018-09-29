#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "BinSerialProtocol.h"
#include "BinSerialWriteBuffer.h"
#include "IndiVector.h"
#include "CommonUtils.h"
#include "Utils.h"

BinSerialWriteBuffer::BinSerialWriteBuffer(uint8_t * into, int size): WriteBuffer(into, size)
{}

BinSerialWriteBuffer::~BinSerialWriteBuffer()
{
}

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

void BinSerialWriteBuffer::appendSymbol(const Symbol & str)
{
#ifdef ARDUINO
    PGM_P p = reinterpret_cast<PGM_P>(str.base);

	while (1) {
		unsigned char c = pgm_read_byte(p++);
		if (c == 0) break;
		writeStringChar(c);
	}
	if (str.suffix) {
		writeStringChar('_');
		writeStringChar('0' + str.suffix);
	}
#else
    for(unsigned u = 0; u < str.length(); ++u)
    {
        unsigned char c = str[u];
        writeStringChar(c);
    }
#endif
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

void BinSerialWriteBuffer::writeVectorGroup(const Symbol & group)
{
	appendSymbol(group);
}

void BinSerialWriteBuffer::writeVectorName(const Symbol & name)
{
	appendSymbol(name);
}

void BinSerialWriteBuffer::writeVectorFlag(uint8_t fl)
{
    appendUint7(fl);
}

void BinSerialWriteBuffer::writeVectorLabel(const Symbol & name)
{
	appendSymbol(name);
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

void BinSerialWriteBuffer::writeVectorMemberName(const Symbol & name)
{
	appendSymbol(name);
}

void BinSerialWriteBuffer::writeVectorMemberLabel(const Symbol & name)
{
	appendSymbol(name);
}

void BinSerialWriteBuffer::writeString(const char * str)
{
    while(*str) {
        uint8_t c = *(str++);
        writeStringChar(c);
    }
    writeStringChar(0);
}

void BinSerialWriteBuffer::writeBool(bool b)
{
	appendUint7(b ? 1 : 0);
}

void BinSerialWriteBuffer::writeFloat(float value)
{
	writeInt(*(int32_t*)&value);
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

void BinSerialWriteBuffer::debug() const
{
#ifndef ARDUINO
	int toDisplayOffset = 0;
	uint8_t * start = ptr - size();
	while(toDisplayOffset < size()) {
		int thisLoop = size() - toDisplayOffset;
		if (thisLoop  > 30) thisLoop = 30;

		char display[3 * thisLoop + 1];
		char * dp = display;
		for(int i = 0; i < thisLoop; ++i)
		{
			uint8_t c = start[toDisplayOffset + i];
			*(dp++) = Utils::hex(c >> 4);
			*(dp++) = Utils::hex(c & 15);
			*(dp++) = ' ';
		}
		*dp = '\0';
		DEBUG(F("Produced (hex): "), display);

		dp = display;
		for(int i = 0; i < thisLoop; ++i)
		{
			uint8_t c = start[toDisplayOffset + i];
			if (c > 27 && c < 128) {
				*(dp++) = c;
			} else {
				*(dp++) = ' ';
			}
			*(dp++) = ' ';
			*(dp++) = ' ';
		}
		*dp = '\0';
		DEBUG(F("Produced (asc): "), display);

		toDisplayOffset += thisLoop;
	}
#endif
}
