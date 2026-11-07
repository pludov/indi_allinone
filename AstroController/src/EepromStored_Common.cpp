#include <Arduino.h>
#include "EepromStored.h"
#include "CommonUtils.h"


#define check(t, dplay) while(!(t)) {DEBUG(dplay);}

static void checkAddr(uint8_t v)
{
	check(v < 64, F("Invalid address"));
}

uint32_t EepromStored::Addr(uint8_t v1)
{
	checkAddr(v1);
	return v1;
}

uint32_t EepromStored::Addr(uint32_t v1, uint8_t v2)
{
	checkAddr(v2);
	int used = 1;
	while(v1 >> (8 * used)) {
		used ++;
	}
	check(used < 4, F("Addr overflow"));
	return v1 | (((uint32_t)v2) << (8*used));
}

uint32_t EepromStored::Addr(uint32_t v1, uint8_t v2, uint8_t v3)
{
	return Addr(Addr(v1, v2), v3);
}

uint32_t EepromStored::Addr(uint32_t v1, uint8_t v2, uint8_t v3, uint8_t v4)
{
	return Addr(Addr(Addr(v1, v2), v3), v4);
}
