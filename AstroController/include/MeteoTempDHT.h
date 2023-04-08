/*
 * meteoTempDHT.h
 *
 */

#ifndef METEOTEMP_DHT_H_
#define METEOTEMP_DHT_H_

#include "Scheduled.h"

#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"
#include "MeteoTemp.h"

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21


class MeteoTempDHT : public MeteoTemp {

private:
	uint8_t data[6];
	uint8_t _pin, _type, _count;
	uint8_t nextStep;

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
	MeteoTempDHT(uint8_t pin, uint8_t type);
	virtual ~MeteoTempDHT();

	virtual void tick();
};


#endif /* METEOTEMP_DHT_H_ */
