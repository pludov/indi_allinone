#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <string.h>

#include "CommonUtils.h"
#include "Blinker.h"

Blinker::Blinker(uint8_t pin) : Scheduled(F("blinker")) {
    this->pin = pin;
    this->cnxState = false;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    this->nextTick = UTime::now();
    this->flipFlop = 0;
	this->priority = 2;
	this->tickExpectedDuration = US(100);
}


void Blinker::tick()
{
	this->nextTick += MS(500);
    if (!this->cnxState) {
        digitalWrite(pin, flipFlop);
        flipFlop = !flipFlop;
    } else {
        digitalWrite(pin, LOW);
    }
}

void Blinker::setCnxState(bool state) {
    if (this->cnxState == state) {
        return;
    }
    this->cnxState = state;
    this->nextTick = UTime::now();
    this->flipFlop = 0;
}

Blinker::~Blinker() {
}
