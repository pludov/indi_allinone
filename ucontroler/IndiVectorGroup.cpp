#ifdef ARDUINO
#include <Arduino.h>
#endif
#include "IndiVectorGroup.h"
#include "WriteBuffer.h"
#include "Utils.h"

IndiVectorGroup::IndiVectorGroup(Symbol name, int8_t suffix)
	: name(name)
{
    this->suffix = suffix;
}

/*void IndiVectorGroup::dumpXmlEncoded(WriteBuffer & into) const
{
    into.appendXmlEscaped(name);
    if (suffix) {
        into.append('_');
        into.append(Utils::hex(suffix));
    }
}
*/
