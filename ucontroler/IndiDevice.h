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

class Vector;
class DeviceWriter;

class Device {
	friend class DeviceWriter;
	Vector ** list;
	int variableCount;
	DeviceWriter * firstWriter;
protected:
	friend class Vector;
	void add(Vector * v);
public:
	Device(int variableCount);
	void dump(WriteBuffer & into);

	static Device & instance();
};


#endif /* INDIDEVICE_H_ */
