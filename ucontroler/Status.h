/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef STATUS_H_
#define STATUS_H_

#include "Scheduled.h"

/*
 * Le protocol serie est :
 * Paquet de statut:
 *   [Pxxxxx] xxxxx = position moteur
 *   [My]		y = 0 off / 1 on
 *   [Txxxx]   xxxx = température scope en hexa de -327.68 à 327.67
 *   [Exxxxyy]   xxxx = température exterieure en hexa de -100.00 à 555.35
 *                 yyy = humidité (0 à 100 en ramené sur 0 - 16383)
 *   [Vxxx]   voltage en hexa, x 100 (1100 = 11V)
 *   [Hxx]	  heater % (hexa)
 *
 *
 * => chaque paquet va faire: ~20 caractères. le buffer serie peut faire 64 maxi
 * Paquet de query:
 *  P.*#
 * Paquet de reponse:
 *  P.*#
 * Problème : on ne sait pas trouver la fin d'un paquet de réponse
 *
 */
struct Payload
{
	// Position hexa : de 0 à 262k
	char motor[5];
	// moving ou pas (0 ou 1)
	char motorState[1];
	// Hexa: de -255 à +255
	char scopeTemp[4];
	// Hexa: de 0 (-50) à +1024 (50)
	char extTemp[4];
	// Hexa: de 0 à 4095 (100%)
	char extHum[3];
	// Hexa
	char battery[3];
	// Hexa
	char heater[2];
	// Position de la roue à filtre, de 0 à 262k
	char filterwheel[5];
	// See FilterWheelMotor.h:
	//  K: calibration failed
	//  C: calibration running
	//  M: moving
	//  I: idle
	char filterwheelState;
} ;


class Status : public Scheduled{
	void sendStatus();
public:
	Status();
	virtual ~Status();

	void needUpdate();

	virtual void tick();

	Payload getStatusPayload();
};

extern Status status;

void writeHex(char * buff, int length, uint32_t value);
uint32_t readHex(String & val);

#endif /* STATUS_H_ */
