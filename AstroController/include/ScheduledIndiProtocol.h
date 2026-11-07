#ifndef SCHEDULEDINDIPROTOCOL_H_
#define SCHEDULEDINDIPROTOCOL_H_ 1
#include "IndiProtocol.h"
#include "Scheduled.h"
#include "BinSerialWriteBuffer.h"

class ScheduledIndiProtocol : public IndiProtocol, public Scheduled
{
protected:
    ::Stream * serial;
public:
    ScheduledIndiProtocol(::Stream * target);
    
    virtual void onIncomingPacketReady();
    virtual void onAckPacketBufferEmpty();

    virtual void handleIncomingPacket(uint8_t * packet, int packetSize, BinSerialWriteBuffer & answer);

    virtual void tick();

    virtual void idle();
};


#endif