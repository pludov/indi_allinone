/*
 * scheduler.cpp
 *
 *  Created on: 3 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "CommonUtils.h"
#include "Scheduler.h"

#undef SCHEDULER_DEBUG

Scheduler::Scheduler()
{
	this->first = 0;
}

// Wait the end of the previous signal if that is soon (< 5ms). Otherwise return false
boolean waitSigEndIfImmediate(const UTime & currentSigEnd) {
	long int wait;
	UTime currentNanos = UTime::now();
	wait = currentSigEnd - currentNanos;
	if (wait < 1000) {
		if (wait > 0)
			delayMicroseconds(wait);
		return true;
	} else {
		return false;
	}
}

static long lastMicros = 0;

static unsigned long lastDebugMicros = 0;
static int nextDebugId = 0;

void Scheduler::loop()
{
	long newMicros = micros();
	if (newMicros - lastMicros >  2000000) {
		DEBUG(F("alive"));
		lastMicros = newMicros;
	}
	yield();
	
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

	bool freeTime = false;
	if (targetForLoop) {
		if (waitSigEndIfImmediate(targetForLoop->nextTick)) {
#ifdef SCHEDULER_DEBUG
			int retard = 0;
			if (targetForLoop->priority == 0 && now > targetForLoop->nextTick) {
				retard = (now - targetForLoop->nextTick) >> 7;
			}
#endif
			auto start = micros();
			targetForLoop->tick();
			auto end = micros();
			targetForLoop->perf.addSample(end - start);
#ifdef SCHEDULER_DEBUG
			if (retard) {
				DEBUG(F("Too Late:"));
				DEBUG(retard / 8.0);
			}
#endif
			UTime now;
			if ((!targetForLoop->nextTick.isNever()) && targetForLoop->nextTick < (now  = UTime::now())) {
				targetForLoop->nextTick = now;
			}
		} else {
			freeTime = true;
		}
	} else {
		// DEBUG("No task to run");
		// Is this the right place for yield?
		freeTime = true;
	}

	if (freeTime) {
		unsigned long m = micros();
		if (m - lastDebugMicros > 2000000) {
			lastDebugMicros = m;

			bool displayed = false;
			int id = 0;
			DEBUG(F("Dumping latency sample"), nextDebugId);
			for(Scheduled * sch = first; sch; sch = sch->nextScheduled) {
				if (id == nextDebugId) {
					sch->debugPerf();
					displayed = true;
					break;
				}
				id++;
			}

			if (displayed) {
				nextDebugId++;
			} else {
				nextDebugId = 0;
			}
		}
	}
}

void Scheduler::yield()
{
	for(Scheduled * sch = first; sch; sch = sch->nextScheduled)
	{
		sch->idle();
	}
}

static Scheduler * instancePtr = 0;

Scheduler & Scheduler::instance() {
	if (!instancePtr) {
		instancePtr = new Scheduler();
	}
	return *instancePtr;
}


