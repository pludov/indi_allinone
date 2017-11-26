#ifndef DEWHEATER_H
#define DEWHEATER_H 1

#include <OneWire.h>
#include "Scheduled.h"
#include "IndiVectorGroup.h"
#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"
#include "IndiSwitchVector.h"
#include "IndiSwitchVectorMember.h"

class DewHeater : public Scheduled {
    IndiVectorGroup group;
    IndiNumberVector statusVec;
    IndiFloatVectorMember temperature;
    IndiFloatVectorMember pwm;

    IndiTextVector uidVec;
    IndiTextVectorMember uid;

    IndiSwitchVector powerMode;
    IndiSwitchVectorMember powerModeOff, powerModeForced;

    OneWire oneWire;
    uint8_t status;
    uint8_t addr[8];

    void scan();
    void startMeasure();
    void endMeasure();

    void failed();

    void setControlMode(uint8_t value);

    void setPwmLevel(float level);
public:
    DewHeater(int port, int suffix);

    virtual void tick();
};


#endif
