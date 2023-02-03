#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <string.h>

#include "CommonUtils.h"
#include "Focuser.h"
#include "BaseDriver.h"
#include "EepromStored.h"

#define FOCUSER_DEFAULT_INC_PULSE       8800
#define FOCUSER_DEFAULT_DEC_PULSE       12000
#define FOCUSER_DEFAULT_MAX_ACCEL_STEP  100
#define FOCUSER_DEFAULT_INVERT          0

struct Settings {
    unsigned long pos;
    uint16_t incPulse, decPulse;
    uint16_t maxAccelStep;
    uint8_t invert;
};

class FocuserMemory: public EepromStored {
protected:
	virtual void decodeEepromValue(void * buffer, uint8_t sze){
		if (sze != sizeof(Settings)) {
			DEBUG(F("Focuser EEPROM invalid size"), sze);
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
	Settings settings;

	FocuserMemory(uint32_t addr): EepromStored(addr) {
		settings.pos = 50000;
        settings.decPulse = FOCUSER_DEFAULT_DEC_PULSE;
        settings.incPulse = FOCUSER_DEFAULT_INC_PULSE;
        settings.maxAccelStep = FOCUSER_DEFAULT_MAX_ACCEL_STEP;
        settings.invert =  FOCUSER_DEFAULT_INVERT;
	}

	void save()
	{
		write();
	}
};

Focuser::Focuser(BaseDriver * bd, uint32_t addr, const uint8_t* pins, int suffix):
    Motor(pins, F("Focuser"), FOCUSER_DEFAULT_DEC_PULSE, FOCUSER_DEFAULT_INC_PULSE),
    group(Symbol(F("FOCUSER"), suffix)),
    absolutePositionVec(group, Symbol(F("ABS_FOCUS_POSITION"), suffix), F("Absolute Position"), VECTOR_WRITABLE|VECTOR_READABLE),
    absolutePosition(&absolutePositionVec, F("FOCUS_ABSOLUTE_POSITION"), F("Ticks"), 0, 100000, 1),
    focusAbortMotionVec(group, Symbol(F("FOCUS_ABORT_MOTION"), suffix), F("Abort Motion"), VECTOR_WRITABLE|VECTOR_READABLE|VECTOR_SWITCH_ATMOSTONE),
    focusAbortMotion(&focusAbortMotionVec, F("ABORT"), F("Abort")),
    focusResetPositionVec(group, Symbol(F("FOCUS_RESET_POSITION"), suffix), F("Reset to 50k"), VECTOR_WRITABLE|VECTOR_READABLE|VECTOR_SWITCH_ATMOSTONE),
    focusResetPosition(&focusResetPositionVec, F("RESET"), F("Reset")),
    motorSettingsVec(group, Symbol(F("FOCUS_SETTINGS"), suffix), F("Motor settings"), VECTOR_WRITABLE|VECTOR_READABLE),
    incMotorPulse(&motorSettingsVec, F("INC_MOTOR_PULSE"), F("inc pulse (ms)"), 100, 100000, FOCUSER_DEFAULT_INC_PULSE),
    decMotorPulse(&motorSettingsVec, F("DEC_MOTOR_PULSE"), F("dec pulse (ms)"), 100, 100000, FOCUSER_DEFAULT_DEC_PULSE),
    motorMaxAccelStep(&motorSettingsVec, F("ACCEL_STEPS"), F("max accel steps"), 0, 100000, FOCUSER_DEFAULT_MAX_ACCEL_STEP),
    motorInvert(&motorSettingsVec, F("INVERT"), F("Invert motor"), 0, 1, FOCUSER_DEFAULT_INVERT),
    eepromReadyListener(EepromCallback(&Focuser::loadInitialSettings, this))
{
    memory = new FocuserMemory(EepromStored::Addr(addr, 44));
    setMaxAccelStep(FOCUSER_DEFAULT_MAX_ACCEL_STEP);
    loadPosition(50000, 0);
    setInvert(FOCUSER_DEFAULT_INVERT);
    onProgress();
    absolutePositionVec.onRequested(VectorCallback(&Focuser::absPositionChanged, this));
    focusAbortMotionVec.onRequested(VectorCallback(&Focuser::abortChanged, this));
    focusResetPositionVec.onRequested(VectorCallback(&Focuser::resetPositionChanged, this));
    motorSettingsVec.onRequested(VectorCallback(&Focuser::motorSettingsChanged, this));
    bd->addCapability(FOCUSER_INTERFACE);
}

void Focuser::loadMotorSettings(const Settings & settings) {
    setInvert(settings.invert);
    this->motorInvert.setValue(getInvert() ? 1 : 0);

    setMaxAccelStep(settings.maxAccelStep);
    this->motorMaxAccelStep.setValue(getMaxAccelStep());

    updatePulse(settings.incPulse, settings.decPulse);
    this->incMotorPulse.setValue(getFastestPerHalfStepAsc());
    this->decMotorPulse.setValue(getFastestPerHalfStepDesc());
}

void Focuser::loadInitialSettings()
{
    loadPosition(memory->settings.pos);
    loadMotorSettings(memory->settings);
    if (this->isMoving()) {
        setTargetPosition(getCurrentPosition());
    }
    onProgress();
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

void Focuser::resetPositionChanged() {
    if (this->focusResetPosition.getValue()) {
        loadPosition(50000);
        onProgress();
    }
}

void Focuser::motorSettingsChanged() {
    memory->settings.incPulse = incMotorPulse.getValue();
    memory->settings.decPulse = decMotorPulse.getValue();
    memory->settings.maxAccelStep = motorMaxAccelStep.getValue();
    memory->settings.invert = !!motorInvert.getValue();

    loadMotorSettings(memory->settings);

    if (this->isMoving()) {
        setTargetPosition(getCurrentPosition());
    }

    memory->save();
}

void Focuser::onProgress()
{
    bool busy = this->isActive();
    absolutePositionVec.set(VECTOR_BUSY, busy);
    absolutePosition.setValue(this->currentPosition);
    if ((!busy) && (memory->settings.pos != this->currentPosition)) {
        memory->settings.pos = this->currentPosition;
        memory->save();
    }
}

void Focuser::onTargetPositionReached() {
    this->onProgress();
}
