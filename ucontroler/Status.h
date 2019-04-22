/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef STATUS_H_
#define STATUS_H_

#include "Scheduled.h"

#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"


/** Report uptime, to track uc reboot */
class Status : public Scheduled{
	Symbol statusGroup;
	IndiNumberVector uptime;
	IndiFloatVectorMember uptimeValue;

public:
	Status();
	virtual ~Status();

	virtual void tick();
};

#endif /* STATUS_H_ */
