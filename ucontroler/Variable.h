/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef VARIABLE_H_
#define VARIABLE_H_

#include "Scheduled.h"

class WriteBuffer {
	char * ptr;
	int left;
	int totalSize;
public:
	WriteBuffer(char * into, int size);

	void append(char c);
	void append(const char * s);
	void append(const __FlashStringHelper * s);

	void appendXmlEscaped(char c);
	void appendXmlEscaped(const __FlashStringHelper * s);
	void appendXmlEscaped(const char * s);
	bool finish();
	int size();
};

class Vector;

class DeviceWriter : public Scheduled 
{
	char * notifPacket;
	Stream * serial;
	char * writeBuffer;
	int writeBufferLeft;
	int lastVector;
	uint8_t clientId;

	void fillBuffer();
public:
	DeviceWriter(Stream * target, int8_t id);
	void tick();
};

class Device {
	friend class DeviceWriter;
	Vector ** list;
	int variableCount;
protected:
	friend class Vector;
	void add(Vector * v);
public:
	Device(int variableCount);
	void dump(WriteBuffer & into);

	static Device & instance();
};

extern Device & getDevice();

class Group {
	friend class Vector;
	const __FlashStringHelper * name;
public:
	Group(const __FlashStringHelper * name);
};

class Member;

#define VECTOR_WRITABLE 1
#define VECTOR_READABLE 2
#define VECTOR_BUSY 4

class Vector {
	friend class DeviceWriter;
	friend class Member;
	const __FlashStringHelper * name;
	int8_t nameSuffix;
	const __FlashStringHelper * label;

	Group * group;
	Member * first, * last;
	
	uint8_t flag;

	int8_t announced;
	int8_t updated;
	int8_t uid;

	void notifyUpdate();
	
public:
	Vector(Group * parent, const __FlashStringHelper * name, const __FlashStringHelper * label);

	void set(uint8_t flag, bool status);
	
	virtual void dump(WriteBuffer & into);
};

class Member {
	friend class Vector;
	const __FlashStringHelper * name;
	const __FlashStringHelper * label;
	int value;
	int min, max;
	Vector * vector;
	Member * next;
public:
	Member(Vector * vector, 
			const __FlashStringHelper * name, 
			const __FlashStringHelper * label,
			int min, int max);

	void setValue(int v);

	virtual void dump(WriteBuffer & into, int8_t nameSuffix);
};

#endif /* STATUS_H_ */
