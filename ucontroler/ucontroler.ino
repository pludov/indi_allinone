// Indi All-in-one Arduino Driver
// @2019 Ludovic Pollet
//
// Release history:
// 
//------------------------------------------------------------------

#include <Arduino.h>


// Include necessary libraries
#include <OneWire.h>                          // DS18B20 temp sensor
#include <DallasTemperature.h>                // DS18B20 temp sensor
#include <EEPROM.h>                           // EEPROM Library

#include "CommonUtils.h"
#include "EepromStored.h"
#include "IndiProtocol.h"
#include "ScheduledIndiProtocol.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"
#include "XmlWriteBuffer.h"
#include "BinSerialReadBuffer.h"

#include "utime.h"
#include "BaseDriver.h"
#include "Focuser.h"
#include "FilterWheel.h"
#include "Scheduler.h"
#include "pwmresistor.h"
#include "Status.h"
#include "MeteoTemp.h"
#include "DewHeater.h"
#include "WattMeter.h"


// --------------------------------------------------------------
// Declaration of hardware interfaces
// --------------------------------------------------------------
void declareHardware(BaseDriver * baseDriver) {

	// DHT22 sensor on pin8
	MeteoTemp * meteoTemp = new MeteoTemp(8, DHT22);

	// DewHeater with sensor on pin 11 & pwm on pin 9
	DewHeater * dw = new DewHeater(meteoTemp, 11, 9, 1);
	// another dewHeater with sensor on pin 12 & pwm on pin 10
	DewHeater * dw2 = new DewHeater(meteoTemp, 12, 10, 2);

	// Measure voltage on pin 5 and current on pin 4
	new WattMeter(5, 4, 0, EepromStored::Addr(1));

	// Focuser motor - 4 pins for motor control
	static const uint8_t focuserMotorPins[4] = { A6, A7, A8, A9 };
	new Focuser(baseDriver, EepromStored::Addr(4), focuserMotorPins, 0);

	// Focuser motor - 4 pins for motor control + 1 for hall sensor
	static const uint8_t filterWheelPins[5] = { 13, 14, 15, 16, 17};
	new FilterWheel(baseDriver, EepromStored::Addr(3), filterWheelPins, 0);

	// Report uptime
	new Status();
}

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------
void setup() {
	// initialize serial for ASCOM
	Serial.begin(115200);
	Serial1.begin(115200);
	DEBUG(F("Init!"));

	delay(500);

	BaseDriver * baseDriver = new BaseDriver();
	declareHardware(baseDriver);

	// Read eeprom
	EepromStored::init();

	// Let's talk packed indi over a serial link
#ifdef __AVR_ATmega2560__
	ScheduledIndiProtocol * serialWriter = new ScheduledIndiProtocol(&Serial);
#else
	ScheduledIndiProtocol * serialWriter = new ScheduledIndiProtocol(&Serial1);
#endif
	DEBUG(F("Welcome!"));
}

//------------------------------------------------------------------
// Main Loop
//------------------------------------------------------------------
void loop() {
	Scheduler::instance().loop();
}

