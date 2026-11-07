#ifndef PERF_HISTOGRAM_H_
#define PERF_HISTOGRAM_H_

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#endif

class PerfHistogram {
	uint32_t*histo;
public:
	PerfHistogram();
	~PerfHistogram();

	void addSample(uint32_t usec);
	int getLevelCount() const;
	uint32_t getUsecLevel(int level) const;
	uint32_t getSampleCount(int level) const {
		return histo[level];
	};
};

#endif
