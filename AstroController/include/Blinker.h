#ifndef BLINKER_H_
#define BLINKER_H_

#include "Scheduled.h"

/**
 * Blinker is a scheduled task that blinks a led
 * The led blinks when no connection is established
 */
class Blinker: public Scheduled {
    uint8_t pin;
    uint8_t flipFlop;
    bool cnxState;
public:
    Blinker(uint8_t pin);
    virtual ~Blinker();
    virtual void tick();

    void setCnxState(bool state);
};

#endif
