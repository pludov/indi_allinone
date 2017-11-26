#include <Arduino.h>

#include "DewHeater.h"
#include "Utils.h"
#include "CommonUtils.h"
#define STATUS_NEED_SCAN 0
#define STATUS_MEASURE 1
#define STATUS_IDLE 2

#define CONTROL_OFF 0
#define CONTROL_PWM_LEVEL 1
#define CONTROL_TARGET_TEMP 2
#define CONTROL_DEW_POINT_DELTA 2

DewHeater::DewHeater(uint8_t pin, uint8_t pwmPin, int suffix)
    :Scheduled(F("DewHeater")),
    group(Symbol(F("DEW_HEATER"), suffix)),

	statusVec(&group, Symbol(F("DEW_HEATER_TEMP"), suffix), F("HW Temperature")),
    temperature(&statusVec, F("DEW_HEATER_TEMP_VALUE"), F("Readen Temperature (°C)"),-273.15, 100, 1),
    pwm(&statusVec, F("DEW_HEATER_PWM_LEVEL"), F("Power applied (%)"),0, 100, 1),

    uidVec(&group, Symbol(F("DEW_HEATER_UID"), suffix), F("Unique Identifier")),
    uid(&uidVec, F("DEW_HEATER_UID_VALUE"), F("Unique Identifier"),12),

	powerMode(&group, Symbol(F("POWER_MODE"), suffix), F("Power Mode"), VECTOR_WRITABLE|VECTOR_READABLE),
	powerModeOff(&powerMode, F("POWER_MODE_OFF"), F("Off")),
	powerModeForced(&powerMode, F("POWER_MODE_FORCED"), F("Forced")),

    oneWire(pin)
{
    this->priority = 2;
    this->status = 0;
    this->pwmPin = pwmPin;
    this->failed();
    this->nextTick = UTime::now();
    pinMode(pwmPin, OUTPUT);
    digitalWrite(pwmPin, 0);
}

// void setControlMode(uint8_t value)
// {
//     switch(value) {
//         case CONTROL_OFF:
//             setPwmLevel(0);
//             break;
//         case CONTROL_PWM_LEVEL:

//     }
// }

void DewHeater::setPwmLevel(float level)
{
    pwm.setValue(level);
}

void DewHeater::failed()
{
    uid.setValue("");
    // Shutdown
    setPwmLevel(0);
    this->status = STATUS_NEED_SCAN;
    this->nextTick = UTime::now() + MS(1000);
}

static void formatAddr(byte * addr, char * buffer)
{
    addr++;
    for(int i = 0; i < 6; ++i)
    {
        uint8_t v = addr[i];
        *(buffer++) = Utils::hex(v >> 4);
        *(buffer++) = Utils::hex(v & 15);
    }
    *buffer = 0;
}

void DewHeater::scan()
{
    DEBUG(F("scan"));
    
    oneWire.reset_search();
    if (!oneWire.search(addr)) {
        failed();
        return;
    }
    if (OneWire::crc8(addr, 7) != addr[7]) {
        failed();
        return;
    }
    if (addr[0] != 0x28) {
        // Mauvais type de capteur
        failed();
        return;
    }
    
    oneWire.select(addr);

    char strAddr[32];
    formatAddr(addr, strAddr);
    uid.setValue(strAddr);
    // FIXME: update variables from scan...
    
    DEBUG(F("scan ok at "), strAddr);
    
    this->status = STATUS_IDLE;
    this->nextTick = UTime::now() + MS(100);
}

void DewHeater::startMeasure()
{
    long t1 = micros();
    DEBUG(F("startMeasure"));
    oneWire.reset();
    oneWire.skip();
    oneWire.write(0x44, 1);
    long t2 = micros();
    this->status = STATUS_MEASURE;
    this->nextTick = UTime::now() + MS(800);
    DEBUG(F("startmeasure done in "), t2 - t1);
}

void DewHeater::endMeasure()
{
    DEBUG(F("endMeasure"));
    long t1 = micros();
    oneWire.reset();
    oneWire.skip();
    //oneWire.select(addr);
    oneWire.write(0xBE);
    
    byte data[9];
    for (byte i = 0; i < 9; i++) {
        data[i] = oneWire.read();
    }
    long t2 = micros();
    DEBUG(F("measure done in "), t2 - t1);

    if (OneWire::crc8(data, 8) != data[8]) {
        DEBUG(F("Invalid data"));
        failed();
        return;
    }

    temperature.setValue(((data[1] << 8) | data[0]) * 0.0625);

    this->status = STATUS_IDLE;
    this->nextTick = UTime::now() + MS(5000);
}

void DewHeater::tick()
{
    if (this->status == STATUS_NEED_SCAN) {
        scan();
    } else {
        if (this->status == STATUS_IDLE) {
            startMeasure();
        } else {
            endMeasure();
        }
    }
}
