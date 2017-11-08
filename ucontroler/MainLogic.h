/*
 * MainLogic.h
 *
 *  Created on: 7 fï¿½vr. 2015
 *      Author: utilisateur
 */

#ifndef MAINLOGIC_H_
#define MAINLOGIC_H_

#include "Scheduled.h"

class MainLogic : public Scheduled{
	void updatePwm();
public:
	MainLogic();
	virtual ~MainLogic();

	void temperatureUpdated();

	void tick();
};

extern MainLogic mainLogic;

#endif /* MAINLOGIC_H_ */
