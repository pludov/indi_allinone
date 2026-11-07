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
protected:
    Symbol group;
	IndiNumberVector statusVec;
	IndiFloatVectorMember temp;
	IndiFloatVectorMember hum;
	IndiFloatVectorMember dewPoint;

	// Compute dewpoint & update vectors
	virtual void setInvalid();
	// Clear dewpoint & update vectors
	virtual void setValid();

	float tempValue;
	float humValue;

private:
	bool hasData;
	float dewPointValue;

public:
	MeteoTemp();
	virtual ~MeteoTemp();

	bool isReady() const {
		return hasData;
	}

	float getDewPoint() const {
		return dewPointValue;
	}
};

#endif /* METEOTEMP_H_ */
