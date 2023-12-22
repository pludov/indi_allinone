/*
 * meteoTempBME.h
 *
 */

#ifndef METEOTEMP_BME_H_
#define METEOTEMP_BME_H_

#include "Wire.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "Scheduled.h"
#include "TaskSequence.h"

#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"
#include "MeteoTemp.h"

struct MeasureSequence;

class MeteoTempBME : public MeteoTemp {
    friend class TaskSequenceScheduler<MeteoTempBME>;
private:
	IndiFloatVectorMember pressure;
	uint8_t addr;
	TwoWire * wire;
	Adafruit_BME280 bme; // I2C

	float pressureValue;

	void scheduleReset(bool immediate);
	void scheduleNextStep(uint8_t stepid, int msWait);
	bool init();
	bool readTemperature();
	bool readHumidity();
	bool readPressure();
	void publish();

	virtual void setInvalid();
	virtual void setValid();

	void onMeasureFailure();
	TaskSequenceScheduler<MeteoTempBME> measureScheduler;
	static TaskSequence<MeteoTempBME>* measureSequence();

public:
	MeteoTempBME(TwoWire * wire, uint8_t pinSda, uint8_t pinScl, int addr = -1);
	virtual ~MeteoTempBME();

	virtual void tick();
};


#endif /* METEOTEMP_BME_H_ */
