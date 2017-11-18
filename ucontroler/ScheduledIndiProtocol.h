#ifndef SCHEDULEDINDIPROTOCOL_H_
#define SCHEDULEDINDIPROTOCOL_H_ 1
#include "IndiProtocol.h"
#include "Scheduled.h"

class ScheduledIndiProtocol : public IndiProtocol, public Scheduled
{
public:
    ScheduledIndiProtocol(Stream * target);
    
    virtual void tick();
    

};


#endif