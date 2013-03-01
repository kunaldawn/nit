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

#include "nit/input/InputUser.h"

#include "nit/input/InputDevice.h"
#include "nit/input/InputSource.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

InputUser::InputUser(const String& name, uint32 index)
: _name(name)
, _index(index)
, _lockCount(0)
{

}

void InputUser::setIdentity(DataObject* identity)
{
	if (_identity == identity) return;

	_identity = identity;

	if (_channel)
		_channel->send(Events::OnInputUserIdentityChanged, new InputUserEvent(this, NULL));
}

void InputUser::update(int frame)
{
	for (InputUser::Devices::iterator itr = _devices.begin(), end = _devices.end(); itr != end;)
	{
		InputDevice* device = *itr;
		if (device == NULL)
		{
			_devices.erase(itr++);
			continue;
		}

		device->update(frame);
		++itr;
	}
}

void InputUser::lock(bool reset, const String& pattern)
{
	++_lockCount;

	for (InputUser::Devices::iterator itr = _devices.begin(), end = _devices.end(); itr != end; ++itr)
	{
		InputDevice* device = *itr;
		if (device && Wildcard::match(pattern, device->getName()))
			device->lock(1, reset);
	}
}

void InputUser::unlock(const String& pattern)
{
	--_lockCount;

	for (InputUser::Devices::iterator itr = _devices.begin(), end = _devices.end(); itr != end; ++itr)
	{
		InputDevice* device = *itr;
		if (device && Wildcard::match(pattern, device->getName()))
			device->unlock(1);
	}
}

void InputUser::acquire(InputDevice* device)
{
	if (device == NULL || device->_user == this)
		return;

	if (device->_user)
		device->_user->release(device);

	device->_user = this;
	_devices.insert(device);

	svc_Input->post(Events::OnInputUserAcquire, new InputUserEvent(this, device));
	device->lock(_lockCount, true);
}

void InputUser::release(InputDevice* device)
{
	if (device == NULL || device->_user != this)
		return;

	device->_user = NULL;
	_devices.erase(device);

	svc_Input->post(Events::OnInputUserRelease, new InputUserEvent(this, device));
	device->unlock(_lockCount);
}

void InputUser::find(const String& pattern, vector<InputDevice*>::type& outResults)
{
	for (InputUser::Devices::iterator itr = _devices.begin(), end = _devices.end(); itr != end; ++itr)
	{
		InputDevice* device = *itr;
		if (device && Wildcard::match(pattern, device->getName()))
			outResults.push_back(device);
	}
}

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(OnInputUserIdentityChanged, InputUserEvent);
NIT_EVENT_DEFINE(OnInputUserAcquire,	InputUserEvent);
NIT_EVENT_DEFINE(OnInputUserRelease,	InputUserEvent);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
