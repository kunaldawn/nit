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

#include "nit/platform/SystemTimer.h"

////////////////////////////////////////////////////////////////////////////////

#if !defined(NIT_NO_LOG)

// Generally, specify CH as 0 as default (default channel for the current thread)
#define LOG(CH, ...)					::nit::LogManager::getSingleton().log(CH, 0, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_SCOPE(CH, ...)				auto ::nit::LogScope __log_scope(CH, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define LOG_TIMESCOPE(CH, ...)			auto ::nit::LogTimeScope __log_timescope(CH, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);

#else // #if defined(NIT_NO_LOG)

#define LOG(CH, ...)					(void)0
#define LOG_SCOPE(CH, ...)				(void)0
#define LOG_TIMESCOPE(CH, ...)			(void)0

#endif // #if defined(NIT_NO_LOG)

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class Logger;
class LogManager;

class NIT_API LogChannel
{
public:
	LogChannel(const char* name, LogChannel* parent = NULL);
	~LogChannel();

public: // tiny refcounted
	LogChannel*							_ref()									{ return this; }
	void								incRefCount()							{ ++_refCount; }
	void								decRefCount()							{ if (--_refCount == 0) delete this; }

public:
	LogChannel*							getParent()	const						{ return _parent; }
	int									getIndent()	const						{ return _indent; }
	const std::string&					getName() const							{ return _name; }

private:
	Ref<LogChannel>						_parent;
	int									_refCount;
	int									_indent;
	std::string							_name;

	uint32								_prevTag;

	Mutex								_mutex;

	friend class						LogManager;
	LogChannel(LogManager* manager = NULL);
};

////////////////////////////////////////////////////////////////////////////////

enum LogLevel
{
	LOG_LEVEL_DEFAULT = -1,
	LOG_LEVEL_IGNORED = 0,

	LOG_LEVEL_VERBOSE,					// ..
	LOG_LEVEL_DEBUG,					// --
	LOG_LEVEL_INFO,						// ++
	LOG_LEVEL_WARNING,					// ??
	LOG_LEVEL_ERROR,					// ***
	LOG_LEVEL_FATAL,					// !!!

	LOG_LEVEL_QUIET,
};

typedef uint16 LogTagID;

struct NIT_API LogTagInfo
{
	LogLevel							level;
};

////////////////////////////////////////////////////////////////////////////////

struct NIT_API LogEntry
{
	float								time;
	const LogChannel*					channel;
	LogLevel							logLevel;
	LogTagID							tagID;
	const char*							act;
	const char*							srcName;
	uint								line;
	const char*							fnName;
	const char*							tagStr;
	const char*							message;
	size_t								messageLen;
	bool								lineEnd;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API LogManager
{
public:
	static LogManager&					getSingleton()							{ static LogManager s_Instance; return s_Instance; }

public:
	void								shutdown();

public:
	const static int					MAX_BUF_SIZE = 2048;

	void								log(LogChannel* channel, const char* act, const char* srcname, uint line, const char* fnname, const char* fmt, ...);
	void								vlog(LogChannel* channel, const char* act, const char* srcname, uint line, const char* fnname, const char* fmt, va_list args);
	
	void								format(std::string& outString, const char* fmt, ...);
	void								vformat(std::string& outString, const char* fmt, va_list args);

	void								beginScope(LogChannel* channel);
	void								endScope(LogChannel* channel);

	void								doLog(LogChannel* channel, const char* act, const char* srcname, uint line, const char* fnname, const char* tag, const char* msg, int msgLen, bool forceLineEnd);
	void								doLog(const LogEntry* entry);

	const char*							parseTag(const char*& msg, int& msgLen, uint32& outTagBuf);

public:
	void								attach(Logger* logger);
	void								detach(Logger* logger);

public:
	LogChannel*							needThreadRootChannel();
	void								setThreadRootChannel(LogChannel* channel);

public:
	void								setDefaultLogLevel(LogLevel level)		{ _defaultLogLevel = level; }
	LogLevel							getDefaultLogLevel()					{ return _defaultLogLevel; }

	static LogTagID						tagId(const char* tag)					{ return tag ? (tag[0] << 8) | (tag[0] ? tag[1] : 0) : 0; }
	void								setLogLevel(const char* tag, LogLevel level);
	LogLevel							getLogLevel(LogTagID tagID);

private:
	LogManager();
	~LogManager();

private:
	LogLevel							_defaultLogLevel;

	typedef std::vector<Ref<Logger> >	Loggers;
	Loggers								_loggers;

	typedef std::map<uint16, LogTagInfo> Tags;
	Tags								_tags;

	Ref<LogChannel>						_root;
	ThreadLocal<Ref<LogChannel> >		_threadRoot;

	Mutex								_mutex;
	bool								_shutdown;

	friend class LogChannel;
	void								openChannel(LogChannel* channel);
	void								closeChannel(LogChannel* channel);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API Logger // tiny-ref-counted
{
public:
	Logger();
	virtual ~Logger() { }

public:
	virtual bool						isSupported()							{ return true; }

	virtual void						doLog(const LogEntry* entry) = 0;

	LogLevel							getLogLevel()							{ return _logLevel; }
	void								setLogLevel(LogLevel level)				{ _logLevel = level; }

	virtual void						onChannelOpened(LogChannel* channel)	{ }
	virtual void						onChannelClosed(LogChannel* channel)	{ }

	int									formatLog(const LogEntry* entry, char* buf, int bufSize);

public: // Tiny ref-counted
	Logger*								_ref()									{ return this; }
	void								incRefCount()							{ _refCount++; }
	void								decRefCount()							{ if (--_refCount == 0) destroy(); }
	virtual void						destroy()								{ delete this; }

protected:
	friend class						LogManager;
	bool								_lineStart;

private:
	int									_refCount;
	LogLevel							_logLevel;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API StdLogger : public Logger
{
public:
	StdLogger(FILE* stdOut, FILE* stdErr, LogLevel errorLevel = LOG_LEVEL_VERBOSE);
	~StdLogger();

	virtual void						doLog(const LogEntry* entry);

	FILE*								_stdOut;
	FILE*								_stdErr;
	LogLevel							_errorLevel;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API DebuggerLogger : public Logger
{
public:
	DebuggerLogger();

public:
	virtual void						doLog(const LogEntry* entry);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API LogScope
{
public:
	LogScope(LogChannel* channel, const char* srcname, uint line, const char* fnname, const char* fmt, ...);
	~LogScope();

private:
	Ref<LogChannel>						_channel;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API LogTimeScope
{
public:
	LogTimeScope(LogChannel* channel, const char* srcname, uint line, const char* fnname, const char* fmt, ...);
	~LogTimeScope();

private:
	Ref<LogChannel>						_channel;
	std::string							_logText;
	double								_startTime;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
