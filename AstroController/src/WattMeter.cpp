#include <Arduino.h>

#include "WattMeter.h"
#include "Utils.h"
#include "CommonUtils.h"
#include "EepromStored.h"
#include "IndiVectorMemberStorage.h"

#define STATUS_NEED_SCAN 0
#define STATUS_MEASURE 1
#define STATUS_IDLE 2

#define CONTROL_OFF 0
#define CONTROL_PWM_LEVEL 1
#define CONTROL_TARGET_TEMP 2
#define CONTROL_DEW_POINT_DELTA 2

WattMeter::WattMeter(uint8_t vPin, uint8_t aPin, int suffix, uint32_t addr)
    :Scheduled(F("WattMeter")),
    group(Symbol(F("Watt Meter"), suffix)),
	statusVec(group, Symbol(F("WATT_METER_MEASURE"), suffix), F("Measure")),
    v(&statusVec, F("WATT_METER_VOLTAGE"), F("Voltage"),0, 160, 1),
    a(&statusVec, F("WATT_METER_CURRENT"), F("Current"),0, 10, 1),

	vBiasVec(group, Symbol(F("WATT_METER_S_VOLTAGE_BIAS"), suffix), F("Voltage Bias"), VECTOR_READABLE | VECTOR_WRITABLE),
	vBias(&vBiasVec, F("WATT_METER_VOLTAGE_BIAS"), F("Voltage Bias"),-8192, 8192, 1),

	vMultVec(group, Symbol(F("WATT_METER_S_VOLTAGE_MULT"), suffix), F("Voltage Mult"), VECTOR_READABLE | VECTOR_WRITABLE),
	vMult(&vMultVec, F("WATT_METER_VOLTAGE_MULT"), F("Voltage Mult"),0, 100, 1),

	aBiasVec(group, Symbol(F("WATT_METER_S_CURRENT_BIAS"), suffix), F("Current Bias"), VECTOR_READABLE | VECTOR_WRITABLE),
	aBias(&aBiasVec, F("WATT_METER_CURRENT_BIAS"), F("Current Bias"),-8192, 8192, 1),

	aMultVec(group, Symbol(F("WATT_METER_S_CURRENT_MULT"), suffix), F("Current Mult"), VECTOR_READABLE | VECTOR_WRITABLE),
	aMult(&aMultVec, F("WATT_METER_CURRENT_MULT"), F("Current Mult"),0, 100, 1),


	lastAVal(-1),
	lastVVal(-1)
{
    this->priority = 2;
    this->aPin = aPin;
    this->vPin = vPin;
    this->nextTick = UTime::now();
    this->cpt = 0;
    this->aval = 0;
    this->vval = 0;
    // Default values (will be overwritten during init)
    vMult.setValue(1.0);
    aMult.setValue(1.0);

    IndiVectorMemberStorage::remember(&vBias, EepromStored::Addr(addr, 1));
    IndiVectorMemberStorage::remember(&vMult, EepromStored::Addr(addr, 2));
    IndiVectorMemberStorage::remember(&aBias, EepromStored::Addr(addr, 3));
    IndiVectorMemberStorage::remember(&aMult, EepromStored::Addr(addr, 4));

    vBiasVec.onRequested(VectorCallback(&WattMeter::updateValues, this));
    vMultVec.onRequested(VectorCallback(&WattMeter::updateValues, this));
    aBiasVec.onRequested(VectorCallback(&WattMeter::updateValues, this));
    aMultVec.onRequested(VectorCallback(&WattMeter::updateValues, this));
}

void WattMeter::tick()
{
	// 50hz = 20ms
    this->nextTick = UTime::now() + MS(50);
    long l = micros();
#ifdef __AVR_ATmega2560__
    // No analog read resolution for AtMeta2560
#else
	#ifndef TEENSYDUINO
    // #error "Unsupported build"
    #endif
    analogReadResolution(12);
#endif

    int nvvval = analogRead(vPin);
    int nvaval = analogRead(aPin);
    l = micros() - l;


    aval += nvaval;
    vval += nvvval;
    cpt++;
    if (cpt == 50) {
    	DEBUG(F("Read done in "),l);

    	aval /= 50;
    	vval /= 50;
    	lastAVal = aval;
    	lastVVal = vval;
   	    aval = 0;
   	    vval = 0;
   	    cpt = 0;
   	    updateValues();
    }
}

void WattMeter::updateValues()
{
	double vf = lastVVal == -1 ? -1 : (lastVVal - vBias.getDoubleValue()) * vMult.getDoubleValue();
	double af = lastAVal == -1 ? -1 : (lastAVal - aBias.getDoubleValue()) * aMult.getDoubleValue();
	v.setValue(vf);
	a.setValue(af);
}
