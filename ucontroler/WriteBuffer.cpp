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

