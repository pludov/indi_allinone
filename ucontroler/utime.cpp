/*
 * utime.cpp
 *
 *  Created on: 4 févr. 2015
 *      Author: utilisateur
 */

#include <Arduino.h>
#include "utime.h"

uint8_t UTime::lastOverflowCount = 0;
unsigned long UTime::lastUs = 0;
