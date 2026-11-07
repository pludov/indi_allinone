/*
 * meteotemp.cpp
 *
 *  Created on: 4 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "CommonUtils.h"
#include "MeteoTemp.h"
#include "Status.h"


// Interval between read attempts (usec)
#define READ_INTERVAL 5000000


// how many timing transitions we need to keep track of. 2 * number bits + extra
#define MAXTIMINGS 85



MeteoTemp::MeteoTemp() : Scheduled(F("MeteoTemp")),
	group(Symbol(F("METEO"))),
	statusVec(group, Symbol(F("Meteo")), F("Measure")),
	temp(&statusVec, F("METEO_TEMP"), F("Temp"), -100,100,1),
	hum(&statusVec, F("METEO_HUM"), F("Hum (%)"), 0, 100, 1),
	dewPoint(&statusVec, F("METEO_DEW_POINT"), F("Dew Point"), -100, 100, 1)
{
	dewPointValue = NAN;
	hasData = false;

	setInvalid();
}

void MeteoTemp::setInvalid()
{
	hasData= false;
	dewPointValue = NAN;
	temp.setValue(-1000);
	hum.setValue(-1);
	dewPoint.setValue(-1000);
}

// calculates dew point
// input:   humidity [%RH], temperature in C
// output:  dew point in C
// use this where RH is < 50%
static float calc_dewpoint(float t, float h) {
  float logEx, dew_point;
  logEx = 0.66077 + 7.5 * t / (237.3 + t) + (log10(h) - 2);
  dew_point = (logEx - 0.66077) * 237.3 / (0.66077 + 7.5 - logEx);
  return dew_point;
}

void MeteoTemp::setValid()
{
	hasData= true;
	dewPointValue = calc_dewpoint(tempValue, humValue);
	DEBUG(F("DewPoint="),dewPointValue, "C");

	temp.setValue(tempValue);
	hum.setValue(humValue);
	dewPoint.setValue(dewPointValue);
}


MeteoTemp::~MeteoTemp() {
	// TODO Auto-generated destructor stub
}

