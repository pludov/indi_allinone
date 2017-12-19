/*
 * meteoTemp.h
 *
 *  Created on: 4 févr. 2015
 *      Author: utilisateur
 */

#ifndef METEOTEMP_H_
#define METEOTEMP_H_

#include "Scheduled.h"

#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

class MeteoTemp : public Scheduled {
    Symbol group;
	IndiNumberVector statusVec;
	IndiFloatVectorMember temp;
	IndiFloatVectorMember hum;
	IndiFloatVectorMember dewPoint;

	void setInvalid();
	void setValid();
private:
	uint8_t data[6];
	uint8_t _pin, _type, _count;
	uint8_t nextStep;
	bool hasData;
	float tempValue;
	float humValue;
	float dewPointValue;

	// pull the pin high then need to wait 250 milliseconds
	void prepareStep1(bool first);
	void doStep1();
	// pull it low for ~20 milliseconds
	void prepareStep2();
	void doStep2();
	// read the result into data
	void prepareStep3();
	void doStep3();

	float extractTemperature();
	float extractHumidity();

public:
	MeteoTemp(uint8_t pin, uint8_t type);
	virtual ~MeteoTemp();

	virtual void tick();

};

extern MeteoTemp meteoTemp;

#endif /* METEOTEMP_H_ */
