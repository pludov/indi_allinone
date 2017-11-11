/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIDEVICE_H_
#define INDIDEVICE_H_

#include "Scheduled.h"
#include "WriteBuffer.h"

class IndiVector;
class IndiProtocol;

class IndiDevice {
	friend class IndiProtocol;
	IndiVector ** list;
	int variableCount;
	IndiProtocol * firstWriter;
protected:
	friend class IndiVector;
	void add(IndiVector * v);
public:
	IndiDevice(int variableCount);
	void dump(WriteBuffer & into);

	static IndiDevice & instance();
};


#endif /* INDIDEVICE_H_ */
