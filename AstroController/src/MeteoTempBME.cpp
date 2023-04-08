#include <Arduino.h>
#include "CommonUtils.h"
#include "MeteoTemp.h"
#include "MeteoTempBME.h"
#include "Status.h"


// Interval between read attempts (msec)
#define READ_INTERVAL 5000


typedef bool (MeteoTempBME::*MeasureStep)();

struct MeasureSequence {
	MeasureStep func;
	uint16_t msWait;
};

MeasureSequence* MeteoTempBME::measureSequence() {
	static MeasureSequence seq[] = {
		{&MeteoTempBME::readTemperature, 10},
		{&MeteoTempBME::readHumidity, 10},
		{&MeteoTempBME::readPressure, 10},
		{&MeteoTempBME::publish, READ_INTERVAL},
		{nullptr, 0}
	};

	return seq;
};


MeteoTempBME::MeteoTempBME(TwoWire * wire, uint8_t pinSda, uint8_t pinScl, int addr) : MeteoTemp(),
    pressure(&statusVec, F("METEO_PRESSURE"), F("Pressure"), 900, 1100, 0.1)
{
    wire->setSDA(pinSda);
	wire->setSCL(pinScl);
    setInvalid();


    started = false;

    this->addr = addr == -1 ? BME280_ADDRESS : addr;
    this->wire = wire;

	scheduleReset(true);
}

void MeteoTempBME::setInvalid() {
    MeteoTemp::setInvalid();
    pressure.setValue(-1);
}

void MeteoTempBME::setValid() {
    MeteoTemp::setValid();
    pressure.setValue(pressureValue);
}

void MeteoTempBME::scheduleReset(bool immediate)
{
    started = false;
    nextTick = UTime::now() + MS(immediate ? 0 : READ_INTERVAL);
    tickExpectedDuration = MS(200);
    priority = 3;
    nextStep = -1;
}

void MeteoTempBME::scheduleNextStep(uint8_t stepid, int msWait)
{
    auto stepDef = measureSequence()[stepid];
    if (stepDef.func == nullptr) {
        stepid = 0;
        stepDef = measureSequence()[stepid];
    }

    this->nextTick = UTime::now() + MS(msWait);
    this->nextStep = stepid;
    // Reading humidity & pressure re-read temperature so takes longer
    this->tickExpectedDuration = US(1300);
    this->priority = 2;
}

void MeteoTempBME::tick()
{
	DEBUG(F("MeteoTempBME::tick"), nextStep);
    if (!started) {
        long t1 = micros();
        started = bme.begin(addr, wire);
        long t2 = micros();
        DEBUG(F("MeteoTempBME::begin"), started, F(" in "), t2 - t1, F("us"));
        if (!started) {
            DEBUG(F("MeteoTempBME failed to start"));
            scheduleReset(false);
            return;
        }
        DEBUG(F("MeteoTempBME started successfully"));
        nextStep = 0;
        scheduleNextStep(0, 0);
        return;
    }

    auto stepId = this->nextStep;
    auto step = measureSequence()[stepId];
    auto func = step.func;
    auto msWait = step.msWait;

    long t1 = micros();
    bool success = ((*this).*func)();
    long t2 = micros();
    DEBUG(F("step "), stepId, F(" yield "), success, F(" in "), t2 - t1, F("us"));
    if (success) {
        scheduleNextStep(stepId + 1, msWait);
    } else {
        started = false;
        scheduleReset(false);
    }
}

MeteoTempBME::~MeteoTempBME() {
}


bool MeteoTempBME::readTemperature()
{
    // Never fails (or lies ...)
    auto temperature = bme.readTemperature();
    this->tempValue = temperature;
    return true;
}

bool MeteoTempBME::readHumidity()
{
    // Never fails (or lies ...)
    auto humidity = bme.readHumidity();
    this->humValue = humidity;
    return true;
}

bool MeteoTempBME::readPressure()
{
    // Never fails (or lies ...)
    auto pressure = bme.readPressure();
    this->pressureValue = pressure / 100.0;
    return true;
}

bool MeteoTempBME::publish() {
    setValid();
    return true;
}
