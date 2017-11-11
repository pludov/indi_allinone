/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "IndiDevice.h"
#include "IndiProtocol.h"
#include "IndiVectorGroup.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"

// a 115200, on transmet un caractère (9 bits), en: 1000000 / (115200 / 9) us
#define CHAR_XMIT_DELAY 79

#define NOTIF_PACKET_MAX_SIZE 2048

static IndiDevice * mainDevice = 0;

IndiDevice & IndiDevice::instance()
{
	if (!mainDevice) {
		mainDevice = new IndiDevice(100);
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

IndiProtocol::IndiProtocol(Stream * target)
{
	IndiDevice & device = IndiDevice::instance();
	
	this->serial = target;
	this->notifPacket = (char*)malloc(NOTIF_PACKET_MAX_SIZE);
	this->writeBuffer = 0;
	this->writeBufferLeft = 0;
	this->priority = 100;
	this->nextTick = UTime::now();

	this->next = device.firstWriter;
	this->clientId = this->next ? this->next->clientId + 1 : 0;
	device.firstWriter = this;

	if (device.variableCount) {
		this->nextDirtyVector = (uint8_t*)malloc(device.variableCount);
		for(int i = 0; i < device.variableCount -1; ++i) {
			this->nextDirtyVector[i] = i + 1;
		}
		this->nextDirtyVector[device.variableCount - 1] = VECNONE;
		this->firstDirtyVector = 0;
		this->lastDirtyVector = device.variableCount - 1;
	} else {
		this->firstDirtyVector = 0;
		this->lastDirtyVector = 0;
		this->nextDirtyVector = 0;
	}
}

void IndiProtocol::dirtied(IndiVector* vector)
{
	uint8_t which = vector->uid;
	if (which == VECNONE) return;

	// Si il est dans la liste:
	//  - soit c'est le dernier
	//  - soit il a un suivant
	if (this->lastDirtyVector == which || this->nextDirtyVector[which] != VECNONE) {
		return;
	}

	// C'est notre nouveau dernier
	if (this->lastDirtyVector != VECNONE) {
		this->nextDirtyVector[this->lastDirtyVector] = which;
		this->lastDirtyVector = which;
	} else {
		this->firstDirtyVector = which;
		this->lastDirtyVector = which;
	}
}


struct DirtyVector {
	IndiVector * vector;
	uint8_t dirtyFlags;
};


void IndiProtocol::popDirty(DirtyVector & result)
{
	uint8_t vectorId = this->firstDirtyVector;
	result.dirtyFlags = 0;
	if (vectorId != VECNONE) {
		IndiVector * v = IndiDevice::instance().list[vectorId];
		result.vector = v;
		for(int i = 0; i < VECTOR_COMM_COUNT; ++i) {
			if (v->cleanDirty(clientId, i)) {
				result.dirtyFlags |= (1 << i);
			}
		}
		
		this->firstDirtyVector = this->nextDirtyVector[vectorId];
		this->nextDirtyVector[vectorId] = VECNONE;
		if (this->firstDirtyVector == VECNONE) {
			this->lastDirtyVector = VECNONE;
		}
	} else {
		result.vector = 0;
	}
}

void IndiProtocol::fillBuffer()
{
	// Il y a de la place, on n'a rien à dire...
	// PArcours la liste des variables (ouch, on peut mieux faire là...)
	DirtyVector toSend;
	popDirty(toSend);
	if (!toSend.vector) {
		return;
	}

	// FIXME: depending on dirtyFlags, ...

	WriteBuffer wf(notifPacket, NOTIF_PACKET_MAX_SIZE);

	toSend.vector->dump(wf);
	if (wf.finish()) {
		writeBuffer = notifPacket;
		writeBufferLeft = wf.size();
		return;
	} else {
		// WTF ? on peut rien faire... on oublie
	}
}

void IndiProtocol::tick()
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

IndiDevice::IndiDevice(int variableCount)
{
	list = (IndiVector**)malloc(sizeof(IndiVector*)*variableCount);
	this->variableCount = 0;
	this->firstWriter = 0;
}

void IndiDevice::add(IndiVector * v)
{
	v->uid = 255;
	if (firstWriter) {
		Serial.println(F("vector added after writer : not supported"));
		return;
	}
	if (variableCount >= 254) {
		Serial.println(F("Limit of 254 vector reached"));
		return;
	}
	v->uid = variableCount;
	list[variableCount++]=v;
}

void IndiDevice::dump(WriteBuffer & into)
{
	for(int i = 0; i < variableCount; ++i)
	{
		IndiVector * cur = list[i];
		cur->dump(into);
	}
}


IndiVectorGroup::IndiVectorGroup(const __FlashStringHelper * name)
{
	this->name = name;
}

IndiVector::IndiVector(IndiVectorGroup * group, const __FlashStringHelper * name, const __FlashStringHelper * label)
{
	this->group = group;
	this->name = name;
	this->label = label;
	this->first = 0;
	this->last = 0;
	this->nameSuffix = 0;
	this->flag = 0;
	IndiDevice::instance().add(this);
}

void IndiVector::notifyUpdate(uint8_t which)
{
	IndiDevice & device = IndiDevice::instance();
	notifStatus[which] = 0;

	for(IndiProtocol * dw = device.firstWriter; dw; dw = dw->next)
	{
		Serial.println("dirtied");
		dw->dirtied(this);
	}
}

bool IndiVector::isDirty(uint8_t clientId, uint8_t commId)
{
	// Dirty means bit is set to 0
	return (notifStatus[commId] & (1 << clientId)) == 0;
}

bool IndiVector::cleanDirty(uint8_t clientId, uint8_t commId)
{
	// Set bit to 1
	uint8_t mask = 1 << clientId;
	uint8_t val = notifStatus[commId];
	if (!(val & mask)) {
		val |= mask;
		return true;
	}
	return false;
}


void IndiVector::set(uint8_t flag, bool status)
{
	uint8_t newFlag = (this->flag & ~flag);
	if (status) newFlag |= flag;
	if (this->flag != newFlag) {
		notifyUpdate(VECTOR_MUTATION);
	}
}


void IndiVector::dump(WriteBuffer & into)
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
	for(IndiVectorMember * cur = first; cur; cur=cur->next)
	{
		into.append('\t');
		cur->dump(into, nameSuffix);
		into.append('\n');
	}
	into.append(F("</defNumberVector>\n"));
}

IndiVectorMember::IndiVectorMember(IndiVector * vector, 
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

void IndiVectorMember::setValue(int newValue)
{
	if (value == newValue) return;
	value = newValue;
	vector->notifyUpdate(VECTOR_VALUE);
}

void IndiVectorMember::dump(WriteBuffer & into, int8_t nameSuffix)
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