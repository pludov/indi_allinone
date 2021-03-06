#ifdef ARDUINO
#include <Arduino.h>
#else
#define F(a) (a)
#endif

#include "XmlWriteBuffer.h"
#include "IndiVector.h"
#include "Utils.h"

XmlWriteBuffer::XmlWriteBuffer(uint8_t * into, int size): WriteBuffer(into, size)
{}


#ifdef ARDUINO

void XmlWriteBuffer::append(const __FlashStringHelper * str)
{
	PGM_P p = reinterpret_cast<PGM_P>(str);

	while (1) {
		char c = pgm_read_byte(p++);
		if (c == 0) break;
		append(c);
	}
}

void XmlWriteBuffer::append(const Symbol & str)
{
	append(str.base);
	if (str.suffix) {
		append('_');
		append('0' + str.suffix);
	}
}

#else

void XmlWriteBuffer::append(const Symbol & str)
{
	for(int u = 0; u < str.length(); ++u)
	{
		char c = str[u];
		append(c);
	}
}

#endif

void XmlWriteBuffer::appendXmlEscaped(char c) {
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

void XmlWriteBuffer::appendXmlEscaped(const Symbol & str)
{
	PGM_P p = reinterpret_cast<PGM_P>(str.base);
	
	while (1) {
		unsigned char c = pgm_read_byte(p++);
		if (c == 0) break;
		appendXmlEscaped(c);
	}
	if (str.suffix) {
		append('_');
		append('0' + str.suffix);
	}
}

#else

void XmlWriteBuffer::appendXmlEscaped(std::string str)
{
	for(int u = 0; u < str.length(); ++u)
	{
		unsigned char c = str[u];
		appendXmlEscaped(c);
	}
}


#endif

void XmlWriteBuffer::appendXmlEscaped(const char * s)
{
	while(*s) {
		appendXmlEscaped(*(s++));
	}
}

void XmlWriteBuffer::append(const char * s)
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
	

void XmlWriteBuffer::appendSymbol(const Symbol & s)
{
	appendXmlEscaped(s);
}

bool XmlWriteBuffer::supportUpdateValue() const {
	return false;
}
void XmlWriteBuffer::startWelcomePacket()
{
	append(F("<welcome/>"));
}

void XmlWriteBuffer::writeDeleteVectorPacket(const IndiVector & vec)
{
	append(F("<delProperty name=\""));
	appendSymbol(vec.name);
	append(F("\"/>\n"));
}

void XmlWriteBuffer::startAnnounceVectorPacket(const IndiVector & vec)
{
	append(F("<"));
	append(vec.kind().defVectorText);
}

void XmlWriteBuffer::endAnnounceVectorPacket(const IndiVector & vec)
{
	append(F("</"));
	append(vec.kind().defVectorText);
	append(F(">\n"));
}


void XmlWriteBuffer::startMutateVectorPacket(const IndiVector & vec)
{
	append(F("<"));
	append(vec.kind().newVectorText);
}

void XmlWriteBuffer::endMutateVectorPacket(const IndiVector & vec)
{
	append(F("</"));
	append(vec.kind().newVectorText);
	append(F(">\n"));
}

void XmlWriteBuffer::startUpdateValuesPacket(const IndiVector & vec)
{
}

void XmlWriteBuffer::endUpdateValuesPacket(const IndiVector & vec)
{
}

void XmlWriteBuffer::writeVectorName(const Symbol &  name)
{
	append(F(" name=\""));
	appendSymbol(name);
	append(F("\""));
}

void XmlWriteBuffer::writeVectorGroup(const Symbol & group)
{
	append(F(" group=\""));
	appendSymbol(group);
	append(F("\""));
}

void XmlWriteBuffer::writeVectorFlag(uint8_t fl)
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

void XmlWriteBuffer::writeVectorLabel(const Symbol &  name)
{
	append(F(" label=\""));
	appendSymbol(name);
	append(F("\""));
}

void XmlWriteBuffer::writeVectorUid(uint8_t uid)
{
	append(F(">\n"));
}

void XmlWriteBuffer::startMember(const IndiVector & vec)
{
	append(F("\t<"));
	append(vec.kind().oneMemberText);
}

void XmlWriteBuffer::endMember(const IndiVector & vec)
{
	append(F("</"));
	append(vec.kind().oneMemberText);
	append(F(">\n"));
}

void XmlWriteBuffer::writeVectorMemberSubtype(uint8_t subtype)
{

}

void XmlWriteBuffer::writeVectorMemberName(const Symbol &  name)
{
	writeVectorName(name);
}

void XmlWriteBuffer::writeVectorMemberLabel(const Symbol &  name)
{
	writeVectorLabel(name);
	append('>');
}

void XmlWriteBuffer::writeString(const char * str, int maxSize)
{
	appendXmlEscaped(str);
}

void XmlWriteBuffer::writeBool(bool b)
{
	if (b) {
		append(F("true"));
	} else {
		append(F("false"));
	}
}


void XmlWriteBuffer::writeFloat(float value)
{
	char buffer[32];
	snprintf(buffer, 32, "%.2f", value);
	appendXmlEscaped(buffer);
}


void XmlWriteBuffer::writeInt(int32_t value)
{
	char buffer[32];
	snprintf(buffer, 32, "%" PRIX32, value);
	appendXmlEscaped(buffer);
}
