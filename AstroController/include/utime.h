/*
 * utime.h
 *
 *  Created on: 4 févr. 2015
 *      Author: utilisateur
 */

#ifndef UTIME_H_
#define UTIME_H_

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#endif

class ShortDuration
{
	friend class UTime;
	friend class LongDuration;
	unsigned int duration16us;
public:
	ShortDuration()
	{
		this->duration16us = 0;
	}
	ShortDuration(unsigned int duration16us)
	{
		this->duration16us = duration16us;
	}
};

/** Durée jusqu'à une heure */
class LongDuration
{
	friend class UTime;
	unsigned long int durationUs;
public:
	LongDuration()
	{
		this->durationUs = 0;
	}
	LongDuration(unsigned long int durationUs)
	{
		this->durationUs = durationUs;
	}
	LongDuration(const ShortDuration & duration)
	{
		this->durationUs = ((unsigned long)duration.duration16us) << 4;
	}

	static LongDuration seconds(int i)
	{
		return LongDuration(((unsigned long)i)*1000000);
	}
};

/** Encapsulate time with operator/functino
 * Range : 2^40 us, soit 305h (12j)
 */
class UTime {
private:
	static uint8_t lastOverflowCount;
	static unsigned long lastUs;

	// Nombre de us (2^32)
	unsigned long us;
	// overflow (2^8)
	uint8_t overflowcount;
	bool neverFlag;
private:

	UTime(bool isNever)
	{
		this->neverFlag = isNever;
		this->us = 0;
		this->overflowcount = 0;
	}


public:
	UTime()
	{
		us = lastUs;
		overflowcount = lastOverflowCount;
		neverFlag = false;
	}

	~UTime()
	{
	}

	bool operator<(const UTime & t) const
	{
		int8_t delta = overflowcount - t.overflowcount;
		if (delta < 0) return true;
		if (delta > 0) return false;
		return us < t.us;
	}

	bool operator<=(const UTime & t) const
	{
		int8_t delta = overflowcount - t.overflowcount;
		if (delta < 0) return true;
		if (delta > 0) return false;
		return us <= t.us;
	}

	bool operator>(const UTime & t) const
	{
		int8_t delta = overflowcount - t.overflowcount;
		if (delta > 0) return true;
		if (delta < 0) return false;
		return us >t.us;
	}

	bool operator>=(const UTime & t) const
	{
		int8_t delta = overflowcount - t.overflowcount;
		if (delta > 0) return true;
		if (delta < 0) return false;
		return us >= t.us;
	}

	// Fait une soustraction avec saturation dans les borne des int.
	long int operator-(const UTime & t) const
	{
		// On veut limiter à maxi +0x7fffffff, -0x80000000
		int overflowDist = (int)overflowcount - (int)t.overflowcount;
		if (overflowDist > 0) {
			// t était bien avant. on doit retourner un positif
			if (overflowDist > 1 || us >= t.us) {
				return 0x7fffffff;
			}
			// On a exactement un écart de 1 overflow, et t.us > us
			long ecart = us - t.us;
			if (ecart < 0) return 0x7ffffff;
			return ecart;
		} else if (overflowDist < 0) {
			// t est bien apres. on doit retourner un negatif.
			if (overflowDist < -1 || us <= t.us) {
				return -0x8000000;
			}
			// On a exactement un ecart de 1 overflow et t.us < us
			long ecart = us - t.us;
			if (ecart > 0) return -0x80000000;
			return ecart;
		}
		// Même overflowdist
		// On veut retourner us - t.us
		if (us >= t.us) {
			// on veut retourner un positif ou null
			long ecart = us - t.us;
			if (ecart < 0) return 0x7fffffff;
			return ecart;
		} else {
			// On veut retourner un negatif
			long ecart = us - t.us;
			if (ecart >= 0) return -0x80000000;
			return ecart;
		}
	}

	UTime operator+(unsigned long int usToAdd) const
	{
		UTime result = *this;
		result += usToAdd;
		return result;
	}

	UTime & operator+=(unsigned long usToAdd)
	{
		unsigned long newMs = us + usToAdd;
		if (newMs < us) {
			// Overflow
			overflowcount++;
		}
		us = newMs;
		return *this;
	}

	UTime operator+(const ShortDuration & toAdd) const
	{
		UTime result = *this;
		result += ((unsigned long)toAdd.duration16us) << 4;
		return result;
	}

	UTime & operator+=(const ShortDuration & toAdd)
	{
		return this->operator +=(((unsigned long)toAdd.duration16us) << 4);
	}

	UTime operator+(const LongDuration & toAdd) const
	{
		UTime result = *this;
		result += toAdd.durationUs;
		return result;
	}

	UTime & operator+=(const LongDuration & toAdd)
	{
		return this->operator+=(toAdd.durationUs);
	}

	static UTime never()
	{
		return UTime(true);
	}

	static UTime now();

	bool isNever()
	{
		return neverFlag;
	}

	unsigned long ms()
	{
		return ((unsigned long long)us + (((unsigned long long)overflowcount) << 32)) / 1000;
	}

	unsigned long minutes()
	{
		return ((unsigned long long)us + (((unsigned long long)overflowcount) << 32)) / 60000000;
	}
};

#define MS(x) ShortDuration((125 * (long)(x)) / 2)
#define US(x) ShortDuration((x) / 16)

#endif /* UTIME_H_ */
