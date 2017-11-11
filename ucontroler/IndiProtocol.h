/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIPROTOCOL_H_
#define INDIPROTOCOL_H_

#include "Scheduled.h"
#include "WriteBuffer.h"

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


#endif /* INDIPROTOCOL_H_ */
