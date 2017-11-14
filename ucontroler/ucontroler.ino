//------------------------------------------------------------------
// Ascom-Arduino Focusser
// Forker from works of Dave Wells,
// With thanks for code snippets & inspiration:
//  o  Gina (Stargazers Lounge) for the stepper control basics
//  o  russellhq  (Stargazers Lounge) for the 1-wire code and info.
//------------------------------------------------------------------

//------ Change Log for AAF2 -------------------------------------------
//  Version    Date        Change
//  0.0.2      26/05/2014  Initial - copied from Windows version
//  2.0.0      22/06/2014  Renumbered to match Ascom Driver Numbering
//  2.0.1      09/08/2014  Initial position set to 1000
//  2.0.2      16/08/2014  Halt function implemented with H# command
//                         New I# command to set an initial position
//
//  version numbering brought into line with driver numbering
//  2.2.0      03/10/2014  Implemented Temperature Sensing C# command
//  2.3.0      07/10/2014  Now stores position in EEPROM
//                         Changed some int variables to unsigned int
//                         Fixed minor bug in motor hi/lo speed detection
//                         Temperature now in 100ths of degree for internationlisation
//                         Block temperature requests during focuser movement
//------------------------------------------------------------------

#include <Arduino.h>


// Include necessary libraries
#include <OneWire.h>                          // DS18B20 temp sensor
#include <DallasTemperature.h>                // DS18B20 temp sensor
#include <EEPROM.h>                           // EEPROM Library

#include "IndiProtocol.h"
#include "IndiVectorGroup.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"

#include "utime.h"
#include "MainLogic.h"
#include "Motor.h"
#include "FilterWheelMotor.h"
#include "Scheduler.h"
#include "pwmresistor.h"
#include "scopetemp.h"
#include "Status.h"
#include "MeteoTemp.h"
#include "voltmeter.h"
#include "Config.h"
#include "debug.h"
#include "SerialIO.h"
#include "DewHeater.h"


#define ONE_WIRE_BUS 3                        // DS18B20 DATA wire connected to Digital Pin 6
// EEPROM storage locations
#define EE_LOC_POS 33                          // Location of position (3 bytes)
#define EE_LOC_PSTAT 0                        // Location of Position Status (1 Byte)
// Position Statuses
#define POS_VALID 55                          // Stored position valid if this value otherwise assume invalid

const String programName = "Pludov ArdFoc";
const String programVersion = "2.3.0";

#define RESISTORPIN 9		// PIN for the heater
#define DHTPIN 2  		// what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)

const uint8_t motorPins[4] = { 5, 6, 7, 8 }; // Declare pins to drive motor control board
const uint8_t filterWheelPins[4] = {A3, A2, A1, A0};

// Initialise the temp sensor
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices
// DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.

int debug = 0;

/*Motor motor(motorPins);
FilterWheelMotor filterWheelMotor(filterWheelPins);
PWMResistor resistor(RESISTORPIN);
ScopeTemp scopeTemp(&oneWire);
MeteoTemp meteoTemp(DHTPIN, DHTTYPE);
Voltmeter voltmeter(7);
MainLogic mainLogic;*/
Status status;

//
////------------------------------------------------------------------
//
////------------------------------------------------------------------
//// Check if stored position in EEPROM is valid
////------------------------------------------------------------------
//boolean storedPositionValid(void) {
//	byte status = EEPROM.read(EE_LOC_PSTAT);
//	if (status == POS_VALID)
//		return true;
//	else
//		return false;
//}
//
////------------------------------------------------------------------
//// Save position in EEPROM - split into 2 byte values. Also sets position valid
////------------------------------------------------------------------
//void savePosition(unsigned long p) {
//
//	for(int i = 0; i < 3; ++i) {
//		byte b = (p >> (i * 8)) & 0xFF;
//		if (EEPROM.read(EE_LOC_POS + i) == b) {
//			continue;
//		}
//		EEPROM.write(EE_LOC_POS + i, b);
//	}
//
//	if (!storedPositionValid()) {
//		EEPROM.write(EE_LOC_PSTAT, POS_VALID);   // stored position is valid
//	}
//}
////------------------------------------------------------------------
//
////------------------------------------------------------------------
//// Restore position from EEPROM
////------------------------------------------------------------------
//unsigned long restorePosition(void) {
//	unsigned long result = 0;
//	for(int i = 0; i < 3; ++i) {
//		unsigned long b = (unsigned long)EEPROM.read(EE_LOC_POS + i);
//		result |= b << (i * 8);
//	}
//
//	return result;
//}
////------------------------------------------------------------------

//------------------------------------------------------------------

//------------------------------------------------------------------
// ASCOM Serial Commands
//------------------------------------------------------------------
void serialCommand(String command) {
	if (command.length() == 0) {
		serialIO.sendPacket("ERR");
		return;
	}
	switch (command.charAt(0)) {
	case 'X':  // Confirm Connection
		serialIO.sendPacket(command);
		break;
// 	case 'T': // Set Target Position
// 	{
// 		String targetPosS = command.substring(1);
// 		unsigned long targetPosI = targetPosS.toInt();
// 		motor.setTargetPosition(targetPosI);
// 		serialIO.sendPacket("T" + targetPosS + ":OK");
// 		break;
// 	}
// 	case 'F': // Set Filterwheel Position
// 	{
// 		if (!filterWheelMotor.lastCalibrationFailed()) {
// 			String targetPosS = command.substring(1);
// 			unsigned long targetPosI = targetPosS.toInt();
// 			filterWheelMotor.setTargetPosition(targetPosI);
// 			serialIO.sendPacket("F" + targetPosS + ":OK");
// 		} else {
// #ifdef DEBUG
// 			Serial.println(F("Move without calibration"));
// #endif
// 			serialIO.sendPacket("F:ERR");
// 		}

// 		break;
// 	}
// 	case 'Q': // Calibration
// 	{

// 		String targetPosS = command.substring(1);
// 		unsigned long targetPosI = targetPosS.toInt();

// 		filterWheelMotor.startCalibration(targetPosI);
// 		serialIO.sendPacket("Q" + targetPosS + ":OK");

// 		break;

// 	}
// 	case 'C': // Get Temperature
// 	{
// 		double t;

// 		// if moving block temperature requests as they interfere with movement. Just return last reading.
// 		t = scopeTemp.lastValue;

// 		serialIO.sendPacket(String("C")  + (int)(t * 100) + ":OK#");
// 		break;
// 	}
// 	case 'I': // Set Initial Position. Sets Position without any movement
// 	{
// 		int hashpos = command.indexOf('#');    // position of hash in string
// 		String initPosS = command.substring(1, hashpos);
// 		unsigned long initPosI = initPosS.toInt();
// 		motor.loadPosition(initPosI);
// 		config.storedPosition().position = initPosI;
// 		config.commitStoredPosition();

// 		serialIO.sendPacket("I" + initPosS + ":OK");
// 		break;
// 	}
// 	case 'P': // Get Current Position
// 	{
// 		String currentPositionS = String(motor.getCurrentPosition());
// 		serialIO.sendPacket("P" + currentPositionS + ":OK");
// 		break;
// 	}
// 	case 'f': // Get current filterwheel position and status
// 	{
// 		String currentPositionS = String(filterWheelMotor.getCurrentPosition()) + String(filterWheelMotor.getProtocolStatus());
// 		serialIO.sendPacket("f" + currentPositionS + ":OK");
// 		break;
// 	}
// 	case 'H': // Halt
// 	{
// 		unsigned long where = motor.getCurrentPosition();
// 		motor.setTargetPosition(where);
// 		String currentPositionS = String(where);
// 		serialIO.sendPacket("H" + currentPositionS + ":OK");
// 		break;
// 	}
// 	case 'M': // Is motor moving
// 	{
// 		if (motor.isActive()) {
// 			serialIO.sendPacket("M1:OK");
// 		} else {
// 			serialIO.sendPacket("M0:OK");
// 		}
// 		break;
// 	}
// 	case 'V': // Get Version and abilities
// 	{
// 		String tempInstalled = (scopeTemp.isAvailable() ? "(Temp. Sensor)" : "");

// 		serialIO.sendPacket("V" + programName + "-" +  programVersion + tempInstalled);
// 		break;
// 	}
// 	case 'S':
// 	{
// 		Payload statusPayload = status.getStatusPayload();
// 		serialIO.sendPacket('S', (uint8_t*)&statusPayload, sizeof(statusPayload));
// 		break;
// 	}
// 	case 'Z':
// 	{
// 		// Acces à la conf
// 		int eq;
// 		if ((eq = command.indexOf('=')) == -1) {
// 			String initPosS = command.substring(1);
// 			unsigned long initPosI = initPosS.toInt();
// 			char reply[9];

// 			writeHex(reply, 8, *(uint32_t*)config.getRawStorageData(initPosI));
// 			reply[8] = 0;

// 			serialIO.sendPacket('Z', (uint8_t*)&reply, 8);
// 		} else {
// 			String initPosS = command.substring(1, eq);
// 			String val = command.substring(eq + 1);
// 			unsigned long initPosI = initPosS.toInt();

// 			uint32_t v = readHex(val);
// 			*((uint32_t*)config.getRawStorageData(initPosI)) = v;
// 			config.commitStorage(initPosI);
// 			serialIO.sendPacket("Z:OK");
// 		}
// 		break;
// 	}
	default: {
		serialIO.sendPacket("ERR");
		break;
	}
	}
}
//------------------------------------------------------------------

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------
void setup() {
	// initialize serial for ASCOM
	Serial.begin(115200);
	Serial1.begin(115200);

	delay(500);
#ifdef DEBUG
	Serial.println(F("Welcome on board"));
	Serial.println(F("Commands are"));
	Serial.println(F("X - confirm"));
	Serial.println(F("T? - set focuser pos"));
	Serial.println(F("F? - set fw pos"));
	Serial.println(F("Q? - calibrate fw to pos"));
	Serial.println(F("C - get temp"));
	Serial.println(F("I? - set focuser pos"));
	Serial.println(F("P - get focuser pos"));
	Serial.println(F("f - get fw pos & status"));
	Serial.println(F("H - halt focuser"));
	Serial.println(F("M - is focuser moving"));
	Serial.println(F("V - version"));
	Serial.println(F("S - query status"));
	Serial.println(F("Z? - get conf item"));
	Serial.println(F("Z?=HEX - set conf item"));



	delay(500);
#endif
	// reserve 200 bytes for the ASCOM driver inputString:


	//EEPROM.write(EE_LOC_PSTAT, 0); // FOR TESTING - invalidate stored position

	// Use position from EEPROM if it is valid, otherwise use default
	//config.init();
#ifdef DEBUG
	Serial.println(F("done with init conf"));
#endif

	//motor.loadConfigPosition();
	//filterWheelMotor.loadConfigPosition();
#ifdef DEBUG
	Serial.println(F("now reset search"));
#endif
	// OneWire Libary setup
	oneWire.reset_search();                    // Reset search

#ifdef DEBUG
	Serial.println(F("Init done"));
#endif


	DewHeater * dw = new DewHeater(11, 1);

	IndiProtocol * serialWriter = new IndiProtocol(&Serial);
	IndiProtocol * serialWriter2 = new IndiProtocol(&Serial1);


	char buffer[4096];
	WriteBuffer into(buffer, 4096);
// 	IndiDevice::instance().dump(into);


	if (into.finish()) {
		Serial.print("root:");
		Serial.println(buffer);
	} else {
		Serial.println("Not enough space");
	}


}
//------------------------------------------------------------------

UTime previousTime = UTime::now();
//------------------------------------------------------------------
// Main Loop
//------------------------------------------------------------------
void loop() {
	// process the command string when a hash arrives:
	if (serialIO.hasReadyInput()) {

		serialCommand(serialIO.getReadyInput());
	}

	Scheduler::instance().loop();
}

