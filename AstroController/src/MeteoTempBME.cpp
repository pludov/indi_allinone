#include <Arduino.h>
#include "CommonUtils.h"
#include "MeteoTemp.h"
#include "MeteoTempBME.h"
#include "Status.h"


// Interval between read attempts (msec)
#define READ_INTERVAL 5000

TaskSequence<MeteoTempBME>* MeteoTempBME::measureSequence() {
	static TaskSequence<MeteoTempBME> seq[] = {
        {&MeteoTempBME::init, 5, nullptr, 10},
		{&MeteoTempBME::readTemperature, 2, nullptr, 10, SEQUENCE_LOOP_HERE},
		{&MeteoTempBME::readHumidity, 2, nullptr, 10},
		{&MeteoTempBME::readPressure, 2, &MeteoTempBME::publish, READ_INTERVAL},
		{nullptr, 0, nullptr, 0}
	};

	return seq;
};

MeteoTempBME::MeteoTempBME(TwoWire * wire, uint8_t pinSda, uint8_t pinScl, int addr) : MeteoTemp(),
    pressure(&statusVec, F("METEO_PRESSURE"), F("Pressure"), 900, 1100, 0.1),
    measureScheduler(this, measureSequence(), &MeteoTempBME::onMeasureFailure)
{
    wire->setSDA(pinSda);
	wire->setSCL(pinScl);
    setInvalid();

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
    nextTick = UTime::now() + MS(immediate ? 0 : READ_INTERVAL);
    tickExpectedDuration = MS(200);
    priority = 3;
}

void MeteoTempBME::onMeasureFailure()
{
    measureScheduler.stop();
    setInvalid();
    scheduleReset(false);
}

void MeteoTempBME::tick()
{
    if (measureScheduler.handleTick()) {
        return;
    }

    measureScheduler.start(MS(2000));
}

MeteoTempBME::~MeteoTempBME() {
}

bool MeteoTempBME::init()
{
    long t1 = micros();
    bool started = bme.begin(addr, wire);
    long t2 = micros();
    DEBUG(F("MeteoTempBME::begin"), started, F(" in "), t2 - t1, F("us"));
    if (!started) {
        DEBUG(F("MeteoTempBME failed to start"));
    } else {
        DEBUG(F("MeteoTempBME started successfully"));
    }
    return started;
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

void MeteoTempBME::publish() {
    setValid();
}
