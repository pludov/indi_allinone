/*
 * SerialIO.cpp
 *
 *  Created on: 21 mars 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "SerialIO.h"
#include "debug.h"

#define InputMaxLength	50

#define PACKET_PREFIX ((uint8_t)171)
#define PACKET_SUFFIX ((uint8_t)187)

SerialIO serialIO;

SerialIO::SerialIO() {
	inputString.reserve(InputMaxLength + 4);
	inputString = "";

	stringComplete = false;
}

SerialIO::~SerialIO() {
}

void SerialIO::sendPacket(char head, uint8_t * buffer, int length)
{
	Serial.write(PACKET_PREFIX);
	Serial.write((uint8_t)head);
	Serial.write(buffer, length);
	Serial.write(PACKET_SUFFIX);
}


void SerialIO::sendPacket(uint8_t * buffer, int length)
{
	Serial.write(PACKET_PREFIX);
	Serial.write(buffer, length);
	Serial.write(PACKET_SUFFIX);
}


void SerialIO::sendPacket(const String & str)
{
	Serial.write(PACKET_PREFIX);
	Serial.print(str);
	Serial.write(PACKET_SUFFIX);
}


void SerialIO::serialEvent()
{
	if (stringComplete) return;

	int available = Serial.available();
	while(!stringComplete && available > 0) {
		int inChar = Serial.read();
		// add it to the inputString:
		available--;

		if (inputString.length() == 0){
			// Au début, il faut forcement un <<
			if (inChar == PACKET_PREFIX) {
				inputString += (char)inChar;
			} else {
				// garbage... on ignore
#ifdef DEBUG
				Serial.println("ignoring garbage");
#endif
			}
		} else {
			// On est dans un packet
			if (inChar == PACKET_PREFIX) {
#ifdef DEBUG
				Serial.println("ignoring garbage (reset)");
#endif
				// Si on reçoit un nouveau prefix, on repart de zero
				inputString = "";
			}

			inputString += (char)inChar;
			if (inChar == PACKET_SUFFIX) {
				stringComplete = true;
			} else if (inputString.length() >= InputMaxLength) {
#ifdef DEBUG
				Serial.println("overflow");
#endif
				// On met tout à la poubelle
				inputString = "";
			}
		}
	}
}

String SerialIO::getReadyInput()
{
	if (!stringComplete) return "";
	stringComplete = false;
	String result = inputString;
	result = result.substring(1, result.length() - 1);
	inputString = "";
	return result;
}

bool SerialIO::hasReadyInput()
{
	return stringComplete;
}
//------------------------------------------------------------------

//------------------------------------------------------------------
// SerialEvent occurs whenever new data comes in the serial RX.
//------------------------------------------------------------------
void serialEvent() {
	serialIO.serialEvent();
}
//------------------------------------------------------------------
