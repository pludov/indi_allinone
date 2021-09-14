#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <string.h>

#include "CommonUtils.h"
#include "Focuser.h"
#include "BaseDriver.h"
#include "EepromStored.h"

struct Settings {
    unsigned long pos;
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
	}

	void save()
	{
		write();
	}
};

Focuser::Focuser(BaseDriver * bd, uint32_t addr, const uint8_t* pins, int suffix):
    Motor(pins, F("Focuser"), 12000, 8800),
    group(Symbol(F("FOCUSER"), suffix)),
    absolutePositionVec(group, Symbol(F("ABS_FOCUS_POSITION"), suffix), F("Absolute Position"), VECTOR_WRITABLE|VECTOR_READABLE),
    absolutePosition(&absolutePositionVec, F("FOCUS_ABSOLUTE_POSITION"), F("Ticks"), 0, 100000, 1),
    focusAbortMotionVec(group, Symbol(F("FOCUS_ABORT_MOTION"), suffix), F("Abort Motion"), VECTOR_WRITABLE|VECTOR_READABLE|VECTOR_SWITCH_ATMOSTONE),
    focusAbortMotion(&focusAbortMotionVec, F("ABORT"), F("Abort")),
    focusResetPositionVec(group, Symbol(F("FOCUS_RESET_POSITION"), suffix), F("Reset to 50k"), VECTOR_WRITABLE|VECTOR_READABLE|VECTOR_SWITCH_ATMOSTONE),
    focusResetPosition(&focusResetPositionVec, F("RESET"), F("Reset")),
    eepromReadyListener(EepromCallback(&Focuser::loadInitialSettings, this))
{
    memory = new FocuserMemory(EepromStored::Addr(addr, 44));
    loadPosition(50000, 0);
    onProgress();
    absolutePositionVec.onRequested(VectorCallback(&Focuser::absPositionChanged, this));
    focusAbortMotionVec.onRequested(VectorCallback(&Focuser::abortChanged, this));
    focusResetPositionVec.onRequested(VectorCallback(&Focuser::resetPositionChanged, this));
    bd->addCapability(FOCUSER_INTERFACE);
}

void Focuser::loadInitialSettings()
{
    loadPosition(memory->settings.pos);
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
