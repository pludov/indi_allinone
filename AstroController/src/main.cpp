// Indi All-in-one Arduino Driver
// @2019 Ludovic Pollet
//
// Release history:
// 
//------------------------------------------------------------------

#include <Arduino.h>

#ifdef USE_TINYUSB
#include <Adafruit_TinyUSB.h>
#endif


// Include necessary libraries
#include <OneWire.h>                          // DS18B20 temp sensor
#include <EEPROM.h>                           // EEPROM Library

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

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
#include "Status.h"
#include "MeteoTemp.h"
#include "MeteoTempDHT.h"
#include "MeteoTempBME.h"
#include "DewHeater.h"
#include "WattMeter.h"
#include "Blinker.h"

#ifdef USE_TINYUSB
Adafruit_USBD_CDC communicationSerial;
#endif

// --------------------------------------------------------------
// Declaration of hardware interfaces
// --------------------------------------------------------------
void declareHardware(BaseDriver * baseDriver) {

#ifdef TEENSYDUINO
	// DHT22 sensor on pin8
	MeteoTemp * meteoTemp = new MeteoTempDHT(8, DHT22);

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

#else
	// BME Sensor

#if 1
	MeteoTemp * meteoTemp = new MeteoTempBME(&Wire, 16, 17);
	new DewHeater(meteoTemp, 21, 18, 1);
	new DewHeater(meteoTemp, 13, 19, 2);
	new DewHeater(meteoTemp, 26, 20, 3);
#else

	MeteoTemp * meteoTemp = new MeteoTempBME(&Wire, 16, 17);
	// DewHeater : sensor, resistor
//  new DewHeater(meteoTemp, 21, 18, 1);
//	new DewHeater(meteoTemp, 22, 19, 2);
//	// Limited by single oneWire available. Cannot read temperature concurrently
//	new DewHeater(meteoTemp, 26, 20, 3);
#endif

	// Focuser motor - 4 pins for motor control + 1 for hall sensor
	static const uint8_t filterWheelPins[5] = { 6, 7, 8, 9, 15};
	new FilterWheel(baseDriver, EepromStored::Addr(3), filterWheelPins, 0);

	// Focuser motor - 4 pins for motor control
	static const uint8_t focuserMotorPins[4] = { 2, 3, 4, 5 };
	new Focuser(baseDriver, EepromStored::Addr(4), focuserMotorPins, 0);

	new Blinker(PIN_LED);

#endif


	// Report uptime
	new Status();
}


#ifdef ARDUINO_ARCH_RP2040
// Take ~ 64Ko for storage
FlashStore flashStore(16);
#endif

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------
void setup() {
	pinMode(PIN_LED, OUTPUT); //
	digitalWrite(PIN_LED, LOW);
	delay(250);
	// initialize serial for ASCOM
	Serial.begin(115200);
	digitalWrite(PIN_LED, HIGH);

#ifdef USE_TINYUSB
	communicationSerial.begin(115200);
#else
	// Larger fifo size.
	// Debug should never block the main thread
	Serial1.setFIFOSize(256);
	Serial1.begin(230400);
#endif

	DEBUG(F("Init!"));

	delay(500);

	BaseDriver * baseDriver = new BaseDriver();
	declareHardware(baseDriver);

#ifdef ARDUINO_ARCH_RP2040
	flashStore.initialize();
#else
	// Read eeprom
	EepromStored::init();
#endif

	// Let's talk packed indi over a serial link
#ifdef __AVR_ATmega2560__
	ScheduledIndiProtocol * serialWriter = new ScheduledIndiProtocol(&Serial);
#elif USE_TINYUSB
	ScheduledIndiProtocol * serialWriter = new ScheduledIndiProtocol(&communicationSerial);
#elif TEENSYDUINO
	ScheduledIndiProtocol * serialWriter = new ScheduledIndiProtocol(&Serial1);
#else
	ScheduledIndiProtocol * serialWriter = new ScheduledIndiProtocol(&Serial);
#endif
	DEBUG(F("Welcome!"));
}

//------------------------------------------------------------------
// Main Loop
//------------------------------------------------------------------
void loop() {
	Scheduler::instance().loop();
}

