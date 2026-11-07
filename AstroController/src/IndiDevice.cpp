/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "WriteBuffer.h"
#include "IndiDevice.h"
#include "IndiProtocol.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"

#include "CommonUtils.h"

static IndiDevice * mainDevice = 0;

IndiDevice & IndiDevice::instance()
{
	if (!mainDevice) {
		mainDevice = new IndiDevice(100);
	}
	return *mainDevice;
}


IndiDevice::IndiDevice(int variableCount)
{
	list = (IndiVector**)malloc(sizeof(IndiVector*)*variableCount);
	for(int i = 0; i < variableCount;++i) {
		this->list[i] = nullptr;
	}
	this->variableCount = 0;
	this->firstWriter = 0;
}

void IndiDevice::add(IndiVector * v)
{
	v->uid = 255;
	if (firstWriter) {
		DEBUG(F("vector added after writer : not supported"));
		return;
	}
	if (variableCount >= 254) {
		DEBUG(F("Limit of 254 vector reached"));
		return;
	}
	v->uid = variableCount;
	list[variableCount++]=v;
}
