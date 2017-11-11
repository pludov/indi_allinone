/*
 * scheduler.cpp
 *
 *  Created on: 3 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "debug.h"
#include "Scheduler.h"

Scheduler::Scheduler()
{
	this->first = 0;
}


// Wait the end of the previous signal
void waitSigEnd(unsigned long currentSigEnd) {
	long int wait;
	do {
		unsigned long int currentNanos = micros();
		wait = currentSigEnd - currentNanos;

		if (wait > 16000) {
			delayMicroseconds(10000);
		}
	} while (wait > 16000);
	if (wait > 0) {
		delayMicroseconds(wait);
	}
}

// Wait the end of the previous signal if that is soon (< 5ms). Otherwise return false
boolean waitSigEndIfImmediate(const UTime & currentSigEnd) {
	long int wait;
	UTime currentNanos = UTime::now();
	wait = currentSigEnd - currentNanos;
	if (wait < 5000) {
		if (wait > 0)
			delayMicroseconds(wait);
		return true;
	} else {
		return false;
	}
}

void Scheduler::loop()
{
	UTime now = UTime::now();
	Scheduled * targetForLoop = 0;
	// On prend en priorité les p0 qui arrivent bientot
	// On vé rifie qu'un process pris ne va pas empecher un p0 de s'executer.

	// 1- trouver un item P0 pret. Choisir le plus en retard
	for(Scheduled * sch = first; sch; sch = sch->nextScheduled)
	{
		if (sch->priority == 0 && !sch->nextTick.isNever()) {
			if (targetForLoop == 0 || targetForLoop->nextTick > sch->nextTick) {
				targetForLoop = sch;
			}
		}
	}

	// Regarder parmis les autres process avec priorités qui sont en retard et qui ne generont pas targetForLoop...
	Scheduled * targetP1 = 0;
	for(Scheduled * sch = first; sch; sch = sch->nextScheduled)
	{
		if (!sch->nextTick.isNever() && sch->priority > 0 && sch->nextTick <= now) {
			if (targetForLoop == 0 || targetForLoop->nextTick > (now + sch->tickExpectedDuration)) {
				if (targetP1 == 0
						|| targetP1->priority > sch->priority
						|| (targetP1->priority == sch->priority && targetP1->nextTick > sch->nextTick)) {
					targetP1 = sch;
				}
			}
		}
	}

	if (targetP1) targetForLoop = targetP1;

	if (targetForLoop) {
		if (waitSigEndIfImmediate(targetForLoop->nextTick)) {
#ifdef DEBUG
			int retard = 0;
			if (targetForLoop->priority == 0 && now > targetForLoop->nextTick) {
				retard = (now - targetForLoop->nextTick) >> 7;
			}
#endif
			targetForLoop->tick();
#ifdef DEBUG
			if (retard) {
				Serial.print(F("Too Late:"));
				Serial.println(retard / 8.0);
			}
#endif
			UTime now;
			if ((!targetForLoop->nextTick.isNever()) && targetForLoop->nextTick < (now  = UTime::now())) {
				targetForLoop->nextTick = now;
			}
		}
	} else {
		// Serial.println("No task to run");
	}
}


static Scheduler * instancePtr = 0;

Scheduler & Scheduler::instance() {
	if (!instancePtr) {
		instancePtr = new Scheduler();
	}
	return *instancePtr;
}


