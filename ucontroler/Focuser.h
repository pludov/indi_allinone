#ifndef FOCUSER_H_
#define FOCUSER_H_

#include "Motor.h"
#include "Scheduled.h"
#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"
#include "IndiIntVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"
#include "IndiSwitchVector.h"
#include "IndiSwitchVectorMember.h"


class Focuser: public Motor {
    Symbol group;
    IndiNumberVector absolutePositionVec;
    IndiIntVectorMember absolutePosition;

    void absPositionChanged();
public:
    Focuser(const uint8_t * pins, int suffix);
    virtual void onProgress();
    virtual void onTargetPositionReached();

};

#endif
