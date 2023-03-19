#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <string.h>

#include "CommonUtils.h"
#include "FilterWheel.h"
#include "BaseDriver.h"
#include "EepromStored.h"
#include "IndiVectorMemberStorage.h"

#define DEFAULT_MOTOR_PULSE 4000

#define POS_INVALID ((uint32_t)0x7fffffff)

struct Settings {
	uint32_t pos;
    // Base of the last calibration
    uint8_t base;
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
        settings.base = 0;
	}

	void save()
	{
		write();
	}
};

FilterWheel::FilterWheel(BaseDriver * bd, uint32_t addr, const uint8_t* pins, int suffix)
:
    Motor(pins, F("Filterwheel"), DEFAULT_MOTOR_PULSE, DEFAULT_MOTOR_PULSE),
    group(Symbol(F("FILTERWHEEL"), suffix)),
    filterSlotVec(group, Symbol(F("FILTER_SLOT"), suffix), F("Filter Slot"), VECTOR_WRITABLE|VECTOR_READABLE),
    filterSlot(&filterSlotVec, F("FILTER_SLOT_VALUE"), F("Filter"), 0, FILTER_SLOT_COUNT - 1, 1),
    rawPosVec(group, Symbol(F("FILTER_RAW_POSITION"), suffix), F("Raw Position"), VECTOR_WRITABLE|VECTOR_READABLE),
    rawPos(&rawPosVec, F("FILTER_RAW_POSITION"), F("Ticks"), 0, 100000, 1),
    motorSettingsVec(group, Symbol(F("FILTER_MOTOR_SETTINGS"), suffix), F("Motor settings"), VECTOR_WRITABLE|VECTOR_READABLE),
    motorPulse(&motorSettingsVec, F("FILTER_MOTOR_PULSE"), F("pulse (ms)"), 100,100000, DEFAULT_MOTOR_PULSE),
    motorBacklash(&motorSettingsVec, F("FILTER_MOTOR_BACKLASH"), F("backlash"), -10000,10000, 0),
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
    memory = new FilterWheelMemory(EepromStored::Addr(addr, 47));
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
    IndiVectorMemberStorage::remember(&motorPulse, EepromStored::Addr(addr, 46));
    IndiVectorMemberStorage::remember(&motorBacklash, EepromStored::Addr(addr, 45));
    this->currentCalibration = CALIBRATION_IDLE;

    pinMode(this->motorPins[4], INPUT);
    loadPosition(50000, 0);
    onProgress();
    rawPosVec.onRequested(VectorCallback(&FilterWheel::rawPosChanged, this));
    filterSlotVec.onRequested(VectorCallback(&FilterWheel::filterSlotChanged, this));
    motorSettingsVec.onRequested(VectorCallback(&FilterWheel::motorSettingsChanged, this));
    abortMotionVec.onRequested(VectorCallback(&FilterWheel::abortChanged, this));
    calibrateVec.onRequested(VectorCallback(&FilterWheel::calibrateChanged, this));
    bd->addCapability(FILTER_INTERFACE);
}

void FilterWheel::loadInitialSettings()
{
    if (memory->settings.pos == POS_INVALID) {
        // Will stay idle in calibration state
        this->currentCalibration = 2;
    } else {
        this->currentCalibration = CALIBRATION_IDLE;
        loadPosition(memory->settings.pos, memory->settings.base);
        if (this->isMoving()) {
            setTargetPosition(getCurrentPosition());
        }
    }
    this->motorSettingsChanged();
    onProgress();
}

void FilterWheel::rawPosChanged() {
    if (this->currentCalibration) {
        // Busy - Ignore for now
        return;
    }
    gotoRawPos(rawPos.getValue());
}

void FilterWheel::gotoRawPos(int32_t target) {
    int32_t backlash = motorBacklash.getValue();
    int32_t intermediate = target;
    int32_t current = getCurrentPosition();
    if (backlash != 0) {
        if (backlash > 0) {
            if (target < current) {
                intermediate = target - backlash;
                if (intermediate < 0) {
                    intermediate = 0;
                }
            }
        } else {
            if (target > current) {
                intermediate = target - backlash;
            }
        }
    }
    if (target < 0) {
        target = 0;
    }
    if (intermediate < 0) {
        intermediate = 0;
    }
    this->setTargetPosition(target, intermediate);
}

void FilterWheel::motorSettingsChanged() {
    this->updatePulse(motorPulse.getValue(), motorPulse.getValue());
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
    int32_t target = filterPositions[newValue - 1]->getValue();
    gotoRawPos(target);
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
    loadPosition(1000000 + ((currentPosition + positionBase) & 7), 0);
    setTargetPosition(1100000);
    onProgress();
}

void FilterWheel::saveMemoryPos(uint32_t value, uint8_t positionBaseValue)
{
    if (!EepromStored::eepromReady()) {
        return;
    }
    if (memory->settings.pos != value || memory->settings.base != positionBaseValue) {
        memory->settings.pos = value;
        memory->settings.base = positionBaseValue;
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
        saveMemoryPos(POS_INVALID, 0);
    }

    if (currentCalibration) {
        if (pin == (currentCalibration == CALIBRATION_WAITING_1)) {
            // expected value received
            if (currentCalibration == CALIBRATION_WAITING_0) {
                currentCalibration = CALIBRATION_WAITING_1;
            } else {
                // Calibration done !. Go on to the selected filter
                currentCalibration = 0;
                clearOutput();
                // Keep the low bits for precision
                loadPosition(100000, (currentPosition + positionBase) & 7);
                setTargetPosition(100000);
                saveMemoryPos(100000, positionBase);
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
            saveMemoryPos(newValue, positionBase);
        }
        filterSlotVec.set(VECTOR_BUSY, busy);
    }
}

void FilterWheel::onTargetPositionReached() {
    this->onProgress();
}
