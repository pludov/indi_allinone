/*
 * Config.h
 *
 *  Created on: 28 févr. 2015
 *      Author: utilisateur
 */

#ifndef CONFIG_H_
#define CONFIG_H_

// Nombre de storages
#define STORAGE_COUNT 5
// Taille d'un storage en octet
#define STORAGE_SIZE 4

#define ID_STORAGE_POSITION 0
#define ID_STORAGE_RANGE 1
#define ID_STORAGE_TEMPERATURE 2
#define ID_STORAGE_VOLTMETER 3
#define ID_STORAGE_FILTERWHEEL_POSITION 4

struct PositionStorage {
	uint32_t position;
}__attribute__((packed));

struct RangeStorage {
	uint32_t maxPosition;
}__attribute__((packed));

struct TemperatureStorage {
	// Décalage de température exterieure en 1/10 de degres => -12.8/+12.7°
	int extTempDelta:8;
	// Décalage de température exterieure en 1/10 de degres => -12.8/+12.7°
	int intTempDelta:8;
	// Biais sur l'humidité en % (-64 => +63%)
	int humBias:7;
	// Facteur de proportion de 0.744 à 1.255
	int humFactor:9;
}__attribute__((packed));

struct VoltmeterStorage {
	// Multiplicateur pour le voltage (16 bits) v = (0-1023) * (voltmeter_mult) / 131072
	// 12 = 627 * (voltmeter_mult) / 131072
	// 12 * 131072 / 627 = voltmeter_mult
	// voltmeter_mult = 12 * 131072 / 627
	unsigned int voltmeter_mult:13;

	// Tension mini de fonctionnement (4.5 + x / 10)
	unsigned int minVol:7;

	// On cible de nombre de degres au dessus (3 + x / 10)
	unsigned int targetDewPoint:6;

	// Aggressivité de l'algo (réaction en unité de pwm)
	// 2 ^ (pwmAggressiveness  / 8)
	unsigned int pwmAggressiveness:6;
}__attribute__((packed));


/** Stocke la configuration */
class Config {
	bool initialised;
	uint32_t data[STORAGE_COUNT];


public:
	Config();
	~Config();

	void init();

	void commitStorage(uint8_t pos);

	inline PositionStorage & storedPosition() {
		return *(PositionStorage*)&data[ID_STORAGE_POSITION];
	}

	inline PositionStorage & storedFilterWheelPosition() {
		return *(PositionStorage*)&data[ID_STORAGE_FILTERWHEEL_POSITION];
	}


	inline void commitStoredPosition() {
		commitStorage(ID_STORAGE_POSITION);
	}

	inline RangeStorage & storedRange() {
		return *((RangeStorage*)&data[ID_STORAGE_RANGE]);
	}

	inline void commitStoredRange() {
		commitStorage(ID_STORAGE_RANGE);
	}

	inline TemperatureStorage & storedTemperature() {
		return *((TemperatureStorage*)&data[ID_STORAGE_TEMPERATURE]);
	}

	inline void commitStoredTemperature() {
		commitStorage(ID_STORAGE_TEMPERATURE);
	}

	inline VoltmeterStorage & storedVoltmeter() {
		return *((VoltmeterStorage*)&data[ID_STORAGE_VOLTMETER]);
	}

	inline void commitStoredVoltmeter() {
		commitStorage(ID_STORAGE_VOLTMETER);
	}

	// Give address of a configuration slot by id
	uint8_t* getRawStorageData(uint8_t storage);

	float getTargetDeltaTemp() {
		return 3 + storedVoltmeter().targetDewPoint / 10.0;
	}
	int getPwmStep() {
		return pow(2, storedVoltmeter().pwmAggressiveness / 8.0);
	}
};

extern Config config;

#endif /* CONFIG_H_ */
