/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <inttypes.h>

#include "CommonUtils.h"
#include "WriteBuffer.h"
#include "Utils.h"
#include "IndiVector.h"

WriteBuffer::WriteBuffer(uint8_t * into, int size)
{
	this->ptr = into;
	this->left = size;
	this->totalSize = size;
}

WriteBuffer::~WriteBuffer()
{}

void WriteBuffer::append(char c)
{
	if (this->left) {
		this->ptr[0] = c;
		this->ptr++;
		this->left--;
	} else {
		this->left = -1;
		FATAL(F("WriteBuffer overflow"));
	}
}

int WriteBuffer::size() const
{
	return totalSize - left;
}

bool WriteBuffer::isEmpty() const
{
	return totalSize == left;
}

bool WriteBuffer::finish()
{
	if (left) {
		*ptr = 0;
		return true;
	}
	return false;
}
