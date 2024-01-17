#include <Arduino.h>
#include "CommonUtils.h"
#include "MeteoTemp.h"
#include "MeteoTempBME.h"
#include "EepromStored.h"
#include "Status.h"


class BMEMemory: public EepromStored {
public:
    float compensationValue;

    virtual void decodeEepromValue(void * buffer, uint8_t sze){
        if (sze != sizeof(float)) {
            DEBUG(F("BMEMemory EEPROM invalid size"), sze);
            return;
        }
        memcpy(&compensationValue, (void*)buffer, sze);
    }

    virtual void encodeEepromValue(void * buffer, uint8_t sze) {
        if (sze != sizeof(float)) return;
        memcpy(buffer, (void*)&compensationValue, sze);
    }

    virtual int getEepromSize() const {
        return sizeof(float);
    }
public:
    BMEMemory(uint32_t addr): EepromStored(addr) {
        compensationValue = 0.0;
    }

    void setComposenationValue(float v) {
        if (this->compensationValue != v) {
            this->compensationValue = v;
            write();
        }
    }
    float getComposenationValue() const {
        return this->compensationValue;
    }
};

// Interval between read attempts (msec)
#define READ_INTERVAL 5000

TaskSequence<MeteoTempBME>* MeteoTempBME::measureSequence() {
	static TaskSequence<MeteoTempBME> seq[] = {
        {&MeteoTempBME::init, 5, nullptr, 10},
        {nullptr, 0,                     &MeteoTempBME::copySettings, 10, SEQUENCE_LOOP_HERE},
		{&MeteoTempBME::readTemperature, 2, nullptr, 10},
		{&MeteoTempBME::readHumidity, 2, nullptr, 10},
		{&MeteoTempBME::readPressure, 2, &MeteoTempBME::publish, READ_INTERVAL},
		{nullptr, 0, nullptr, 0}
	};

	return seq;
};

MeteoTempBME::MeteoTempBME(uint32_t addr, TwoWire * wire, uint8_t pinSda, uint8_t pinScl, int bmeAddr) : MeteoTemp(),
    pressure(&statusVec, F("METEO_PRESSURE"), F("Pressure"), 900, 1100, 0.1),
    sensorSettingsVec(group, F("METEO_SETTINGS"), F("Sensor Settings"), VECTOR_WRITABLE|VECTOR_READABLE),
    compensationValue(&sensorSettingsVec, F("TEMP_COMP"), F("Temp compensation"),-100, 100, 0.1),
    measureScheduler(this, measureSequence(), &MeteoTempBME::onMeasureFailure),
    eepromReadyListener(EepromCallback(&MeteoTempBME::loadInitialSettings, this))
{
    memory = new BMEMemory(addr);
    wire->setSDA(pinSda);
	wire->setSCL(pinScl);
    setInvalid();

    this->addr = bmeAddr == -1 ? BME280_ADDRESS : bmeAddr;
    this->wire = wire;
    compensationValue.setValue(0.0);
    sensorSettingsVec.onRequested(VectorCallback(&MeteoTempBME::updateMemory, this));

	scheduleReset(true);
}

void MeteoTempBME::updateMemory() {
    memory->setComposenationValue(compensationValue.getValue());
}

void MeteoTempBME::loadInitialSettings() {
    compensationValue.setValue(memory->getComposenationValue());
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

void MeteoTempBME::copySettings()
{
    // This is done in a sync function to avoid race access to the float value
    this->compensationValueForCurrentMeasure = compensationValue.getValue();
}

bool MeteoTempBME::readTemperature()
{
    bme.setTemperatureCompensation(this->compensationValueForCurrentMeasure);
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
