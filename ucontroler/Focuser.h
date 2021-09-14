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
#include "EepromStored.h"

class BaseDriver;
class FocuserMemory;

/**
 * Drive a focus motor using 4 digital PINS (ex: unipolar BYJ48) and expose the corresponding INDI interface
 */
class Focuser: public Motor {
    Symbol group;
    IndiNumberVector absolutePositionVec;
    IndiIntVectorMember absolutePosition;
    IndiSwitchVector focusAbortMotionVec;
    IndiSwitchVectorMember focusAbortMotion;
    IndiSwitchVector focusResetPositionVec;
    IndiSwitchVectorMember focusResetPosition;

    EepromReadyListener eepromReadyListener;

    void absPositionChanged();
    void abortChanged();
    void resetPositionChanged();
    void loadInitialSettings();

    FocuserMemory * memory;
public:
    /** pins needs to contains 4 digital pins */
    Focuser(BaseDriver * bd, uint32_t addr, const uint8_t * pins, int suffix);
    virtual void onProgress();
    virtual void onTargetPositionReached();

};

#endif
