#ifndef DEWHEATER_H
#define DEWHEATER_H 1

#include <OneWire.h>
#include "Scheduled.h"
#include "IndiVectorGroup.h"
#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"

class DewHeater : public Scheduled {
    IndiVectorGroup group;
    IndiNumberVector temperatureVec;
    IndiFloatVectorMember temperature;
    IndiTextVector uidVec;
    IndiTextVectorMember uid;

    OneWire oneWire;
    uint8_t status;
    byte addr[8];

    void scan();
    void startMeasure();
    void endMeasure();

    void failed();
public:
    DewHeater(int port, int suffix);

    virtual void tick();
};


#endif