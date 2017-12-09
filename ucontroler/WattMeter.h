#ifndef WATTMETER_H
#define WATTMETER_H 1

#include "Scheduled.h"
#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"
#include "IndiSwitchVector.h"
#include "IndiSwitchVectorMember.h"

class WattMeter : public Scheduled {
    Symbol group;
    IndiNumberVector statusVec;
    IndiFloatVectorMember v;
    IndiFloatVectorMember a;

    IndiNumberVector vBiasVec;
    IndiFloatVectorMember vBias;
    IndiNumberVector vMultVec;
    IndiFloatVectorMember vMult;

    IndiNumberVector aBiasVec;
    IndiFloatVectorMember aBias;
    IndiNumberVector aMultVec;
    IndiFloatVectorMember aMult;

    uint8_t aPin;
    uint8_t vPin;
    uint8_t cpt;
    int16_t lastAVal;
    int16_t lastVVal;
    int aval, vval;

    void scan();
    void startMeasure();
    void endMeasure();

    void failed();

    void setControlMode(uint8_t value);

    void updateValues();

    void setPwmLevel(float level);
public:
    WattMeter(uint8_t vPin, uint8_t aPin, int suffix);

    virtual void tick();
};


#endif
