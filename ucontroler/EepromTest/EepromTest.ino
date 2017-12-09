#include <Arduino.h>

#include "EepromStored.h"

#include "CommonUtils.h"

class EepromTest : public EepromStored
{
	uint32_t value;
protected:

	virtual void decodeEepromValue(void * buffer, uint8_t sze)
	{
		if (sze < 4) {
			return;
		}
		value = *(uint32_t*)buffer;
	}

	virtual void encodeEepromValue(void * buffer, uint8_t sze)
	{
		if (sze < 4) {
			return;
		}
		*(uint32_t*)buffer = value;
	}

	// Size required in byte
	virtual int getEepromSize() const
	{
		return 16; //4 + 998 / 2 - 1;
	}
public:
	EepromTest(uint32_t addr) : EepromStored(addr), value(0)
	{}

	virtual ~EepromTest() {}

	void setValue(uint32_t v)
	{
		this->value = v;
		write();
	}
	uint32_t getValue()
	{
		return value;
	}
};
EepromTest * a;
EepromTest * b;

void setup()
{
	Serial.begin(115200);
	delay(500);
	DEBUG(F("Welcome to nano test\n"));
	a = new EepromTest(EepromStored::Addr(127, 2));
	b = new EepromTest(EepromStored::Addr(127, 3));
	a->setValue(125);
	b->setValue(4444);
	EepromTest * ep[48];
	for(int i = 0; i < 48; ++i) {
		ep[i] = new EepromTest(EepromStored::Addr(127,10+i));
		while(!ep[i]) {
			DEBUG(F("oom"));
		}
		//ep[i]->setValue(i);
	}
	EepromStored::init();
	for(int i = 0; i < 48; ++i) {
		if (ep[i]->getValue() != i) {
			DEBUG(F("mismatch at i="), i);
		}
	}
	DEBUG(F("nano test done\n"));
}

void loop()
{
	delay(5000);
	DEBUG(F("."));
	//a->setValue(rand());
}
