/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#ifdef ARDUINO
#include <Arduino.h>
#else
#define F(a) (a)
#endif

#include "WriteBuffer.h"
#include "Utils.h"
#include "IndiVector.h"

WriteBuffer::WriteBuffer(char * into, int size)
{
	this->ptr = into;
	this->left = size;
	this->totalSize = size;
}

void WriteBuffer::append(char c)
{
	if (this->left) {
		this->ptr[0] = c;
		this->ptr++;
		this->left--;
	} else {
		this->left = -1;
	}
}

int WriteBuffer::size()
{
	return totalSize - left;
}

bool WriteBuffer::isEmpty()
{
	return totalSize == left;
}

#ifdef ARDUINO

void WriteBuffer::append(const __FlashStringHelper * str)
{
	PGM_P p = reinterpret_cast<PGM_P>(str);

	while (1) {
	  unsigned char c = pgm_read_byte(p++);
	  if (c == 0) break;
	  append(c);
	}
}

#endif

void WriteBuffer::appendXmlEscaped(char c) {
	switch(c) {
		case '"':
			append(F("&quot;"));
			break;
		case '\'':
			append(F("&apos;"));
			break;
		case '<':
			append(F("&lt;"));
			break;
		case '>':
			append(F("&gt;"));
			break;
		case ';':
			append(F("&amp;"));
			break;
		default:
			append(c);
	}
}

#ifdef ARDUINO

void WriteBuffer::appendXmlEscaped(const __FlashStringHelper * str)
{
	PGM_P p = reinterpret_cast<PGM_P>(str);
	
	while (1) {
		unsigned char c = pgm_read_byte(p++);
		if (c == 0) break;
		appendXmlEscaped(c);
	}
}

#endif

void WriteBuffer::appendXmlEscaped(const char * s)
{
	while(*s) {
		appendXmlEscaped(*(s++));
	}
}

void WriteBuffer::append(const char * s)
{
	append('"');
	while(*s) {
		unsigned char c = *(s++);
		switch (c) {
			case '\\':
			case '"':
			case '/':
				append('\\');
				append(c);
				break;
			case '\b':
				append('\\');
				append('b');
				break;
			case '\t':
				append('\\');
				append('t');
				break;
			case '\n':
				append('\\');
				append('n');
				break;
			case '\f':
				append('\\');
				append('f');
				break;
			case '\r':
				append('\\');
				append('i');
				break;
			default:
				if (c < ' ') {
					append('\\');
					append('u');
					append('0');
					append('0');
					append(Utils::hex(c >> 4));
					append(Utils::hex(c & 15));
				} else {
					append(c);
				}
		}
	}
	append('"');
}

bool WriteBuffer::finish()
{
	if (left) {
		*ptr = 0;
		return true;
	}
	return false;
}

void WriteBuffer::appendSymbol(Symbol s, uint8_t suffix)
{
	appendXmlEscaped(s);
	if (suffix) {
		append('_');
		append('0' + suffix);
	}
}

void WriteBuffer::writeDeleteVectorPacket(const IndiVector & vec)
{
	append(F("<delProperty name=\""));
	appendSymbol(vec.name, vec.nameSuffix);
	append(F("\"/>\n"));
}

void WriteBuffer::startAnnounceVectorPacket(const IndiVector & vec)
{
	append(F("<"));
	append(vec.kind().defVectorText);
}

void WriteBuffer::endAnnounceVectorPacket(const IndiVector & vec)
{
	append(F("</"));
	append(vec.kind().defVectorText);
	append(F(">\n"));
}


void WriteBuffer::startMutateVectorPacket(const IndiVector & vec)
{
	append(F("<"));
	append(vec.kind().newVectorText);
}

void WriteBuffer::endMutateVectorPacket(const IndiVector & vec)
{
	append(F("</"));
	append(vec.kind().newVectorText);
	append(F(">\n"));
}

void WriteBuffer::startUpdateValuesPacket(const IndiVector & vec)
{
}

void WriteBuffer::endUpdateValuesPacket(const IndiVector & vec)
{
}

void WriteBuffer::writeVectorName(Symbol name, uint8_t suffix)
{
	append(F(" name=\""));
	appendSymbol(name, suffix);
	append(F("\""));
}

void WriteBuffer::writeVectorFlag(uint8_t fl)
{
	append(F(" propertyState=\""));
	if (fl & VECTOR_BUSY) {
		append(F("Busy"));
	} else {
		append(F("Idle"));
	}
	append(F("\" propertyPerm=\""));
	if ((fl & VECTOR_WRITABLE) && (fl & VECTOR_READABLE)) {
		append(F("rw"));
	} else if ((fl & VECTOR_READABLE)) {
		append(F("ro"));
	} else {
		append(F("wo"));
	}
	append('"');
}

void WriteBuffer::writeVectorLabel(Symbol name, uint8_t suffix)
{
	append(F(" label=\""));
	appendSymbol(name, suffix);
	append(F("\""));
}

void WriteBuffer::writeVectorUid(uint8_t uid)
{
	append(F(">\n"));
}

void WriteBuffer::startMember(const IndiVector & vec)
{
	append(F("\t<"));
	append(vec.kind().oneMemberText);
}

void WriteBuffer::endMember(const IndiVector & vec)
{
	append(F("</"));
	append(vec.kind().oneMemberText);
	append(F(">\n"));
}

void WriteBuffer::writeVectorMemberSubtype(uint8_t subtype)
{

}

void WriteBuffer::writeVectorMemberName(Symbol name, uint8_t suffix)
{
	writeVectorName(name, suffix);
}

void WriteBuffer::writeVectorMemberLabel(Symbol name, uint8_t suffix)
{
	writeVectorLabel(name, suffix);
	append('>');
}

void WriteBuffer::writeString(const char * str)
{
	appendXmlEscaped(str);
}


void WriteBuffer::writeFloat(float value)
{
	char buffer[32];
	snprintf(buffer, 32, "%.2f", value);
	appendXmlEscaped(buffer);
}


void WriteBuffer::writeInt(int32_t value)
{
	char buffer[32];
	snprintf(buffer, 32, "%ld", value);
	appendXmlEscaped(buffer);
}
