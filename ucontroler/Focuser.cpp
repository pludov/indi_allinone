#include "Focuser.h"

Focuser::Focuser(const uint8_t* pins, int suffix):
    Motor(pins, F("Focuser"), 12000, 8800),
    group(Symbol(F("FOCUSER"), suffix)),
    absolutePositionVec(group, Symbol(F("ABS_FOCUS_POSITION"), suffix), F("Absolute Position"), VECTOR_WRITABLE|VECTOR_READABLE),
    absolutePosition(&absolutePositionVec, F("FOCUS_ABSOLUTE_POSITION"), F("Ticks"), 0, 100000, 1)
{
    loadPosition(50000);
    onProgress();
    absolutePositionVec.onRequested(VectorCallback(&Focuser::absPositionChanged, this));
}

void Focuser::absPositionChanged() {
    this->setTargetPosition(absolutePosition.getValue());
}

void Focuser::onProgress()
{
    bool busy = this->isActive();
    absolutePositionVec.set(VECTOR_BUSY, busy);
    absolutePosition.setValue(this->currentPosition);
}

void Focuser::onTargetPositionReached() {
    this->onProgress();
}
