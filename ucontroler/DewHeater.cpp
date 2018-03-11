#include <Arduino.h>

#include "DewHeater.h"
#include "MeteoTemp.h"
#include "Utils.h"
#include "CommonUtils.h"
#include "EepromStored.h"
#include "IndiVectorMemberStorage.h"

#define STATUS_NEED_SCAN 0
#define STATUS_MEASURE 1
#define STATUS_IDLE 2

#define CONTROL_OFF 0
#define CONTROL_PWM_LEVEL 1
#define CONTROL_TARGET_TEMP 2
#define CONTROL_DEW_POINT_DELTA 2

#define STORED_ADDR_LENGTH 7
#define STORED_NAME_LENGTH 8

struct Settings {
	uint8_t opMode;
	uint8_t addr[STORED_ADDR_LENGTH];
	char name[STORED_NAME_LENGTH];
	float kp, ki, kd;
	float targetPwm, targetTemp, targetTempAbove;

	bool matchAddr(const uint8_t * taddr)
	{
		for(int i = 0; i < STORED_ADDR_LENGTH; ++i) {
			if (addr[i] != taddr[i]) return false;
		}
		return true;
	}

	void clearAddr()
	{
		for(int i = 0; i < STORED_ADDR_LENGTH; ++i) {
			addr[i] = 0;
		}
	}

	bool isEmptyAddr() {
		for(int i = 0; i < STORED_ADDR_LENGTH; ++i) {
			if (addr[i]) return false;
		}
		return true;
	}

	void setName(const char * v)
	{
		int i = 0;
		while(i < STORED_NAME_LENGTH) {
			char c = v[i];
			name[i] = c;
			i++;
			if (!c) break;
		}
		while(i < STORED_NAME_LENGTH) {
			name[i] = 0;
			i++;
		}
	}
};

const int storedSettingsCount = 3;

class DewHeatersMemory : public EepromStored {
private:
	static DewHeatersMemory * instance;
protected:

	virtual void decodeEepromValue(void * buffer, uint8_t sze){
		if (sze != getEepromSize()) {
			DEBUG(F("DewHeater EEPROM invalid size"), sze);
			return;
		}
		memcpy(settings, (void*)buffer, sze);
	}

	virtual void encodeEepromValue(void * buffer, uint8_t sze) {
		if (sze != getEepromSize()) return;
		memcpy(buffer, (void*)settings, sze);
	}

	virtual int getEepromSize() const {
		return sizeof(Settings) * storedSettingsCount;
	}

	// Bit mask of known slots (not saved)
	uint8_t slotSeen;
	Settings * settings;

	int8_t findPos(const uint8_t * addr) const
	{
		for(int i = 0; i < storedSettingsCount; ++i)
		{
			DEBUG(F("Eeprom: "), settings[i].addr[0], settings[i].addr[1], settings[i].addr[2], settings[i].addr[3], settings[i].addr[4]);
			if (settings[i].matchAddr(addr)) {
				return i;
			}
		}
		return -1;
	}

	int8_t findNewSlot() const
	{
		// Find an empty slot
		for(int i = 0; i < storedSettingsCount; ++i)
		{
			if (settings[i].isEmptyAddr()) {
				return i;
			}
		}

		// Select the last seen item ?
		for(int i = 0; i < storedSettingsCount; ++i) {
			if (!(slotSeen & (1 << i))) {
				return i;
			}
		}
		return 0;
	}

public:
	DewHeatersMemory(uint32_t addr): EepromStored(addr) {
		settings = new Settings[storedSettingsCount];
		slotSeen = 0;
	}

	void save(const Settings & newSettings)
	{
		int8_t slot = findPos(newSettings.addr);
		if (slot == -1) {
			slot = findNewSlot();
		}
		DEBUG(F("Saving to slot"), slot);
		// Don't overwrite now...
		slotSeen |= (1 << slot);
		memcpy((void*)(settings + slot), (const void*)&newSettings, sizeof(Settings));

		write();
	}

	void forget(uint8_t * addr)
	{
		int8_t slotId = findPos(addr);
		if (slotId == -1) {
			return;
		}
		// Just clear the addr
		settings[slotId].clearAddr();
		write();
	}


	const Settings * find(uint8_t * addr)
	{
		DEBUG(F("Looking for "), addr[0], addr[1], addr[2], addr[3], addr[4]);
		int8_t slotId = findPos(addr);
		if (slotId == -1) {
			return nullptr;
		}
		slotSeen |= (1 << slotId);
		return settings + slotId;
	}

	static DewHeatersMemory * getInstance() {
		if (instance == nullptr) {
			instance = new DewHeatersMemory(EepromStored::Addr(2));
		}
		return instance;
	}
};

DewHeatersMemory * DewHeatersMemory::instance = nullptr;



// Operating modes:
//   - Forced (targetPwm 0 -> 100%)
//   - target temperature (use pid to targetTemp)
//   - above dew point (use pid to stay a fixed °C above dewpoint
// This misses a "save" button
DewHeater::DewHeater(MeteoTemp * meteoTemp, uint8_t pin, uint8_t pwmPin, int suffix)
    :Scheduled(F("DewHeater")),
    group(Symbol(F("DEW_HEATER"), suffix)),

	statusVec(group, Symbol(F("DEW_HEATER_INFO_TEMP"), suffix), F("HW Temperature")),
    temperature(&statusVec, F("DEW_HEATER_TEMP_VALUE"), F("Readen Temper (C)"),-273.15, 100, 1),
    pwm(&statusVec, F("DEW_HEATER_PWM_LEVEL"), F("Power applied (%)"),0, 100, 1),
	dynTargetTemp(&statusVec, F("DEW_HEATER_CURR_TARGET_TEMP"), F("Target Temp (C)"), -273.15, 100, 1),

    uidVec(group, Symbol(F("DEW_HEATER_INFO_UID"), suffix), F("Unique Identifier")),
    uid(&uidVec, F("DEW_HEATER_UID_VALUE"), F("Unique Identifier"),12),

    nameVec(group, Symbol(F("DEW_HEATER_INFO_NAME"), suffix), F("Label"), VECTOR_WRITABLE|VECTOR_READABLE),
    name(&nameVec, F("DEW_HEATER_INFO_NAME"), F("Label"),STORED_NAME_LENGTH),

	powerMode(group, Symbol(F("DEW_HEATER_POWER_MODE"), suffix), F("Power Mode"), VECTOR_WRITABLE|VECTOR_READABLE),
	powerModeOff(&powerMode, F("POWER_MODE_OFF"), F("Off")),
	powerModeForced(&powerMode, F("POWER_MODE_FORCED"), F("Forced")),
	powerModeByTemp(&powerMode, F("POWER_MODE_BY_TEMP"), F("Target temp")),
	powerModeOverDew(&powerMode, F("POWER_MODE_OVER_DEW"), F("Follow dew point")),


	targetPwmVec(group, Symbol(F("DEW_HEATER_TARGET_PWM"),suffix), F("Fixed Power"), VECTOR_WRITABLE|VECTOR_READABLE),
	targetPwm(&targetPwmVec, F("TARGET_PWM"), F("0-100"), 0, 100, 0.1),

	targetTempVec(group, Symbol(F("DEW_HEATER_TARGET_TEMP"),suffix), F("Fixed Temperature"), VECTOR_WRITABLE|VECTOR_READABLE),
	targetTemp(&targetTempVec, F("TARGET_TEMPERATURE"), F("C"), -100, 100, 0.1),

	targetTempAboveVec(group, Symbol(F("DEW_HEATER_TARGET_TEMP_ABOVE_DP"),suffix), F("Above Dew Point"), VECTOR_WRITABLE|VECTOR_READABLE),
	targetTempAbove(&targetTempAboveVec, F("TARGET_TEMPERATURE_ABOVE_DP"), F("C"), -100, 100, 0.1),

	settingKpVec(group, Symbol(F("DEW_HEATER_PID_P"), suffix), F("PID Settings - P"), VECTOR_WRITABLE|VECTOR_READABLE),
	settingKp(&settingKpVec, F("P"), F("P"), -1000, 1000, 0.1),

	settingKiVec(group, Symbol(F("DEW_HEATER_PID_I"), suffix), F("PID Settings - I"), VECTOR_WRITABLE|VECTOR_READABLE),
	settingKi(&settingKiVec, F("I"), F("I"), -1000, 1000, 0.1),

	settingKdVec(group, Symbol(F("DEW_HEATER_PID_D"), suffix), F("PID Settings - D"), VECTOR_WRITABLE|VECTOR_READABLE),
	settingKd(&settingKdVec, F("D"), F("D"), -1000, 1000, 0.1),

	configOperationVec(group, Symbol(F("DEW_HEATER_CONFIG_OPERATION"), suffix), F("Eeprom"), VECTOR_WRITABLE|VECTOR_READABLE),
	configIdle(&configOperationVec, F("IDLE"), F("Idle")),
	configSave(&configOperationVec, F("SAVE"), F("Save")),
	configForget(&configOperationVec, F("FORGET"), F("Forget")),
	configReload(&configOperationVec, F("RELOAD"), F("Reload")),

    oneWire(pin)
{
	this->meteoTemp = meteoTemp;
    this->priority = 2;
    this->status = 0;
    this->pwmPin = pwmPin;
    this->failed();
    this->nextTick = UTime::now();
    ITerm = 0;
    pidRunning = false;
    pidDisabled = false;
    tempAvailable = false;
    lastTime = 0;
    lastInput = 0;
    ITerm = 0;

    // Ensure config item is available
    DewHeatersMemory::getInstance();

    pinMode(pwmPin, OUTPUT);
    // Use 50hz so that AC filtering may help
    analogWriteFrequency(pwmPin, 50);
    digitalWrite(pwmPin, 0);

    powerMode.onRequested(VectorCallback(&DewHeater::powerModeChanged, this));
    targetPwmVec.onRequested(VectorCallback(&DewHeater::powerModeChanged, this));
    targetTempVec.onRequested(VectorCallback(&DewHeater::powerModeChanged, this));
    targetTempAboveVec.onRequested(VectorCallback(&DewHeater::powerModeChanged, this));

    configOperationVec.onRequested(VectorCallback(&DewHeater::configOperationChanged, this));
}

uint8_t DewHeater::getPowerModeId() const
{
	if (powerModeForced.getValue()) return 1;
	if (powerModeByTemp.getValue()) return 2;
	if (powerModeOverDew.getValue()) return 3;
	return 0;
}

void DewHeater::setPowerModeId(uint8_t v)
{
	switch(v) {
	case 1:
			powerModeForced.setValue(true);
			break;
	case 2:
			powerModeByTemp.setValue(true);
			break;
	case 3:
			powerModeOverDew.setValue(true);
			break;
	default:
			powerModeOff.setValue(true);
	}
}

double DewHeater::getPidTarget()
{
	double rslt;
	if (powerModeOverDew.getValue()) {
		if (meteoTemp == nullptr) {
			DEBUG(F("No meteo attached"));
			return NAN;
		}
		if (!meteoTemp->isReady()) {
			DEBUG(F("Dew point not avaible"));
			return NAN;
		}
		rslt = meteoTemp->getDewPoint() + targetTempAbove.getDoubleValue();
		DEBUG(F("Targetting above dew point: "), rslt);
	} else {
		rslt = targetTemp.getDoubleValue();
		DEBUG(F("Targetting direct temp: "), rslt);
	}
	dynTargetTemp.setValue(rslt);
	return rslt;
}


void DewHeater::initPid()
{
	pidRunning = true;
	if (tempAvailable) {
		pidDisabled = false;
		lastInput = temperature.getDoubleValue();
		lastTime = millis();
		ITerm = 0;
	} else {
		pidDisabled = true;
	}

}

#define outMax 100
#define outMin 0

int DewHeater::updatePid()
{
	if (!*uid.getTextValue()) {
		pidDisabled = true;
		return 0;
	}

	if (!tempAvailable) {
		pidDisabled = true;
		return 0;
	}

	double target = getPidTarget();


	if (target == NAN) {
		pidDisabled = true;
		return 0;
	}

	if (pidDisabled) {
		initPid();
		return 0;
	}


	// Do actual pid loop
	uint32_t now = millis();
	uint32_t elapsed = now - lastTime;
	DEBUG(F("Running PID for "), elapsed, " ms");
	double sampleTimeInSec = elapsed / 1000.0;

	double Setpoint = target;

	//get the new input value
	double Input = temperature.getDoubleValue();
	DEBUG(F("Read temp is "), Input);
	//compute all working error variables
	double error = Setpoint - Input;

	DEBUG(F("Error is "), error);

    double Kp = settingKp.getValue();
    double Ki = settingKi.getValue();
    double Kd = settingKd.getValue();

	ITerm+=(Ki * sampleTimeInSec * error);
	if(ITerm>outMax) ITerm = outMax;
	else if(ITerm < outMin) ITerm = outMin;
	double dInput = (Input - lastInput);

	//compute PID output
	double Output = Kp * error + ITerm - Kd * dInput / sampleTimeInSec;
	DEBUG(F("Raw Output "), Output);

	if(Output > outMax) Output = outMax;
	else if(Output < outMin) Output = outMin;

	//remember some variables for next round
	lastInput = Input;
	lastTime = now;

	// Convertir les % en 0..255
	Output = Output * 255.0 / 100.0;

	return round(Output);
}

void DewHeater::stopPid()
{
	pidRunning = false;
}


void DewHeater::powerModeChanged()
{
	DEBUG(F("Power mode changed to"), powerMode.getCurrent()->name);
	// 0 - 255
	int effectivePwm = 0;

	if (powerModeOff.getValue() || powerModeForced.getValue()) {
		if (pidRunning) stopPid();
		if (powerModeForced.getValue()) {
			double d = targetPwm.getDoubleValue();
			if (d < 0) d = 0;
			if (d > 100) d = 100;
			effectivePwm = d * 255 / 100.0;
		} else {
			effectivePwm = 0;
		}
	} else {
		if (!pidRunning) {
			initPid();
			effectivePwm = 0;
		} else {
			effectivePwm = updatePid();
		}
	}

	analogWrite(pwmPin, effectivePwm);
	pwm.setValue(effectivePwm * 100.0 / 255.0);
}

void DewHeater::configOperationChanged()
{
	if (configSave.getValue()) {
		// Want a save
		saveToEeprom();
	} else if (configForget.getValue()) {
		DewHeatersMemory::getInstance()->forget(addr);
	} else if (configReload.getValue()) {
		loadFromEeprom();
	}
	configIdle.setValue(true);
}

void DewHeater::setPwmLevel(float level)
{
    pwm.setValue(level);
}

void DewHeater::failed()
{
	tempAvailable = false;
    uid.setValue("");
    name.setValue("");
    memset(addr, 0, 8);
    // FIXME: mark no temp available
    powerModeChanged();
    this->status = STATUS_NEED_SCAN;
    this->nextTick = UTime::now() + MS(1000);
}

static void formatAddr(byte * addr, char * buffer)
{
    addr++;
    for(int i = 0; i < 6; ++i)
    {
        uint8_t v = addr[i];
        *(buffer++) = Utils::hex(v >> 4);
        *(buffer++) = Utils::hex(v & 15);
    }
    *buffer = 0;
}

void DewHeater::scan()
{
    DEBUG(F("scan"));
    
    // FIXME: duration for search ?
    oneWire.reset_search();
    if (!oneWire.search(addr)) {
        failed();
        return;
    }
    if (OneWire::crc8(addr, 7) != addr[7]) {
        failed();
        return;
    }
    if (addr[0] != 0x28) {
        // Mauvais type de capteur
        failed();
        return;
    }
    
    oneWire.select(addr);

    char strAddr[32];
    formatAddr(addr, strAddr);
    uid.setValue(strAddr);
    
    DEBUG(F("scan ok at "), strAddr);
    loadFromEeprom();

    this->status = STATUS_IDLE;
    this->nextTick = UTime::now() + MS(100);
}

void DewHeater::loadFromEeprom()
{
	// Update variables from scan...
	const Settings * settings = DewHeatersMemory::getInstance()->find(addr);
	if (settings != nullptr) {
		DEBUG(F("Loading pid values"));
		setPowerModeId(settings->opMode);
		settingKp.setValue(settings->kp);
		settingKi.setValue(settings->ki);
		settingKd.setValue(settings->kd);
		targetPwm.setValue(settings->targetPwm);
		targetTemp.setValue(settings->targetTemp);
		targetTempAbove.setValue(settings->targetTempAbove);

		char buffer[STORED_NAME_LENGTH + 1];
		memcpy(buffer, settings->name, STORED_NAME_LENGTH);
		buffer[STORED_NAME_LENGTH] = 0;
		name.setValue(buffer);

		powerModeChanged();
	} else {
		DEBUG(F("Not found"));
	}
}

void DewHeater::saveToEeprom()
{
	Settings settings;
	settings.setName(name.getTextValue());
	settings.kp = settingKp.getValue();
	settings.ki = settingKi.getValue();
	settings.kd = settingKd.getValue();
	settings.targetPwm = targetPwm.getValue();
	settings.targetTemp = targetTemp.getValue();
	settings.targetTempAbove = targetTempAbove.getValue();
	settings.opMode = getPowerModeId();
	memcpy(settings.addr, addr, STORED_ADDR_LENGTH);
	DewHeatersMemory::getInstance()->save(settings);
}

void DewHeater::startMeasure()
{
    long t1 = micros();
    DEBUG(F("startMeasure"));
    oneWire.reset();
    oneWire.skip();
    oneWire.write(0x44, 1);
    long t2 = micros();
    this->status = STATUS_MEASURE;
    this->nextTick = UTime::now() + MS(800);
    DEBUG(F("startmeasure done in "), t2 - t1);
}

void DewHeater::endMeasure()
{
    DEBUG(F("endMeasure"));
    long t1 = micros();
    oneWire.reset();
    oneWire.skip();
    //oneWire.select(addr);
    oneWire.write(0xBE);
    
    byte data[9];
    for (byte i = 0; i < 9; i++) {
        data[i] = oneWire.read();
    }
    long t2 = micros();
    DEBUG(F("measure done in "), t2 - t1);

    if (OneWire::crc8(data, 8) != data[8]) {
        DEBUG(F("Invalid data"));
        failed();
        return;
    }

    temperature.setValue(((data[1] << 8) | data[0]) * 0.0625);
    tempAvailable = true;
    if (pidRunning) powerModeChanged();

    this->status = STATUS_IDLE;
    this->nextTick = UTime::now() + MS(5000);
}

void DewHeater::tick()
{
    if (this->status == STATUS_NEED_SCAN) {
        scan();
    } else {
        if (this->status == STATUS_IDLE) {
            startMeasure();
        } else {
            endMeasure();

        }
    }
}
