/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "Variable.h"

// a 115200, on transmet un caractère (9 bits), en: 1000000 / (115200 / 9) us
#define CHAR_XMIT_DELAY 79

#define NOTIF_PACKET_MAX_SIZE 2048

static Device * mainDevice = 0;

Device & Device::instance()
{
	if (!mainDevice) {
		mainDevice = new Device(100);
	}
	return *mainDevice;
}


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

void WriteBuffer::append(const __FlashStringHelper * str)
{
	PGM_P p = reinterpret_cast<PGM_P>(str);

	while (1) {
	  unsigned char c = pgm_read_byte(p++);
	  if (c == 0) break;
	  append(c);
	}
}

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

void WriteBuffer::appendXmlEscaped(const __FlashStringHelper * str)
{
	PGM_P p = reinterpret_cast<PGM_P>(str);
	
	while (1) {
		unsigned char c = pgm_read_byte(p++);
		if (c == 0) break;
		appendXmlEscaped(c);
	}
}

void WriteBuffer::appendXmlEscaped(const char * s)
{
	while(*s) {
		appendXmlEscaped(*(s++));
	}
}

static char hex(uint8_t v)
{
	if (v < 10) {
		return '0' + v;
	}
	return 'a' - 10 + v;
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
					append(hex(c >> 4));
					append(hex(c & 15));
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

DeviceWriter::DeviceWriter(Stream * target, int8_t id)
{
	lastVector = -1;
	this->serial = target;
	this->notifPacket = (char*)malloc(NOTIF_PACKET_MAX_SIZE);
	this->writeBuffer = 0;
	this->writeBufferLeft = 0;
	this->clientId = 1 << id;
	this->priority = 100;
	this->nextTick = UTime::now();
}

void DeviceWriter::fillBuffer()
{
	// Il y a de la place, on n'a rien à dire...
	// PArcours la liste des variables (ouch, on peut mieux faire là...)
	Device & device = Device::instance();
	for(int i = 0; i < device.variableCount; ++i)
	{
		lastVector++;
		if (lastVector == device.variableCount) {
			lastVector = 0;
		}

		Vector * v = device.list[lastVector];
		// FIXME: interrupt safe ?
		if (!(v->announced & clientId)) {
			WriteBuffer wf(notifPacket, NOTIF_PACKET_MAX_SIZE);
			v->announced |= clientId;
			v->updated |= clientId;

			v->dump(wf);
			if (wf.finish()) {
				writeBuffer = notifPacket;
				writeBufferLeft = wf.size();
				return;
			} else {
				// WTF ? on peut rien faire... on oublie
			}
		}

		if (!(v->updated & clientId)) {
			WriteBuffer wf(notifPacket, NOTIF_PACKET_MAX_SIZE);
			v->updated |= clientId;

			v->dump(wf);
			if (wf.finish()) {
				writeBuffer = notifPacket;
				writeBufferLeft = wf.size();
				return;
			} else {
				// WTF ? on peut rien faire... on oublie
			}
		}
	}
}

void DeviceWriter::tick()
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

Device::Device(int variableCount)
{
	list = (Vector**)malloc(sizeof(Vector*)*variableCount);
	this->variableCount = 0;
}

void Device::add(Vector * v)
{
	list[variableCount++]=v;
}

void Device::dump(WriteBuffer & into)
{
	for(int i = 0; i < variableCount; ++i)
	{
		Vector * cur = list[i];
		cur->dump(into);
	}
}


Group::Group(const __FlashStringHelper * name)
{
	this->name = name;
}

Vector::Vector(Group * group, const __FlashStringHelper * name, const __FlashStringHelper * label)
{
	this->group = group;
	this->name = name;
	this->label = label;
	this->first = 0;
	this->last = 0;
	this->nameSuffix = 0;
	this->flag = 0;
	Device::instance().add(this);
}

void Vector::notifyUpdate()
{
	this->updated = 0;
}

void Vector::set(uint8_t flag, bool status)
{
	uint8_t newFlag = (this->flag & ~flag);
	if (status) newFlag |= flag;
	if (this->flag != newFlag) {
		notifyUpdate();
	}
}


void Vector::dump(WriteBuffer & into)
{
	into.append(F("<defNumberVector name=\""));
	into.appendXmlEscaped(name);
	if (nameSuffix) {
		into.append('_');
		into.append(nameSuffix);
	}
	into.append(F("\" label=\""));
	into.appendXmlEscaped(label);
	into.append(F("\" group=\""));
	into.appendXmlEscaped(group->name);
	into.append(F("\" state=\"Idle\" perm=\"ro\">\n"));
	for(Member * cur = first; cur; cur=cur->next)
	{
		into.append('\t');
		cur->dump(into, nameSuffix);
		into.append('\n');
	}
	into.append(F("</defNumberVector>\n"));
}

Member::Member(Vector * vector, 
	const __FlashStringHelper * name, 
	const __FlashStringHelper * label,
	int min,
	int max)
{
	this->vector = vector;
	next = 0;
	if (vector->last) {
		vector->last->next = this;
	} else {
		vector->first = this;
	}
	vector->last = this;
	this->name = name;
	this->label = label;
	this->min = min;
	this->max = max;
	this->value = 1;
}

void Member::setValue(int newValue)
{
	if (value == newValue) return;
	value = newValue;
	vector->notifyUpdate();
}

void Member::dump(WriteBuffer & into, int8_t nameSuffix)
{
	into.append(F("<defNumber name=\""));
	into.appendXmlEscaped(name);
	if (nameSuffix) {
		into.append('_');
		into.append(nameSuffix);
	}
	into.append(F("\" label=\""));
	into.appendXmlEscaped(label);
	into.append(F("\" format=\"%.0f\">"));
	char buffer[32];
	snprintf(buffer, 32, "%d", value);
	into.appendXmlEscaped(buffer);
	into.append(F("</defNumber>"));
}