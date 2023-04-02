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


struct MeasureSequence;

class MeteoTemp;
class DewHeater : public Scheduled {
    Symbol group;
    IndiNumberVector statusVec;
    IndiFloatVectorMember temperature;
    IndiFloatVectorMember pwm;
    IndiFloatVectorMember dynTargetTemp;

    IndiTextVector uidVec;
    IndiTextVectorMember uid;

    IndiTextVector nameVec;
	IndiTextVectorMember name;

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

    IndiSwitchVector configOperationVec;
    IndiSwitchVectorMember configIdle, configSave, configForget, configReload;


    MeteoTemp * meteoTemp;
    OneWire oneWire;
    uint8_t pwmPin;
    uint8_t status;
    uint8_t addr[8];
    uint8_t readBuffer[9];
    uint8_t readBufferPos;
    bool tempAvailable;
    bool pidRunning;
    bool pidDisabled;

    double lastInput;
    long lastTime;
    double ITerm;

    static MeasureSequence * measureSequence();

    void updatePwm();
    void powerModeChanged();
    void configOperationChanged();

    void scan();
    bool startMeasure();
    bool endMeasure();
    bool readMeasure();

    void failed();

    void setControlMode(uint8_t value);

    void setPwmLevel(float level);

    double getPidTarget();
    void initPid();
    void stopPid();
    int updatePid();
    uint8_t getPowerModeId() const;
    void setPowerModeId(uint8_t v);

    void saveToEeprom();
    void loadFromEeprom();
public:
    DewHeater(MeteoTemp * meteo, uint8_t tempPin, uint8_t pwmPin, int suffix);

    virtual void tick();
};


#endif
