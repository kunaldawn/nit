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

#include "nit_pch.h"

#include "nit/data/DateTime.h"

#if defined(NIT_WIN32)
extern "C" char * strptime(const char *buf, const char *fmt, struct tm *tm);
#endif

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Timestamp::Timestamp()
{
	_unixTime64 = 0;
}

Timestamp::Timestamp(const DateTime& d)
{
	// Take d and make a utc time
	_unixTime64 = _mkgmtime(const_cast<tm*>(d.getSysDateTime()));
	_unixTime64 -= d.getTimeZoneDiff();
}

Timestamp::Timestamp(int64 unixTime64)
{
	// TODO: Platforms which don't support 64 bit time_t
	// cannot work beyond year 2038
	_unixTime64 = unixTime64;
}

Timestamp::Timestamp(const String& iso8601)
{
	if (!setIso8601(iso8601))
		NIT_THROW_FMT(EX_SYNTAX, "invalid iso8601 format: '%s'", iso8601.c_str());
}

bool Timestamp::operator == (const Timestamp& other) const
{
	return _unixTime64 == other._unixTime64;
}

bool Timestamp::operator != (const Timestamp& other) const
{
	return _unixTime64 != other._unixTime64;
}

bool Timestamp::operator < (const Timestamp& other) const
{
	return _unixTime64 < other._unixTime64;
}

bool Timestamp::operator > (const Timestamp& other) const
{
	return _unixTime64 > other._unixTime64;
}

// NOTE: It says that time_t is not numerically linear and not suitable for common plus minus arithmatic. 
// TODO: check platform-wise 

Timestamp Timestamp::operator + (double seconds) const
{
	// TODO: check platform-wise
	return Timestamp(_unixTime64 + SysTimestamp(seconds * SECOND));
}

Timestamp Timestamp::operator - (double seconds) const
{
	// TODO: check platform-wise
	return Timestamp(_unixTime64 - SysTimestamp(seconds * SECOND));
}

double Timestamp::operator - (const Timestamp& other) const
{
	return difftime((SysTimestamp)_unixTime64, (SysTimestamp)other._unixTime64);
}

int64 Timestamp::getUnixTime64() const
{
	return _unixTime64;
}

Timestamp& Timestamp::operator+=(double seconds)
{
	// TODO: check platform-wise
	_unixTime64 += SysTimestamp(seconds * SECOND);
	return *this;
}

Timestamp& Timestamp::operator-=(double seconds)
{
	// TODO: check platform-wise
	_unixTime64 -= SysTimestamp(seconds * SECOND);
	return *this;
}

Timestamp Timestamp::now()
{
	return Timestamp((int64)time(NULL));
}

Timestamp Timestamp::ZERO()
{
	return Timestamp(0);
}

String Timestamp::format(const char* fmt) const
{
	return isValid() ? DateTime(_unixTime64).format(fmt) : StringUtil::BLANK();
}

String Timestamp::formatz(const char* fmt) const
{
	return isValid() ? DateTime(_unixTime64).formatz(fmt) : StringUtil::BLANK();
}

bool Timestamp::setIso8601(const String& str)
{
	if (str.empty())
	{
		_unixTime64 = 0;
		return true;
	}

	DateTime d;
	bool ok = d.setIso8601(str);
	if (!ok) return false;

	*this = Timestamp(d);

	return true;
}

String Timestamp::getIso8601() const
{
	if (_unixTime64 == 0) return StringUtil::BLANK();

	// UTC is comfortable for convert, sort etc
	SysTimestamp tt = (SysTimestamp)_unixTime64;
	SysDateTime dt = *gmtime(&tt);

	char buf[128];
	strftime(buf, COUNT_OF(buf), "%Y-%m-%dT%H:%M:%Sz", &dt);
	
#if defined(NIT_WIN32)
	return Unicode::reencode(buf, Unicode::CODE_SYSTEM, Unicode::CODE_UTF8);
#else
	return buf;
#endif
}

////////////////////////////////////////////////////////////////////////////////

DateTime::DateTime()
: _timeZoneDiff(0)
{
	memset(&_sysDateTime, 0, sizeof(_sysDateTime));
	_sysDateTime.tm_year = -1;
}

DateTime::DateTime(const Timestamp& t)
{
	if (t.isValid())
	{
		// take t and make a local time
		SysTimestamp tt = (SysTimestamp)t.getUnixTime64();
		_sysDateTime = *localtime(&tt);
		_timeZoneDiff = getSysTimeZoneDiff();
	}
	else
	{
		memset(&_sysDateTime, 0, sizeof(_sysDateTime));
		_sysDateTime.tm_year	= -1;
		_timeZoneDiff			= 0;
	}
}

DateTime::DateTime(const String& iso8601)
{
	memset(&_sysDateTime, 0, sizeof(_sysDateTime));
	_timeZoneDiff = 0;

	if (!setIso8601(iso8601))
		NIT_THROW_FMT(EX_SYNTAX, "invalid iso8601 format: '%s'", iso8601.c_str());
}

DateTime::DateTime(SysDateTime* _tm, int tzd)
: _sysDateTime(*_tm), _timeZoneDiff(tzd)
{

}

DateTime DateTime::now()
{
	SysTimestamp tt = time(NULL);
	return DateTime(localtime(&tt), getSysTimeZoneDiff());
}

DateTime DateTime::nowUtc()
{
	SysTimestamp tt = time(NULL);
	return DateTime(gmtime(&tt), 0);
}

DateTime DateTime::today(int hour, int minute, int second)
{
	DateTime today = now();
	today._sysDateTime.tm_hour = hour;
	today._sysDateTime.tm_min = minute;
	today._sysDateTime.tm_sec = second;

	return today;
}

DateTime DateTime::todayUtc(int hour /*= 0*/, int minute /*= 0*/, int second /*= 0*/)
{
	DateTime today = nowUtc();
	today._sysDateTime.tm_hour = hour;
	today._sysDateTime.tm_min = minute;
	today._sysDateTime.tm_sec = second;

	return today;
}

DateTime DateTime::wantDay(int year, int month, int day, 
							 int hour, int minute, int second, 
							 int wday, int yday, bool summer)
{
	DateTime dtday;
	dtday._sysDateTime.tm_year = year;
	dtday._sysDateTime.tm_mon = month;
	dtday._sysDateTime.tm_mday = day;
	dtday._sysDateTime.tm_hour = hour;
	dtday._sysDateTime.tm_min = minute;
	dtday._sysDateTime.tm_sec = second;
	dtday._sysDateTime.tm_wday = wday;
	dtday._sysDateTime.tm_yday = yday;
	dtday._sysDateTime.tm_isdst = summer;
	dtday._timeZoneDiff = getSysTimeZoneDiff();
	return dtday;
}

DateTime DateTime::wantDayUtc(int year, int month, int day, int hour, int minute, int second, int wday, int yday, bool summer)
{
	DateTime dtday;
	dtday._sysDateTime.tm_year = year;
	dtday._sysDateTime.tm_mon = month;
	dtday._sysDateTime.tm_mday = day;
	dtday._sysDateTime.tm_hour = hour;
	dtday._sysDateTime.tm_min = minute;
	dtday._sysDateTime.tm_sec = second;
	dtday._sysDateTime.tm_wday = wday;
	dtday._sysDateTime.tm_yday = yday;
	dtday._sysDateTime.tm_isdst = summer;
	dtday._timeZoneDiff = 0;
	return dtday;
}

String DateTime::format(const char* fmt) const
{
	if (!isValid()) return StringUtil::BLANK();

	char buf[128];

	strftime(buf, COUNT_OF(buf), fmt, &_sysDateTime);

#if defined(NIT_WIN32)
	return Unicode::reencode(buf, Unicode::CODE_SYSTEM, Unicode::CODE_UTF8);
#else
	return buf;
#endif
}

String DateTime::formatz(const char* fmt) const
{
	if (!isValid()) return StringUtil::BLANK();

	char buf[128];

	strftime(buf, COUNT_OF(buf), fmt, &_sysDateTime);

	String ret;

#if defined(NIT_WIN32)
	ret = Unicode::reencode(buf, Unicode::CODE_SYSTEM, Unicode::CODE_UTF8);
#else
	ret = buf;
#endif

	int tzd = _timeZoneDiff;

	if (tzd == 0) return ret + " UTC";

	if (tzd < 0) tzd *= -1;

	int tdMin = tzd / (60 * Timestamp::SECOND);
	int tdHour = tdMin / 60;

	return ret + StringUtil::format(" %c%02d:%02d", _timeZoneDiff < 0 ? '-' : '+', tdHour, tdMin % 60);

}

bool DateTime::setIso8601(const String& str)
{
	if (str.empty())
	{
		_sysDateTime.tm_year = -1;
		return true;
	}

	bool hasSecond = true;

	const char* p = strptime(str.c_str(), "%Y-%m-%dT%H:%M:%S", &_sysDateTime);

	if (p == NULL) 
		p = strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &_sysDateTime);

	if (p == NULL)
	{
		hasSecond = false;
		p = strptime(str.c_str(), "%Y-%m-%d %H:%M", &_sysDateTime);
	}

	if (p == NULL)
	{
		hasSecond = false;
		p = strptime(str.c_str(), "%Y-%m-%d", &_sysDateTime);
	}

	if (p == NULL)
		return false;

	if (hasSecond && *p == '.')
	{
		// Read fraction part of second :
		// Currently timestamp doesn't handle such precision so we skip.
		++p;

		while (*p && isdigit(*p)) ++p;
	}

	int sign = 1, tzh = 0, tzm = 0;

	switch (*p)
	{
	case 0:		
		_timeZoneDiff = getSysTimeZoneDiff(); 
		return true;

	case 'Z': case 'z':	_timeZoneDiff = 0; 
		return true;

	case '-':	
		sign = -1; // fall through
	case '+':	
		sscanf(++p, "%d:%d", &tzh, &tzm);
		_timeZoneDiff = tzh * 60 * 60 * Timestamp::SECOND;
		_timeZoneDiff += tzm * 60 * Timestamp::SECOND;
		_timeZoneDiff *= sign;
		return true;

	default:	return false;
	}
}

String DateTime::getIso8601() const
{
	if (!isValid()) return StringUtil::BLANK();

	if (isUtc()) return format("%Y-%m-%dT%H:%M:%Sz");

	int tzd = _timeZoneDiff;
	if (tzd < 0) tzd *= -1;

	int tdMin = tzd / (60 * Timestamp::SECOND);
	int tdHour = tdMin / 60;

	return format("%Y-%m-%dT%H:%M:%S") + StringUtil::format("%c%02d:%02d", _timeZoneDiff < 0 ? '-' : '+', tdHour, tdMin % 60);
}

static int CalcStzd()
{
	SysTimestamp now = time(NULL);		// time in utc

	SysDateTime ldt = *localtime(&now);
	SysTimestamp ltime = _mkgmtime(&ldt);// time in local

	double diffsec = difftime(ltime, now);

	int stzd = int(diffsec * Timestamp::SECOND);

	return stzd;
}

int DateTime::getSysTimeZoneDiff()
{
	static int stzd = CalcStzd();
	return stzd;
}

DateTime DateTime::toUtc() const
{
	if (isUtc()) return *this;

	Timestamp t(*this); // Timestamp is always in utc

	SysTimestamp tt = (SysTimestamp)t.getUnixTime64();
	return DateTime(gmtime(&tt), 0);
}


DateTime DateTime::toLocal() const
{
	if (_timeZoneDiff == getSysTimeZoneDiff()) return *this;

	// For timestamp is always in utc, we can just convert a Timestamp to a DateTime again.
	return DateTime(Timestamp(*this));
}

DateTime DateTime::toLocal(int timeZoneDiffInSecond) const
{
	Timestamp t(*this); // Timestamp is always in utc

	SysTimestamp tt = (SysTimestamp)t.getUnixTime64() + timeZoneDiffInSecond;
	return DateTime(gmtime(&tt), timeZoneDiffInSecond);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
