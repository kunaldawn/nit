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

#include "nit/input/InputService.h"

#include "nit/input/InputUser.h"
#include "nit/input/InputDevice.h"
#include "nit/input/InputSource.h"
#include "nit/input/InputCommand.h"

#include "nit/app/AppBase.h"
#include "nit/app/Package.h"

#include "nit/data/DataSchema.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

InputService::InputService()
: Service("InputService", NULL, SVC_INPUT)
{
	_systemUser	= NULL;
	_defaultUser	= NULL;
}

InputService::~InputService()
{

}

void InputService::onRegister()
{

}

void InputService::onUnregister()
{

}

void InputService::onInit()
{
	float timerPriority = DataValue(g_App->getConfig("input_priority", "-100"));

	_timer = new TickTimer();
	g_App->getClock()->channel()->priority(timerPriority)->bind(EVT::CLOCK, _timer->sourceTimeHandler());

	_timer->channel()->bind(EVT::TICK, this, &InputService::onTick);

	_systemUser = new InputUser("$system", 0);
	_defaultUser = new InputUser("$default", 1);

	_frame = 0;
}

void InputService::onFinish()
{
	_eventQueue.clear();

	_systemUser	= NULL;
	_defaultUser	= NULL;
	_timer			= NULL;
}

void InputService::setDefaultUser(InputUser* user)
{
	if (_defaultUser != user)
		return;

	LOG(0, "++ Default User changed '%s' to '%s'\n", 
		_defaultUser ? _defaultUser->getName().c_str() : "(null)", 
		user ? user->getName().c_str() : "(null)");

	_defaultUser = user;
}

void InputService::post(EventId id, InputEvent* evt, EventChannel* channel)
{
	_eventQueue.push_back(EventEntry());

	EventEntry& e = _eventQueue.back();
	e.eventID = id;
	e.event = evt;
	e.channel = channel;
}

void InputService::post(InputCommandEvent* command)
{
	post(EVT::INPUT_COMMAND, command, _commandChannel);
}

void InputService::onTick(const TimeEvent* evt)
{
	// Send incoming (from outside tick loop) events of previous frame (ex. system event)
	processQueue(_eventQueue);

	++_frame;
	if (_frame < 0) _frame = 0;

	// Update devices of all users
	for (UserNameLookup::iterator itr = _users.begin(), end = _users.end(); itr != end; ++itr)
	{
		InputUser* user = itr->second;
		user->update(_frame);
	}

	// Send events which are queued during updating devices
	processQueue(_eventQueue);
}

void InputService::processQueue(EventQueue& queue)
{
	// Send events which are piled on queue so far
	size_t numEvents = queue.size(); // TODO: Do we have to limit by numEvents?

	while (!queue.empty())
	{
		EventEntry& e = queue.front();

		EventId id = e.eventID;
		Ref<InputEvent> evt = e.event;
		queue.pop_front();

		if (evt && evt->isConsumed())
			continue;

		EventChannel* channel = e.channel;
		if (channel)
		{
			// If channel specified on post : Send to that channel
			channel->send(id, evt);
			continue;
		}

		// When channel not specified: broadcast upward from source channel

		if (evt == NULL)
			continue; // Can't handle event with no channel specified.

		// 1. Broadcast to source channel
		Ref<InputSource> source = evt->getSource();
		if (source && source->hasChannel())
		{
			source->channel()->send(id, evt);
			if (evt->isConsumed() || !evt->isUplinking())
				continue;
		}

		// 2. Broadcast to device channel
		Weak<InputDevice> device = evt->getDevice();
		if (device && device->hasChannel())
		{
			device->channel()->send(id, evt);
			if (evt->isConsumed() || !evt->isUplinking())
				continue;
		}

		// 3. Broadcast to user channel
		Ref<InputUser> user = evt->getUser();

		if (user && user->hasChannel())
		{
			user->channel()->send(id, evt);
			if (evt->isConsumed() || !evt->isUplinking())
				continue;
		}

		// TODO: 4. Broadcast to service channel?
	}
}

void InputService::cancel(EventId id, InputEvent* evt, InputUser* user, InputDevice* device, InputSource* source, InputCommand* command)
{
	for (EventQueue::iterator itr = _eventQueue.begin(); itr != _eventQueue.end(); )
	{
		EventEntry& entry = *itr;
		if (id && entry.eventID != id) ++itr;
		else
		{
			InputEvent* e = entry.event;
			if (evt && e != evt) ++itr;
			else if (user && !e->isRelated(user)) ++itr;
			else if (device && !e->isRelated(device)) ++itr;
			else if (source && !e->isRelated(source)) ++itr;
			else if (command && !e->isRelated(command)) ++itr;
			else
				_eventQueue.erase(itr++);
		}
	}
}

InputUser* InputService::newUser(const String& name)
{
	InputUser* user = new InputUser(name, 0);
	return user;
}

////////////////////////////////////////////////////////////////////////////////

InputUser* InputEvent::getUser() const
{
	InputDevice* device = getDevice();
	return device ? device->getUser() : NULL;
}

InputDevice* InputEvent::getDevice() const
{
	InputSource* source = getSource();
	return source ? source->getDevice() : NULL;
}

bool InputEvent::isRelated(InputUser* user) const
{
	return (getUser() == user) || (_related && _related->isRelated(user));
}

bool InputEvent::isRelated(InputDevice* device) const
{
	return (getDevice() == device) || (_related && _related->isRelated(device));
}

bool InputEvent::isRelated(InputSource* source) const
{
	return (getSource() == source) || (_related && _related->isRelated(source));
}

bool InputEvent::isRelated(InputCommand* command) const
{
	return _related && _related->isRelated(command);
}

////////////////////////////////////////////////////////////////////////////////

class ScopedInputLock::LockEntry : public PooledAlloc
{
public:
	virtual ~LockEntry()				{ }
	virtual void						revert() = 0;

	class UserLock;
	class DeviceLock;
	class SourceLock;
	class UserUnlock;
	class DeviceUnlock;
	class SourceUnlock;
};

class ScopedInputLock::LockEntry::UserLock : public ScopedInputLock::LockEntry
{
public:
	UserLock(InputUser* user, const String& pattern) : _target(user), _pattern(pattern) { }
	virtual void						revert()								{ if (_target) _target->unlock(_pattern); }
	Weak<InputUser>						_target;
	String								_pattern;
};

class ScopedInputLock::LockEntry::DeviceLock : public ScopedInputLock::LockEntry
{
public:
	DeviceLock(InputDevice* device) : _target(device) { }
	virtual void						revert()								{ if (_target) _target->unlock(); }
	Weak<InputDevice>					_target;
};

class ScopedInputLock::LockEntry::SourceLock : public ScopedInputLock::LockEntry
{
public:
	SourceLock(InputSource* source) : _target(source) { }
	virtual void						revert()								{ if (_target) _target->unlock(); }
	Weak<InputSource>					_target;
};

class ScopedInputLock::LockEntry::UserUnlock : public ScopedInputLock::LockEntry
{
public:
	UserUnlock(InputUser* user, const String& pattern) : _target(user), _pattern(pattern) { }
	virtual void						revert()								{ if (_target) _target->lock(false, _pattern); }
	Weak<InputUser>						_target;
	String								_pattern;
};

class ScopedInputLock::LockEntry::DeviceUnlock : public ScopedInputLock::LockEntry
{
public:
	DeviceUnlock(InputDevice* device) : _target(device) { }
	virtual void						revert()								{ if (_target) _target->lock(false); }
	Weak<InputDevice>					_target;
};

class ScopedInputLock::LockEntry::SourceUnlock : public ScopedInputLock::LockEntry
{
public:
	SourceUnlock(InputSource* source) : _target(source) { }
	virtual void						revert()								{ if (_target) _target->lock(false); }
	Weak<InputSource>					_target;
};

////////////////////////////////////////////////////////////////////////////////

ScopedInputLock::ScopedInputLock()
{

}

ScopedInputLock::~ScopedInputLock()
{
	leave();
}

void ScopedInputLock::lock(InputUser* user, bool reset, const String& pattern)
{
	user->lock(reset, pattern);
	_entries.push_back(new LockEntry::UserLock(user, pattern));
}

void ScopedInputLock::lock(InputDevice* device, bool reset)
{
	device->lock(reset);
	_entries.push_back(new LockEntry::DeviceLock(device));
}

void ScopedInputLock::lock(InputSource* source, bool reset)
{
	if (reset) source->reset();
	source->lock();
	_entries.push_back(new LockEntry::SourceLock(source));
}

void ScopedInputLock::unlock(InputUser* user, const String& pattern)
{
	user->unlock(pattern);
	_entries.push_back(new LockEntry::UserUnlock(user, pattern));
}

void ScopedInputLock::unlock(InputDevice* device)
{
	device->unlock();
	_entries.push_back(new LockEntry::DeviceUnlock(device));
}

void ScopedInputLock::unlock(InputSource* source)
{
	source->unlock();
	_entries.push_back(new LockEntry::SourceUnlock(source));
}

void ScopedInputLock::leave(bool unlock)
{
	for (list<LockEntry*>::type::iterator itr = _entries.begin(), end = _entries.end(); itr != end; ++itr)
	{
		if (unlock) (*itr)->revert();
		delete *itr;
	}
	_entries.clear();
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
