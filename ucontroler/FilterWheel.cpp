#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <string.h>

#include "CommonUtils.h"
#include "FilterWheel.h"
#include "BaseDriver.h"
#include "EepromStored.h"
#include "IndiVectorMemberStorage.h"

#define POS_INVALID ((uint32_t)0x7fffffff)

struct Settings {
	uint32_t pos;
};

class FilterWheelMemory : public EepromStored {
protected:
	virtual void decodeEepromValue(void * buffer, uint8_t sze){
		if (sze != sizeof(Settings)) {
			DEBUG(F("FilterWheel EEPROM invalid size"), sze);
			return;
		}
		memcpy(&settings, (void*)buffer, sze);
	}

	virtual void encodeEepromValue(void * buffer, uint8_t sze) {
		if (sze != sizeof(Settings)) return;
		memcpy(buffer, (void*)&settings, sze);
	}

	virtual int getEepromSize() const {
		return sizeof(Settings);
	}
public:
	// Bit mask of known slots (not saved)
	Settings settings;

	FilterWheelMemory(uint32_t addr): EepromStored(addr) {
		settings.pos = POS_INVALID;
	}

	void save()
	{
		write();
	}
};

FilterWheel::FilterWheel(BaseDriver * bd, uint32_t addr, const uint8_t* pins, int suffix)
:
    Motor(pins, F("Filterwheel"), 4000, 4000),
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
    hall(&hallVec, Symbol(F("HALL_VALUE")), F("Value"),0 ,255, 1),
    eepromReadyListener(EepromCallback(&FilterWheel::loadInitialSettings, this))
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
    loadPosition(50000);
    onProgress();
    rawPosVec.onRequested(VectorCallback(&FilterWheel::rawPosChanged, this));
    filterSlotVec.onRequested(VectorCallback(&FilterWheel::filterSlotChanged, this));
    abortMotionVec.onRequested(VectorCallback(&FilterWheel::abortChanged, this));
    calibrateVec.onRequested(VectorCallback(&FilterWheel::calibrateChanged, this));
    bd->addCapability(FILTER_INTERFACE);
    memory = new FilterWheelMemory(EepromStored::Addr(addr, 47));
}

void FilterWheel::loadInitialSettings()
{
    if (memory->settings.pos == POS_INVALID) {
        // Will stay idle in calibration state
        this->currentCalibration = 2;
    } else {
        this->currentCalibration = CALIBRATION_IDLE;
        loadPosition(memory->settings.pos);
        if (this->isMoving()) {
            setTargetPosition(getCurrentPosition());
        }
    }
    onProgress();
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

void FilterWheel::saveMemoryPos(uint32_t value)
{
    if (!EepromStored::eepromReady()) {
        return;
    }
    if (memory->settings.pos != value) {
        memory->settings.pos = value;
        memory->save();
    }
}


void FilterWheel::onProgress()
{
    bool busy = this->isActive();

    bool pin = readPin();
    hall.setValue(pin ? 1 : 0);

    rawPosVec.set(VECTOR_BUSY, busy);
    rawPos.setValue(this->currentPosition);

    if (busy) {
        saveMemoryPos(POS_INVALID);
    }

    if (currentCalibration) {
        if (pin == (currentCalibration == CALIBRATION_WAITING_1)) {
            // expected value received
            if (currentCalibration == CALIBRATION_WAITING_0) {
                currentCalibration = CALIBRATION_WAITING_1;
            } else {
                // Calibration done !. Go on to the selected filter
                currentCalibration = 0;
                // FIXME: don't erase the low bits
                clearOutput();
                loadPosition(100000);
                setTargetPosition(100000);
                saveMemoryPos(100000);
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
            uint32_t newValue = this->currentPosition;
            // Update the filter slot accordingly
            busy = true;
            for(int i = 0; i < FILTER_SLOT_COUNT; ++i) {
                if (filterPositions[i]->getValue() == newValue) {
                    filterSlot.setValue(i + 1);
                    busy = false;
                    break;
                }
            }
            saveMemoryPos(newValue);
        }
        filterSlotVec.set(VECTOR_BUSY, busy);
    }
}

void FilterWheel::onTargetPositionReached() {
    this->onProgress();
}
