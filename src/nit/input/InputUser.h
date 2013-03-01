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

#include "nit/input/InputService.h"
#include "nit/input/InputDevice.h"

#include "nit/data/DataValue.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputUser : public RefCounted
{
public:
	const String&						getName()								{ return _name; }
	uint32								getIndex()								{ return _index; }

public:
	DataObject*							getIdentity()							{ return _identity; }
	void								setIdentity(DataObject* identity);

public:
	void								find(const String& pattern, vector<InputDevice*>::type& outResults);

	void								acquire(InputDevice* device);
	void								release(InputDevice* device);

	bool								isLocked()								{ return _lockCount > 0; }
	void								lock(bool reset = true, const String& pattern = "*");
	void								unlock(const String& pattern = "*");

public:
	bool								hasChannel()							{ return _channel != NULL; }
	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

protected:
	String								_name;
	uint32								_index;
	Weak<DataObject>					_identity;

	Ref<EventChannel>					_channel;
	int									_lockCount;

	typedef set<Weak<InputDevice> >::type Devices;
	Devices								_devices;

private:
	friend class InputService;
	InputUser(const String& name, uint32 index);
	void								update(int frame);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputUserEvent : public InputEvent
{
public:
	InputUserEvent()															{ }
	InputUserEvent(InputUser* user, InputDevice* device) : _user(user), _device(device) { }

public:									// InputEvent Impl
	virtual InputUser*					getUser() const							{ return _user; }
	virtual InputDevice*				getDevice() const						{ return _device; }

protected:
	Ref<InputUser>						_user;
	Weak<InputDevice>					_device;
};

NIT_EVENT_DECLARE(NIT_API, OnInputUserIdentityChanged, InputUserEvent);
NIT_EVENT_DECLARE(NIT_API, OnInputUserAcquire, InputUserEvent);
NIT_EVENT_DECLARE(NIT_API, OnInputUserRelease, InputUserEvent);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
