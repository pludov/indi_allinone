#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

class Utils {

public:
	static char hex(uint8_t v) {
		if (v < 10) {
			return '0' + v;
		}
		return 'a' - 10 + v;
	}
};

#endif /* STATUS_H_ */
