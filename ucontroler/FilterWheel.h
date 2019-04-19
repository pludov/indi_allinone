#ifndef FILTERWHEEL
#define FILTERWHEEL_H_

#include "Motor.h"
#include "Scheduled.h"
#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"
#include "IndiIntVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"
#include "IndiSwitchVector.h"
#include "IndiSwitchVectorMember.h"

class BaseDriver;

#define FILTER_SLOT_COUNT 5

#define CALIBRATION_IDLE 0
#define CALIBRATION_WAITING_0 1
#define CALIBRATION_WAITING_1 2

class FilterWheel: public Motor {
    Symbol group;
    IndiNumberVector filterSlotVec;
    IndiIntVectorMember filterSlot;

    IndiNumberVector rawPosVec;
    IndiIntVectorMember rawPos;

    IndiSwitchVector calibrateVec;
    IndiSwitchVectorMember calibrate;

    IndiSwitchVector abortMotionVec;
    IndiSwitchVectorMember abortMotion;

    IndiNumberVector filterPositionsVec;
    IndiIntVectorMember * filterPositions[FILTER_SLOT_COUNT];

    IndiTextVector filterNamesVec;
    IndiTextVectorMember * filterNames[FILTER_SLOT_COUNT];

    IndiNumberVector hallVec;
    IndiIntVectorMember hall;

    void rawPosChanged();
    void filterSlotChanged();
    void calibrateChanged();
    void abortChanged();

    uint8_t currentCalibration;

    bool readPin();
public:
    // 4 pins for motor + sensor
    FilterWheel(BaseDriver * bd, uint32_t addr, const uint8_t * pins, int suffix);
    virtual void onProgress();
    virtual void onTargetPositionReached();

};

#endif
