#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "WattMeter.h"
#include "Utils.h"
#include "CommonUtils.h"
#define STATUS_NEED_SCAN 0
#define STATUS_MEASURE 1
#define STATUS_IDLE 2

#define CONTROL_OFF 0
#define CONTROL_PWM_LEVEL 1
#define CONTROL_TARGET_TEMP 2
#define CONTROL_DEW_POINT_DELTA 2

WattMeter::WattMeter(uint8_t vPin, uint8_t aPin, int suffix)
    :Scheduled(F("WattMeter")),
    group(Symbol(F("Watt Meter"), suffix)),

	statusVec(group, Symbol(F("WATT_METTER_MEASURE"), suffix), F("Measure")),
    v(&statusVec, F("WATT_METTER_MEASURE_V"), F("Volt"),0, 160, 1),
    a(&statusVec, F("WATT_METTER_MEASURE_A"), F("Ampere"),0, 10, 1)
{
    this->priority = 2;
    this->aPin = aPin;
    this->vPin = vPin;
    this->nextTick = UTime::now();
    this->cpt = 0;
    this->aval = 0;
    this->vval = 0;
//     powerMode.onRequested(VectorCallback(&DewHeater::powerModeChanged, this));
}

void WattMeter::tick()
{
	// 50hz = 20ms
    this->nextTick = UTime::now() + MS(20);
    long l = micros();
    analogReadResolution(12);
    int nvaval = analogRead(aPin);
    int nvvval = analogRead(vPin);
    l = micros() - l;
    DEBUG(F("Read done in "),l);

    aval += nvaval;
    vval += nvvval;
    cpt++;
    if (cpt == 50) {
    	aval /= 50;
    	vval /= 50;
    	a.setValue(aval);
   	    v.setValue(vval);
   	    aval = 0;
   	    vval = 0;
   	    cpt = 0;
    }
}
