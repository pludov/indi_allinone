/*
 * Config.cpp
 *
 *  Created on: 28 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include <EEPROM.h>                           // EEPROM Library
#include "Config.h"
#include "CommonUtils.h"

#define DEBUG_FINE(...)

#define STORAGE_SIGNATURE 0x3F8523F1

// Emplacement de la signature
#define STORAGE_SIG_OFFSET 0
// Tableau des offset des storages
#define STORAGE_PTR_OFFSET 4
// Position du slot de storage 0
#define STORAGE_BASE_OFFSET 12



Config config;

//------------------------------------------------------------------
// Save position in EEPROM - split into 2 byte values. Also sets position valid
//------------------------------------------------------------------
bool write(uint8_t * ptr, int pos, int length) {

	for(int i = 0; i < length; ++i) {
		if (EEPROM.read(pos + i) == ptr[i]) {
			continue;
		}
		EEPROM.write(pos + i, ptr[i]);
		if (EEPROM.read(pos + i) != ptr[i]) {
			return false;
		}
	}
	return true;
}

void read(uint8_t * ptr, int offset, int length)
{
	for(int i = 0; i < length; ++i)
	{
		ptr[i] = EEPROM.read(offset + i);
	}
}

int getCurrentOffset(int storage)
{
	return EEPROM.read(STORAGE_PTR_OFFSET + storage);
}

void setCurrentOffset(int storage, int offset)
{
	DEBUG_FINE(F("Set conf offset"), storage, F(" to "), offset);
	EEPROM.write(STORAGE_PTR_OFFSET + storage, offset);
}

void relocate(int storage)
{
	DEBUG_FINE(F("relocate "), storage);

	int newPos = 0;
	for(int i = 0; i < STORAGE_COUNT; ++i)
	{
		int o = getCurrentOffset(i);
		if (o > newPos) {
			o = newPos;
		}
	}
	newPos++;
	setCurrentOffset(storage, newPos);
}

Config::Config() {

}

Config::~Config() {

}

bool readStorage(int storage, uint8_t * where)
{
	int offset = getCurrentOffset(storage);
	if (offset == -1) {
		return false;
	}
	read(where, STORAGE_BASE_OFFSET + STORAGE_SIZE * offset, STORAGE_SIZE);
	return true;

}

void writeStorage(int storage, uint8_t * where)
{
	while(true){
		int offset = getCurrentOffset(storage);
		if (write(where, STORAGE_BASE_OFFSET + STORAGE_SIZE * offset, STORAGE_SIZE)) {
			return;
		}
		relocate(storage);
	}
}



void Config::init()
{
	if (initialised) {
		return;
	}

	DEBUG(F("Check conf"));
	uint32_t signature;
	read((uint8_t*)&signature, STORAGE_SIG_OFFSET, sizeof(signature));
	DEBUG_FINE(F("Readed signature"), signature);

	if (signature != STORAGE_SIGNATURE) {

		DEBUG(F("Init default conf"));

		for(int i = 0; i < STORAGE_COUNT; ++i) {
			setCurrentOffset(i, i);
		}
		{
			PositionStorage positionStorage;
			positionStorage.position = 1000;
			writeStorage(ID_STORAGE_POSITION, (uint8_t*)&positionStorage);
		}
		{
			RangeStorage rangeStorage;
			rangeStorage.maxPosition = 10000;
			writeStorage(ID_STORAGE_RANGE, (uint8_t*)&rangeStorage);
		}
		{
			TemperatureStorage temperatureStorage;
			temperatureStorage.extTempDelta = 0;
			temperatureStorage.intTempDelta = 0;
			temperatureStorage.humBias = 0;
			temperatureStorage.humFactor = 0;
			writeStorage(ID_STORAGE_TEMPERATURE, (uint8_t*)&temperatureStorage);
		}

		{
			VoltmeterStorage voltmeterStorage;
			voltmeterStorage.voltmeter_mult = 2508;
			voltmeterStorage.minVol = 11.0;
			voltmeterStorage.targetDewPoint = 15; // + 4.5
			voltmeterStorage.pwmAggressiveness = 32;
			writeStorage(ID_STORAGE_VOLTMETER, (uint8_t*)&voltmeterStorage);
		}
		{
			PositionStorage positionFilterWheel;
			positionFilterWheel.position = ((uint32_t)-1L);
			writeStorage(ID_STORAGE_FILTERWHEEL_POSITION, (uint8_t*)&positionFilterWheel);

		}
		signature = STORAGE_SIGNATURE;
		write((uint8_t*)&signature, STORAGE_SIG_OFFSET, sizeof(signature));
	} else {
		DEBUG_FINE(F("conf sig ok"));
	}

	for(uint8_t i = 0; i < STORAGE_COUNT; ++i) {
		readStorage(i, getRawStorageData(i));
	}

	DEBUG(F("config:"));
	DEBUG(F(" pos:"), storedPosition().position);
	DEBUG(F(" range:") ,storedRange().maxPosition);
	DEBUG(F(" temp.extTD:"), storedTemperature().extTempDelta);
	DEBUG(F(" temp.humBias:"), storedTemperature().humBias);
	DEBUG(F(" temp.humFactor:"), storedTemperature().humFactor);
	DEBUG(F(" temp.intTD:"), storedTemperature().intTempDelta);
	DEBUG(F(" volt.minVol:"), storedVoltmeter().minVol);
	DEBUG(F(" volt.vmul:"), storedVoltmeter().voltmeter_mult);
	DEBUG(F(" volt.targetTemp:"), storedVoltmeter().targetDewPoint);
	DEBUG(F(" volt.pwmAgg:"), storedVoltmeter().pwmAggressiveness);
	DEBUG(F(" filterwheel.pos:"),storedFilterWheelPosition().position);
	initialised = true;
}

void Config::commitStorage(uint8_t pos)
{
	writeStorage(pos, (uint8_t*)&data[pos]);
}

uint8_t* Config::getRawStorageData(uint8_t pos)
{
	return (uint8_t*)&data[pos];
}

