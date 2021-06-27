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
#include "EepromStored.h"

class BaseDriver;

#define FILTER_SLOT_COUNT 7

#define CALIBRATION_IDLE 0
#define CALIBRATION_WAITING_0 1
#define CALIBRATION_WAITING_1 2

class FilterWheelMemory;

class FilterWheel: public Motor {
    friend class FilterWheelMemory;
    Symbol group;
    IndiNumberVector filterSlotVec;
    IndiIntVectorMember filterSlot;

    IndiNumberVector rawPosVec;
    IndiIntVectorMember rawPos;

    IndiNumberVector motorSettingsVec;
    IndiIntVectorMember motorPulse;
    IndiIntVectorMember motorBacklash;

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

    EepromReadyListener eepromReadyListener;

    void rawPosChanged();
    void motorSettingsChanged();
    void filterSlotChanged();
    void calibrateChanged();
    void abortChanged();

    uint8_t currentCalibration;
    FilterWheelMemory * memory;
    bool readPin();
    void loadInitialSettings();
    void saveMemoryPos(uint32_t value);
public:
    // 4 pins for motor + sensor
    FilterWheel(BaseDriver * bd, uint32_t addr, const uint8_t * pins, int suffix);
    virtual void onProgress();
    virtual void onTargetPositionReached();

};

#endif
