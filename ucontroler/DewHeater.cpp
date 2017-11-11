#include <Arduino.h>

#include "DewHeater.h"

#define STATUS_NEED_SCAN 0
#define STATUS_MEASURE 1
#define STATUS_IDLE 2

DewHeater::DewHeater(int pin, int suffix)
    :Scheduled(),
    group(F("DEW_HEATER")),
    temperatureVec(&group, F("DEW_HEATER_TEMP"), F("Temperature du tube")),
    temperature(&temperatureVec, F("DEW_HEATER_TEMP_VALUE"), F("Temperature du tube (°C)"),-273.15, 100),
    oneWire(pin)
{
    this->nextTick = UTime::now();
    this->priority = 2;
    this->status = STATUS_NEED_SCAN;
}

void DewHeater::failed()
{
    this->status = STATUS_NEED_SCAN;
    this->nextTick = UTime::now() + MS(1000);
}

void DewHeater::scan()
{
    Serial.println("scan\n");
    
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

    Serial.println("scan ok\n");
    // FIXME: update variables...
    this->status = STATUS_IDLE;
    this->nextTick = UTime::now() + MS(100);
}

void DewHeater::startMeasure()
{
    long t1 = micros();
    Serial.println("startMeasure\n");
    oneWire.reset();
    oneWire.skip();
    oneWire.write(0x44, 1);
    long t2 = micros();
    this->status = STATUS_MEASURE;
    this->nextTick = UTime::now() + MS(800);
    Serial.print("startmeasure done in ");
    Serial.println(t2 - t1);
}

void DewHeater::endMeasure()
{
    Serial.println("endMeasure\n");
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
    Serial.print("measure done in ");
    Serial.println(t2 - t1);

    if (OneWire::crc8(data, 8) != data[8]) {
        Serial.println("Invalid data");
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