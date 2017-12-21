#ifndef DEWHEATER_H
#define DEWHEATER_H 1

#include <OneWire.h>
#include "Scheduled.h"
#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"
#include "IndiSwitchVector.h"
#include "IndiSwitchVectorMember.h"


class MeteoTemp;
class DewHeater : public Scheduled {
    Symbol group;
    IndiNumberVector statusVec;
    IndiFloatVectorMember temperature;
    IndiFloatVectorMember pwm;
    IndiFloatVectorMember dynTargetTemp;

    IndiTextVector uidVec;
    IndiTextVectorMember uid;

    IndiSwitchVector powerMode;
    IndiSwitchVectorMember powerModeOff, powerModeForced, powerModeByTemp, powerModeOverDew;

    IndiNumberVector targetPwmVec;
    IndiFloatVectorMember targetPwm;

    IndiNumberVector targetTempVec;
    IndiFloatVectorMember targetTemp;

    IndiNumberVector targetTempAboveVec;
    IndiFloatVectorMember targetTempAbove;

    IndiNumberVector settingKpVec;
    IndiFloatVectorMember settingKp;
    IndiNumberVector settingKiVec;
    IndiFloatVectorMember settingKi;
    IndiNumberVector settingKdVec;
    IndiFloatVectorMember settingKd;

    MeteoTemp * meteoTemp;
    OneWire oneWire;
    uint8_t pwmPin;
    uint8_t status;
    uint8_t addr[8];
    bool tempAvailable;
    bool pidRunning;
    bool pidDisabled;

    double lastInput;
    long lastTime;
    double ITerm;

    void powerModeChanged();

    void scan();
    void startMeasure();
    void endMeasure();

    void failed();

    void setControlMode(uint8_t value);

    void setPwmLevel(float level);

    double getPidTarget();
    void initPid();
    void stopPid();
    int updatePid();
public:
    DewHeater(MeteoTemp * meteo, uint8_t tempPin, uint8_t pwmPin, int suffix, uint32_t eepromAddr);

    virtual void tick();
};


#endif
