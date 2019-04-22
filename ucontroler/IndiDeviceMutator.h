#ifndef INDIDEVICEMUTATOR_H_
#define INDIDEVICEMUTATOR_H_

#include "IndiDevice.h"

class IndiVector;
class IndiProtocol;

/**
 * Provides notification to the indi driver. Not used in micro controler
 */
class IndiDeviceMutator {

public:
	IndiDeviceMutator(){}

	virtual ~IndiDeviceMutator() {}

	// vec is new. it may have replaced existing vec
	virtual void announced(IndiVector * vec) = 0;
	// vec is not new.
	virtual void updated(IndiVector * vec) = 0;
};


#endif /* INDIDEVICEMUTATOR_H_ */
