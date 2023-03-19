#ifndef INDIPROTOCOL_H_
#define INDIPROTOCOL_H_

/*#include "Scheduled.h"*/
#include "WriteBuffer.h"

class IndiVector;
struct DirtyVector;
class IndiDeviceMutator;

class IndiProtocol
{
protected:
	friend class IndiVector;
	uint8_t * notifPacket;
	uint8_t * ackPacket;
	uint8_t * requestPacket;
	
	uint8_t * writeBuffer;
	int writeBufferLeft;
	
	uint8_t clientId;
	IndiProtocol * next;


	uint8_t * incomingPacket;
	int incomingPacketSize;
	// Set to true as soon as a complete packet is received.
	bool incomingPacketReady;
	// When an ack is ready, > 0.
	int ackPacketSize;
	// When a request is ready, > 0
	int requestPacketSize;

	// First message must be a restarted packet.
	bool welcomed;

	// Linked list of dirty Vectors
	uint8_t * nextDirtyVector;
	uint8_t firstDirtyVector;
	uint8_t lastDirtyVector;

	void fillBuffer();
	void popDirty(DirtyVector & result);
	
	

	// Called when incomingPacketReady is set to true
	virtual void onIncomingPacketReady() {};
	// When the ackPAcketBuffer was just written
	virtual void onAckPacketBufferEmpty() {};
	// When the requestPacketBuffer was just written
	virtual void onRequestPacketBufferEmpty() {};
public:
	IndiProtocol();

	int getClientId() const { return clientId; }

	virtual IndiDeviceMutator * getMutator() { return nullptr; };

	// Clear any pending write/ack/read/... and restart the announce process
	void reset();

	// Something arrived
	void received(uint8_t value);

	void dirtied(IndiVector * which);
};


#endif /* INDIPROTOCOL_H_ */
