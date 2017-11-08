/*
 * SerialIO.h
 *
 *  Created on: 21 mars 2015
 *      Author: utilisateur
 */

#ifndef SERIALIO_H_
#define SERIALIO_H_

class SerialIO {
	String inputString;      // string to hold incoming data
	boolean stringComplete;  // whether the string is complete
public:
	SerialIO();
	virtual ~SerialIO();

	void sendPacket(char head, uint8_t * buffer, int length);
	void sendPacket(uint8_t * buffer, int length);
	void sendPacket(const String & str);

	void serialEvent();

	bool hasReadyInput();

	String getReadyInput();
};

extern SerialIO serialIO;

#endif /* SERIALIO_H_ */
