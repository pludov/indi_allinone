/*
 * scheduler.cpp
 *
 *  Created on: 3 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "CommonUtils.h"
#include "Scheduler.h"

PerfHistogram::PerfHistogram() {
    auto m = getLevelCount();
    histo = new uint32_t[m];
    for (int i = 0; i < m; i++) {
        histo[i] = 0;
    }
}

PerfHistogram::~PerfHistogram() {
    delete[] histo;
}

void PerfHistogram::addSample(uint32_t usec) {
    int i = 0;
    while (i < (getLevelCount() - 1) && getUsecLevel(i) < usec) {
        i++;
    }
    if (i < getLevelCount()) {
        histo[i]++;
    }
}

int PerfHistogram::getLevelCount() const {
    return 10;
}

// Return the threshold in microsecond for the given level
// This is defined for levels within [0 .. getCount() - 1]
uint32_t PerfHistogram::getUsecLevel(int level) const
{
    switch(level) {
        case 0:
            return 100;
        case 1:
            return 200;
        case 2:
            return 500;
        case 3:
            return 1000;
        case 4:
            return 2000;
        case 5:
            return 5000;
        case 6:
            return 10000;
        case 7:
            return 20000;
        case 8:
            return 50000;
        default:
            // Never happen. This is only valid within [0 .. getCount() - 1]
            return 100000;
    }
}
