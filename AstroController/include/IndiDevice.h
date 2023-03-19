/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIDEVICE_H_
#define INDIDEVICE_H_

#include "WriteBuffer.h"

class IndiVector;
class IndiProtocol;
class IndiDeviceMutator;

class IndiDevice {
	friend class IndiProtocol;
public:
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
