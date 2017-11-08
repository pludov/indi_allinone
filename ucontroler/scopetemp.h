/*
 * scopetemp.h
 *
 *  Created on: 4 févr. 2015
 *      Author: utilisateur
 */

#ifndef SCOPETEMP_H_
#define SCOPETEMP_H_

#include <OneWire.h>                          // DS18B20 temp sensor
#include <DallasTemperature.h>                // DS18B20 temp sensor

#include "Scheduled.h"

class ScopeTemp : public Scheduled {
	UTime lastRead;

	DallasTemperature dallas;
	DeviceAddress sensorAddress;
	// 0 => need to request, 1 => first read will occur, ...
	uint8_t requested;
public:
	float lastValue;


	ScopeTemp(OneWire * oneWire);
	virtual ~ScopeTemp();
	virtual void tick();

	// Is the device present
	bool isAvailable();
private:
	bool searchSensor();

};

extern ScopeTemp scopeTemp;

#endif /* SCOPETEMP_H_ */
