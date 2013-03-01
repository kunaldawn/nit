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

#pragma warning (disable: 4996) // "may be unsafe using xxx instead"

////////////////////////////////////////////////////////////////////////////////

#include "LogManager.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

LogManager::LogManager()
{
	_shutdown = false;

	_defaultLogLevel = LOG_LEVEL_DEBUG;

	setLogLevel("***", LOG_LEVEL_ERROR);
	setLogLevel("!!!", LOG_LEVEL_FATAL);
	setLogLevel("..", LOG_LEVEL_VERBOSE);
	setLogLevel("--", LOG_LEVEL_DEBUG);
	setLogLevel("++", LOG_LEVEL_INFO);
	setLogLevel("??", LOG_LEVEL_WARNING);

	_root = new LogChannel(this);
}

LogManager::~LogManager()
{
	shutdown();

    _shutdown = true;

	*_threadRoot = NULL;
    _loggers.clear();
    _tags.clear();
    _root = NULL;

    _shutdown = false;
}

void LogManager::log(LogChannel* channel, const char* act, const char* srcname, uint line, const char* fnname, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vlog(channel, act, srcname, line, fnname, fmt, args);
	va_end(args);
}

void LogManager::vlog(LogChannel* channel, const char* act, const char* srcname, uint line, const char* fnname, const char* fmt, va_list args)
{
	// prepare message
	char msg[MAX_BUF_SIZE];
	int msgLen = vsnprintf(msg, sizeof(msg)-1, fmt, args);

	// TODO: handle with insufficient buffer (snprintf(NULL, ...) may work?)
	if (msgLen < 0)
	{
		assert(false);
		return;
	}

	// TODO: detect tag here first
	doLog(channel, act, srcname, line, fnname, NULL, msg, msgLen, false);
}

inline static bool isspace(char c)
{
	return c == '\t' || c == '\r' || c == '\n' || c == ' ';
}

const char* LogManager::parseTag(const char*& msg, int& msgLen, uint32& outTagBuf)
{
	if (msgLen < 0)
		msgLen = strlen(msg);

	// detect tag here
	char* tag = reinterpret_cast<char*>(&outTagBuf);
	*tag = 0;

	if (msgLen > 4 && isspace(msg[3]) && !isspace(msg[2]))
	{
		// '*** msg' or '&&1 msg' format (two same letter + a non-space letter)
		char t = msg[0];
		if (!isalnum(t) && msg[1] == t)
		{
			tag[0] = *msg++;
			tag[1] = *msg++;
			tag[2] = *msg++;
			tag[3] = 0; msg++;
			msgLen -= 4;
		}
	}

	if (*tag == 0 && msgLen > 3 && isspace(msg[2]))
	{
		// '++ msg' format (two same letter + a space letter)
		char t = msg[0];
		if (!isalnum(t) && msg[1] == t)
		{
			tag[0] = *msg++;
			tag[1] = *msg++;
			tag[2] = 0; msg++;
			msgLen -= 3;
		}
	}

	return tag;
}

void LogManager::doLog(LogChannel* channel, const char* act, const char* srcname, uint line, const char* fnname, const char* tag, const char* msg, int msgLen, bool forceLineEnd)
{
	if (_shutdown) return;

	if (channel == NULL)
		channel = needThreadRootChannel();

	Mutex::ScopedLock lock(channel->_mutex);

	if (msgLen < 0)
		msgLen = strlen(msg);

	assert(channel);

	LogEntry e;
	e.time = float(SystemTimer::now());
	e.channel = channel;
	e.srcName = srcname;
	e.line = line;
	e.fnName = fnname;
	e.act = act;

	uint32 tagbuf = 0;

	if (tag == NULL)
		tag = parseTag(msg, msgLen, tagbuf);

	if (*tag == 0)
	{
		// no tag detected: use previous tag
		tagbuf = channel->_prevTag;
		tag = reinterpret_cast<const char*>(&tagbuf);
	}
	else
	{
		// TODO: MT-Safe
		strcpy(reinterpret_cast<char*>(&channel->_prevTag), tag);
	}

	e.tagStr = tag;
	e.tagID = tagId(tag);
	e.logLevel = getLogLevel(e.tagID);


	const char* lineStart = msg;
	const char* lineEnd = msg;
	const char* msgEnd = msg + msgLen;
	const char* ch = msg;

	while (true)
	{
		if (ch < msgEnd && *ch != '\n')
		{
			++ch;
			continue;
		}

		lineEnd = ch;

		e.lineEnd = *ch == '\n' || forceLineEnd;
		e.message = lineStart;
		e.messageLen = lineEnd - lineStart;

		doLog(&e);

		if (ch >= msgEnd) break;

		e.srcName = NULL;
		e.line = 0;
		e.fnName = NULL;

		lineStart = ++ch;
		if (ch >= msgEnd) break;
	}
}

void LogManager::format(std::string& outString, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vformat(outString, fmt, args);
	va_end(args);
}

void LogManager::vformat(std::string& outString, const char* fmt, va_list args)
{
	char buf[MAX_BUF_SIZE];

	int bufLen = vsnprintf(buf, MAX_BUF_SIZE, fmt, args);
	if (bufLen < 0)
	{
		assert(false);
		return;
	}

	bool truncateIfTooLong = true;

	if (truncateIfTooLong && bufLen >= MAX_BUF_SIZE)
	{
		// TODO: check vsnprintf usage again - NOT USING YET
		int bufSize = bufLen + 1;
		char* largeBuf = (char*) malloc(bufSize);
		if (largeBuf != NULL)
		{
			bufLen = vsnprintf(buf, bufSize, fmt, args);
			outString = std::string(buf, bufLen);
			free(largeBuf);
		}
		else
		{
			outString = std::string(buf, bufLen);
		}
	}
	else
	{
		outString = std::string(buf, bufLen);
		if (bufLen >= MAX_BUF_SIZE)
			outString.append(" [CUT] ");
	}
}

void LogManager::beginScope(LogChannel* channel)
{
	++channel->_indent;
}

void LogManager::endScope(LogChannel* channel)
{
	--channel->_indent;
}

void LogManager::openChannel(LogChannel* channel)
{
	if (_shutdown) return;

	Mutex::ScopedLock lock(channel->_mutex);

	_mutex.lock();

	// notify to loggers that channel is opened
	for (uint i=0; i<_loggers.size(); ++i)
	{
		_loggers[i]->onChannelOpened(channel);
	}

	_mutex.unlock();

	LogChannel* parent = channel->getParent();

	log(parent ? parent : channel, "CO", NULL, 0, NULL, ".. LogChannel %s (%08x) opened\n", channel->getName().c_str(), channel);
}

void LogManager::closeChannel(LogChannel* channel)
{
	if (_shutdown) return;

	Mutex::ScopedLock lock(channel->_mutex);

	LogChannel* parent = channel->getParent();
	log(parent ? parent : channel, "CC", NULL, 0, NULL, ".. LogChannel %s (%08x) closed\n", channel->getName().c_str(), channel);

	_mutex.lock();

	// notify to loggers that channel is closed
	for (uint i=0; i<_loggers.size(); ++i)
	{
		_loggers[i]->onChannelClosed(channel);
	}
	_mutex.unlock();
}

LogChannel* LogManager::needThreadRootChannel()
{
	LogChannel* ch = *_threadRoot;

	if (ch == NULL)
	{
		Thread* th = Thread::current();
		ch = new LogChannel(th ? th->name().c_str() : "main", _root);

		*_threadRoot = ch;
	}

	return ch;
}

void LogManager::setThreadRootChannel(LogChannel* channel)
{
	*_threadRoot = channel;
}

void LogManager::setLogLevel(const char* tag, LogLevel level)
{
	_mutex.lock();

	LogTagID tagID = tagId(tag);
	Tags::iterator itr = _tags.find(tagID);

	// if no such tag yet, register it
	if (itr == _tags.end())
	{
		LogTagInfo ti;
		ti.level = level;
		_tags.insert(std::make_pair(tagID, ti));
	}
	else
	{
		// specify log level to the tag
		itr->second.level = level;
	}

	_mutex.unlock();
}

LogLevel LogManager::getLogLevel(LogTagID tagID)
{
	Tags::iterator itr = _tags.find(tagID);

	if (itr != _tags.end())
		return itr->second.level;

	return _defaultLogLevel;
}

void LogManager::doLog(const LogEntry* entry)
{
	if (_shutdown) return;

	_mutex.lock();

	LogLevel el = entry->logLevel;

	if (el == LOG_LEVEL_DEFAULT)
		el = _defaultLogLevel;

	for (uint i=0; i<_loggers.size(); ++i)
	{
		Logger* l = _loggers[i];

		LogLevel ll = l->getLogLevel();

		if (ll == LOG_LEVEL_DEFAULT)
			ll = _defaultLogLevel;

		if (el < ll)
			continue;

		l->doLog(entry);
		l->_lineStart = entry->lineEnd;
	}

	_mutex.unlock();
}

void LogManager::attach(Logger* logger)
{
	_mutex.lock();
	Ref<Logger> safe = logger;
	if (!logger->isSupported()) return;

	detach(logger);
	_loggers.push_back(logger);
	_mutex.unlock();
}

void LogManager::detach(Logger* logger)
{
	_mutex.lock();
	_loggers.erase(std::remove(_loggers.begin(), _loggers.end(), logger), _loggers.end());
	_mutex.unlock();
}

void LogManager::shutdown()
{
	_mutex.lock();
	doLog(_root, "SH", 0, 0, 0, "++", "Shutting down LogManager", -1, true);
	_shutdown = true;
	_loggers.clear();
	_shutdown = false;
	_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////

LogChannel::LogChannel(LogManager* manager)
{
	// For root channel
	_parent = NULL;
	_refCount = 0;
	_indent = 0;
	_name = "root";

	_prevTag = LogManager::tagId("..");
	
	if (manager == NULL)
		manager = &LogManager::getSingleton();
	
	manager->openChannel(this);
}

LogChannel::LogChannel(const char* name, LogChannel* parent)
{
	_parent = parent ? parent : LogManager::getSingleton().needThreadRootChannel(); 
	_refCount = 0;
	_indent = 0;
	_name = name;

	_prevTag = LogManager::tagId("..");

	LogManager::getSingleton().openChannel(this);
}

LogChannel::~LogChannel()
{
	LogManager::getSingleton().closeChannel(this);
}

////////////////////////////////////////////////////////////////////////////////

Logger::Logger()
{
	_refCount = 0;
	_lineStart = true;
	_logLevel = LogManager::getSingleton().getDefaultLogLevel();
}

int Logger::formatLog(const LogEntry* entry, char* buf, int bufSize)
{
	--bufSize;

	int milli = int(entry->time * 1000) % 1000;
	int sec = int(entry->time);
	int min = sec / 60;
	int hour = min / (60);

	bool shortForm = true;
	int sz = 0;

	if (shortForm)
	{
		sz = _snprintf(buf, bufSize, "%02d:%02d.%03d%10s> %2s%3s ",
			min,
			sec % 60,
			milli / 10,
			entry->channel->getName().c_str(), 
			entry->act ? entry->act : "",
			entry->tagStr);
	}
	else
	{
		sz = _snprintf(buf, bufSize, "%02d:%02d:%02d.%04d%10s> %2s%3s ",
		hour,
		min % 60,
		sec % 60,
		milli,
		entry->channel->getName().c_str(), 
		entry->act ? entry->act : "",
		entry->tagStr);
	}

	return sz;
}

////////////////////////////////////////////////////////////////////////////////

#if defined(NIT_WIN32)
DebuggerLogger::DebuggerLogger()
{
	// You may want to catch even ignored logs on DebuggerLogger.
	// If not, SetLogLevel on your code after construction.
	setLogLevel(LOG_LEVEL_IGNORED);
}

void DebuggerLogger::doLog(const LogEntry* entry)
{
	char buf[LogManager::MAX_BUF_SIZE];
	size_t bufSize = sizeof(buf) - 1;

	if (_lineStart)
	{
		int len = formatLog(entry, buf, sizeof(buf));

		if (len > 0)
			OutputDebugStringA(buf);
	}

	OutputDebugStringW(Unicode::toUtf16(entry->message, (int)entry->messageLen).c_str());

	bool enableErrorLog = false;

	if (enableErrorLog && entry->logLevel >= LOG_LEVEL_ERROR && entry->srcName && entry->fnName)
	{
		int sz = _snprintf(buf, sizeof(buf)-1, " at %s() line %d '%s'",
			entry->fnName, entry->line, entry->srcName);

		if (sz > 0) OutputDebugStringA(buf);
	}

	if (entry->lineEnd)
		OutputDebugStringA("\n");
}
#endif

////////////////////////////////////////////////////////////////////////////////

StdLogger::StdLogger(FILE* stdOut, FILE* stdErr, LogLevel errorLevel)
: _stdOut(stdOut), _stdErr(stdErr), _errorLevel(errorLevel)
{
}

void StdLogger::doLog(const LogEntry* entry)
{
	FILE* out = entry->logLevel < _errorLevel ? _stdOut : _stdErr;

	if (out == NULL) return;

	if (_lineStart)
	{
 		fputs(entry->tagStr, out);
 		fputs(" ", out);
	}

#if defined(NIT_WIN32)
	// NOTE: We can put unicode to std using fputws, but invalid characters with regions
	// could be displayed with the console's font setting.
	fputws(Unicode::toUtf16(entry->message, entry->messageLen).c_str(), out);
#else
	// OSX, IOS supports utf-8 as default
	fwrite(entry->message, entry->messageLen, 1, out);
#endif

	if (entry->lineEnd)
		fputs("\n", out);
}

StdLogger::~StdLogger()
{
	if (_stdOut)
		fflush(_stdOut);
	if (_stdErr)
		fflush(_stdErr);
}

////////////////////////////////////////////////////////////////////////////////

LogScope::LogScope( LogChannel* channel, const char* srcname, uint line, const char* fnname, const char* fmt, ... )
{
	LogManager& l = LogManager::getSingleton();

	if (channel == NULL)
		channel = l.needThreadRootChannel();

	_channel = channel;

	va_list args;
	va_start(args, fmt);
	l.vlog(channel, 0, srcname, line, fnname, fmt, args);
	va_end(args);

	l.beginScope(channel);
}

LogScope::~LogScope()
{
	LogManager& l = LogManager::getSingleton();

	l.endScope(_channel);
}

////////////////////////////////////////////////////////////////////////////////

LogTimeScope::LogTimeScope( LogChannel* channel, const char* srcname, uint line, const char* fnname, const char* fmt, ... )
{
	LogManager& l = LogManager::getSingleton();

	if (channel == NULL)
		channel = l.needThreadRootChannel();

	_channel = channel;

	va_list args;
	va_start(args, fmt);
	l.vformat(_logText, fmt, args);
	va_end(args);

	l.log(channel, "TS", srcname, line, fnname, "%s start\n", _logText.c_str());
	l.beginScope(channel);

	_startTime = SystemTimer::now();
}

LogTimeScope::~LogTimeScope()
{
	double elapsed = SystemTimer::now() - _startTime;

	LogManager& l = LogManager::getSingleton();

	l.endScope(_channel);
	l.log(_channel, "TE", NULL, 0, NULL, "%s end - elapsed %.3f\n", _logText.c_str(), float(elapsed));
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
