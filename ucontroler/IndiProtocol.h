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

class IndiVector;
struct DirtyVector;

class IndiProtocol : public Scheduled
{
	friend class IndiVector;
	char * notifPacket;
	Stream * serial;
	char * writeBuffer;
	int writeBufferLeft;
	
	uint8_t clientId;
	IndiProtocol * next;

	// Linked list of dirty Vectors
	uint8_t * nextDirtyVector;
	uint8_t firstDirtyVector;
	uint8_t lastDirtyVector;

	void fillBuffer();
	void dirtied(IndiVector * which);
	void popDirty(DirtyVector & result);
	
public:
	IndiProtocol(Stream * target);
	void tick();
};


#endif /* INDIPROTOCOL_H_ */
