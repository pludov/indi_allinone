#include "BaseDriver.h"

BaseDriver::BaseDriver():
    ival(0),
    group(F("General Info")),
    driverInfo(group, F("DRIVER_INFO"), F("Driver Info"), VECTOR_READABLE),
    interface(&driverInfo, F("DRIVER_INTERFACE"), F("Interface"), 6)
{
    interface.setValue("0");
}


void BaseDriver::addCapability(uint8_t v)
{
    uint16_t i = 1;
    i = i << v;
    ival |= i;

    i = ival;

    char output[9];
    char* p = &output[8];

    if (!i) {
        *(p--) = 0;
        *(p) = '0';
    } else {
        for(*p--=0;i;i/=10) *p--=i%10+0x30;
        ++p;
    }

    interface.setValue(p);
}