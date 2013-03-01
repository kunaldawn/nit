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

#include "nit/event/Timer.h"

#include "nit/app/Service.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define svc_Input						static_cast<nit::InputService*>(g_Service->getService(::nit::Service::SVC_INPUT))

////////////////////////////////////////////////////////////////////////////////

class InputUser;
class InputDevice;
class InputSource;
class InputCommand;
class InputEvent;
class InputCommandEvent;

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputService : public Service
{
public:
	InputService();
	virtual ~InputService();

public:
	InputUser*							getSystemUser()							{ return _systemUser; }
	InputUser*							getDefaultUser()						{ return _defaultUser; }

public:
	InputUser*							newUser(const String& name);

	size_t								getUserCount()							{ return _users.size(); }
	InputUser*							getUser(const String& name);
	InputUser*							getUser(uint32 index);
	void								findUser(const String& pattern, vector<InputUser*>::type& outResults);

	void								setDefaultUser(InputUser* user);

public:
	TickTimer*							getTimer()								{ return _timer; }

	EventChannel*						commandChannel()						{ return _commandChannel ? _commandChannel : _commandChannel = new EventChannel(); }

	void								post(EventId id, InputEvent* evt, EventChannel* channel = NULL);
	void								post(InputCommandEvent* command);
	void								cancel(EventId id, InputEvent* evt, InputUser* user, InputDevice* device, InputSource* source, InputCommand* command);

public:									// Module Impl
	virtual void						onRegister();
	virtual void						onUnregister();
	virtual void						onInit();
	virtual void						onFinish();

private:
	Ref<TickTimer>						_timer;

	Ref<EventChannel>					_commandChannel;

	typedef map<String, Ref<InputUser> >::type UserNameLookup;
	UserNameLookup						_users;
	Ref<InputUser>						_systemUser;
	Ref<InputUser>						_defaultUser;

	struct EventEntry
	{
		EventId							eventID;
		Ref<InputEvent>					event;
		Weak<EventChannel>				channel;
	};

	typedef list<EventEntry>::type		EventQueue;
	EventQueue							_eventQueue;

	int									_frame;

	void								onTick(const TimeEvent* evt);
	void								processQueue(EventQueue& queue);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputEvent : public Event
{
public:
	virtual InputUser*					getUser() const;
	virtual InputDevice*				getDevice() const;
	virtual InputSource*				getSource()	const						{ return NULL; }
	virtual InputCommand*				getCommand() const						{ return NULL; }

	bool								isRelated(InputUser* user) const;
	bool								isRelated(InputDevice* device) const;
	bool								isRelated(InputSource* source) const;
	bool								isRelated(InputCommand* command) const;

protected:
	Ref<InputEvent>						_related;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ScopedInputLock : public DefaultAlloc
{
public:
	ScopedInputLock();
	~ScopedInputLock();

public:
	void								lock(InputUser* user, bool reset = true, const String& pattern = "*");
	void								lock(InputDevice* device, bool reset = true);
	void								lock(InputSource* source, bool reset = true);
	
	void								unlock(InputUser* user, const String& pattern = "*");
	void								unlock(InputDevice* device);
	void								unlock(InputSource* source);

	void								leave(bool unlock = true);

private:
	class LockEntry;
	list<LockEntry*>::type				_entries;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
