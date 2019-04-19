#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "CommonUtils.h"
#include "FilterWheel.h"
#include "BaseDriver.h"
#include "EepromStored.h"
#include "IndiVectorMemberStorage.h"

#define FILTER_MAX_POS 65535

FilterWheel::FilterWheel(BaseDriver * bd, uint32_t addr, const uint8_t* pins, int suffix)
:
    Motor(pins, F("Filterwheel"), 6000, 6000),
    group(Symbol(F("FILTERWHEEL"), suffix)),
    filterSlotVec(group, Symbol(F("FILTER_SLOT"), suffix), F("Filter Slot"), VECTOR_WRITABLE|VECTOR_READABLE),
    filterSlot(&filterSlotVec, F("FILTER_SLOT_VALUE"), F("Filter"), 0, FILTER_SLOT_COUNT - 1, 1),
    rawPosVec(group, Symbol(F("FILTER_RAW_POSITION"), suffix), F("Raw Position"), VECTOR_WRITABLE|VECTOR_READABLE),
    rawPos(&rawPosVec, F("FILTER_RAW_POSITION"), F("Ticks"), 0, 100000, 1),
    calibrateVec(group, Symbol(F("FILTER_CALIB"), suffix), F("Calibration"), VECTOR_WRITABLE|VECTOR_READABLE|VECTOR_SWITCH_ATMOSTONE),
    calibrate(&calibrateVec, F("CALIB"), F("Calibrate")),
    abortMotionVec(group, Symbol(F("FILTER_ABORT_MOTION"), suffix), F("Abort Motion"), VECTOR_WRITABLE|VECTOR_READABLE|VECTOR_SWITCH_ATMOSTONE),
    abortMotion(&abortMotionVec, F("ABORT"), F("Abort")),
    filterNamesVec(group, Symbol(F("FILTER_NAME"), suffix), F("Names"), VECTOR_WRITABLE|VECTOR_READABLE),
    filterPositionsVec(group, Symbol(F("FILTER_POS")), F("Positions"), VECTOR_WRITABLE|VECTOR_READABLE),
    hallVec(group, Symbol(F("FILTER_HALL"), suffix), F("Hall sensor"), VECTOR_READABLE),
    hall(&hallVec, Symbol(F("HALL_VALUE")), F("Value"),0 ,255, 1)
{
    pauseAfterMove = 100;
    maxAccelStep = 30;
    for(int i = 0; i < FILTER_SLOT_COUNT; ++i) {
        filterNames[i] = new IndiTextVectorMember(&filterNamesVec,
                                        Symbol(F("FILTER_SLOT_NAME_"), i+1),
                                        Symbol(F("Filter"), i+1),
                                        9);
        IndiVectorMemberStorage::remember(filterNames[i], EepromStored::Addr(addr, 56+i));
        filterPositions[i] = new IndiIntVectorMember(&filterPositionsVec,
                                        Symbol(F("FILTER_SLOT_POS_"), i+1),
                                        Symbol(F("Filter"), i+1),
                                        0,
                                        100000,
                                        1);
        IndiVectorMemberStorage::remember(filterPositions[i], EepromStored::Addr(addr, 48+i));
    }
    this->currentCalibration = CALIBRATION_IDLE;

    pinMode(this->motorPins[4], INPUT);
    // FIXME: read position from EEPROM
    loadPosition(50000);
    onProgress();
    rawPosVec.onRequested(VectorCallback(&FilterWheel::rawPosChanged, this));
    filterSlotVec.onRequested(VectorCallback(&FilterWheel::filterSlotChanged, this));
    abortMotionVec.onRequested(VectorCallback(&FilterWheel::abortChanged, this));
    bd->addCapability(FILTER_INTERFACE);
}

void FilterWheel::rawPosChanged() {
    if (this->currentCalibration) {
        // Busy - Ignore for now
        return;
    }
    this->setTargetPosition(rawPos.getValue());
}

void FilterWheel::filterSlotChanged() {
    if (this->currentCalibration) {
        // Busy - ignore for now
        return;
    }
    int32_t newValue = filterSlot.getValue();
    if (newValue < 1 || newValue > FILTER_SLOT_COUNT) {
        newValue = 1;
        filterSlot.setValue(1);
    }
    this->setTargetPosition(filterPositions[newValue - 1]->getValue());
}

void FilterWheel::abortChanged() {
    if (this->abortMotion.getValue()) {
        abortMotion.setValue(false);
        this->currentCalibration = CALIBRATION_IDLE;
        if (this->isMoving()) {
            setTargetPosition(getCurrentPosition());
        }
        onProgress();
    }
}

bool FilterWheel::readPin() {
    return digitalRead(this->motorPins[4]);
}

void FilterWheel::calibrateChanged() {
    if (this->currentCalibration) {
        return;
    }
    if (readPin()) {
        currentCalibration = CALIBRATION_WAITING_0;
    } else {
        currentCalibration = CALIBRATION_WAITING_1;
    }
    loadPosition(1000000);
    setTargetPosition(1100000);
    onProgress();
}

void FilterWheel::onProgress()
{
    bool busy = this->isActive();

    bool pin = readPin();
    hall.setValue(pin ? 1 : 0);

    rawPosVec.set(VECTOR_BUSY, busy);
    rawPos.setValue(this->currentPosition);

    if (currentCalibration) {
        if (pin == (currentCalibration == CALIBRATION_WAITING_1)) {
            // expected value received
            if (currentCalibration == CALIBRATION_WAITING_0) {
                currentCalibration = CALIBRATION_WAITING_1;
            } else {
                // Calibration done !. Go on to the selected filter
                currentCalibration = 0;
                // FIXME: don't erase the low bits
                loadPosition(100000);
                filterSlotChanged();
                onProgress();
                return;
            }
        }
        calibrateVec.set(VECTOR_BUSY, true);
        filterSlotVec.set(VECTOR_BUSY, true);
    } else {
        calibrateVec.set(VECTOR_BUSY, false);
        // Update the vector position. Set to err if no filter is found
        if (!busy) {
            unsigned long newValue = this->currentPosition;
            // Update the filter slot accordingly
            busy = true;
            for(int i = 0; i < FILTER_SLOT_COUNT; ++i) {
                if (filterPositions[i]->getValue() == newValue) {
                    filterSlot.setValue(i + 1);
                    busy = false;
                    break;
                }
            }
            // FIXME: save pos to EEPROM
        }
        filterSlotVec.set(VECTOR_BUSY, busy);
    }
}

void FilterWheel::onTargetPositionReached() {
    this->onProgress();
}
