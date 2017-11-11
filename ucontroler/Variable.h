/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef VARIABLE_H_
#define VARIABLE_H_

#include "Scheduled.h"


#define VECNONE ((uint8_t)-1)

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

struct DirtyVector;

class DeviceWriter : public Scheduled 
{
	friend class Vector;
	char * notifPacket;
	Stream * serial;
	char * writeBuffer;
	int writeBufferLeft;
	
	uint8_t clientId;
	DeviceWriter * next;

	// Linked list of dirty Vectors
	uint8_t * nextDirtyVector;
	uint8_t firstDirtyVector;
	uint8_t lastDirtyVector;

	void fillBuffer();
	void dirtied(Vector * which);
	void popDirty(DirtyVector & result);
	
public:
	DeviceWriter(Stream * target);
	void tick();
};

class Device {
	friend class DeviceWriter;
	Vector ** list;
	int variableCount;
	DeviceWriter * firstWriter;
protected:
	friend class Vector;
	void add(Vector * v);
public:
	Device(int variableCount);
	void dump(WriteBuffer & into);

	static Device & instance();
};

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

#define VECTOR_ANNOUNCED 0
#define VECTOR_MUTATION 1
#define VECTOR_VALUE 2
#define VECTOR_COMM_COUNT 3


class Vector {
	friend class DeviceWriter;
	friend class Device;
	friend class Member;
	const __FlashStringHelper * name;
	int8_t nameSuffix;
	const __FlashStringHelper * label;

	Group * group;
	Member * first, * last;
	
	uint8_t flag;

	// 2 bits for each writer : announced, updated
	uint8_t notifStatus[VECTOR_COMM_COUNT];
	
	int8_t uid;

	/** 
	 * clientId : DeviceWriter id
	 * commId : VECTOR_ANNOUNCED, VECTOR_UPDATED, ...
	 */
	bool isDirty(uint8_t clientId, uint8_t commId);
	bool cleanDirty(uint8_t clientId, uint8_t commId);

	void notifyUpdate(uint8_t commId);
	
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
