#include "Focuser.h"
#include "BaseDriver.h"

Focuser::Focuser(BaseDriver * bd, const uint8_t* pins, int suffix):
    Motor(pins, F("Focuser"), 12000, 8800),
    group(Symbol(F("FOCUSER"), suffix)),
    absolutePositionVec(group, Symbol(F("ABS_FOCUS_POSITION"), suffix), F("Absolute Position"), VECTOR_WRITABLE|VECTOR_READABLE),
    absolutePosition(&absolutePositionVec, F("FOCUS_ABSOLUTE_POSITION"), F("Ticks"), 0, 100000, 1),
    focusAbortMotionVec(group, Symbol(F("FOCUS_ABORT_MOTION"), suffix), F("Abort Motion"), VECTOR_WRITABLE|VECTOR_READABLE|VECTOR_SWITCH_ATMOSTONE),
    focusAbortMotion(&focusAbortMotionVec, F("ABORT"), F("Abort"))
{
    loadPosition(50000);
    onProgress();
    absolutePositionVec.onRequested(VectorCallback(&Focuser::absPositionChanged, this));
    focusAbortMotionVec.onRequested(VectorCallback(&Focuser::abortChanged, this));
    bd->addCapability(FOCUSER_INTERFACE);
}

void Focuser::absPositionChanged() {
    this->setTargetPosition(absolutePosition.getValue());
}

void Focuser::abortChanged() {
    if (this->focusAbortMotion.getValue()) {
        focusAbortMotion.setValue(false);
        if (this->isMoving()) {
            setTargetPosition(getCurrentPosition());
        }
    }
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
