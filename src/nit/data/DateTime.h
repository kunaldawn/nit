/// nit - Noriter Framework
/// A Cross-platform Open Source Integration for Game-oriented Apps
///
/// http://www.github.com/ellongrey/nit
///
/// Copyright (c) 2013 by Jun-hyeok Jang
/// 
/// (see each file to see the different copyright owners)
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// Author: ellongrey

#pragma once

#include "nit/nit.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

typedef time_t SysTimestamp;
typedef struct tm SysDateTime;
class Timestamp;
class DateTime;

#define NIT_TIME_T_RESOLUTION 1 // TODO: If a platform differs from this, move to SysConfig.h

////////////////////////////////////////////////////////////////////////////////

class NIT_API Timestamp
{
public:
	Timestamp();
	Timestamp(int64 unixTime64);
	Timestamp(const DateTime& d);
	Timestamp(const String& iso8601);

public:
	bool								operator == (const Timestamp& other) const;
	bool								operator != (const Timestamp& other) const;
	bool								operator < (const Timestamp& other) const;
	bool								operator > (const Timestamp& other) const;

	Timestamp							operator + (double seconds) const;
	Timestamp							operator - (double seconds) const;
	double								operator - (const Timestamp& other) const;
	Timestamp&							operator += (double seconds);
	Timestamp&							operator -= (double seconds);

	enum
	{
		SECOND							= NIT_TIME_T_RESOLUTION,
		MINUTE							= SECOND * 60,
		HOUR							= MINUTE * 60,
		DAY								= HOUR * 24,
		WEEK							= DAY * 7,

		// Number of month, year cannot be defined here as they differ year to year.
		// TODO: Implement a Calendar class
	};

public:
	static Timestamp					now();
	static Timestamp					ZERO();
	static size_t						sysTimestampSize()						{ return sizeof(SysTimestamp); }

public:
	bool								setIso8601(const String& str);
	String								getIso8601() const;

	String								format(const char* fmt = "%Y-%m-%d %H:%M:%S") const;
	String								formatz(const char* fmt = "%Y-%m-%d %H:%M:%S") const;

public:
	bool								isValid() const							{ return _unixTime64 != 0; }

	int64								getUnixTime64() const;

private:
	// We'll serialize with DataSaver as binary layout
	// so be careful not to add any virtual function or other member variables!
	// This must be equal to an int64 object at binary level
	int64								_unixTime64;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DateTime
{
public:
	DateTime();
	DateTime(SysDateTime* _tm, int tzd);
	DateTime(const Timestamp& t);
	DateTime(const String& iso8601);

public:
	static DateTime						now();
	static DateTime						nowUtc();

	static DateTime						today(int hour = 0, int minute = 0, int second = 0);
	static DateTime						todayUtc(int hour = 0, int minute = 0, int second = 0);

	// TODO : Cannot be most accurate. We need a Calendar class
	static DateTime						wantDay(int year, int month, int day, 
											int hour, int minute, int second, 
											int wday, int yday, bool summer);
	static DateTime						wantDayUtc(int year, int month, int day, 
											int hour, int minute, int second, 
											int wday, int yday, bool summer);

public:
	bool								isValid() const							{ return _sysDateTime.tm_year >= 0; }

	int									getSecond() const						{ return _sysDateTime.tm_sec; }
	int									getMinute() const						{ return _sysDateTime.tm_min; }
	int									getHour() const							{ return _sysDateTime.tm_hour; }
	int									getDay() const							{ return _sysDateTime.tm_mday; }
	int									getMonth() const						{ return _sysDateTime.tm_mon; }
	int									getYear() const							{ return _sysDateTime.tm_year; }
	int									getWeekDay() const						{ return _sysDateTime.tm_wday; }
	int									getYearDay() const						{ return _sysDateTime.tm_yday; }
	bool								isSummerTime() const					{ return _sysDateTime.tm_isdst != 0; }

	int									getTimeZoneDiff() const					{ return _timeZoneDiff; } // in second
	bool								isUtc() const							{ return _timeZoneDiff == 0; }

	DateTime							toUtc() const;
	DateTime							toLocal() const;
	DateTime							toLocal(int timeZoneDiffInSecond) const;

	bool								setIso8601(const String& str);
	String								getIso8601() const;

	String								format(const char* fmt = "%Y-%m-%d %H:%M:%S") const;
	String								formatz(const char* fmt = "%Y-%m-%d %H:%M:%S") const;

public:
	const SysDateTime*					getSysDateTime() const					{ return &_sysDateTime; }

private:
	SysDateTime							_sysDateTime;
	int									_timeZoneDiff;
	static int							getSysTimeZoneDiff();
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;