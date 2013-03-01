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

#include "nit/input/InputDevice.h"

#include "nit/input/InputUser.h"
#include "nit/input/InputSource.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

InputDevice::InputDevice(const String& name)
: _name(name)
, _lockCount(0)
{

}

InputDevice::~InputDevice()
{
	if (_user)
		_user->release(this);
}

void InputDevice::update(int frame)
{
	onUpdate(frame);
}

void InputDevice::onUpdate(int frame)
{
	for (Sources::iterator itr = _sources.begin(), end = _sources.end(); itr != end; ++itr)
	{
		InputSource* source = *itr;
		source->update(frame);
	}
}

void InputDevice::lock(int count, bool reset)
{
	_lockCount += count;
	for (Sources::iterator itr = _sources.begin(), end = _sources.end(); itr != end; ++itr)
	{
		InputSource* source = *itr;
		source->lock(count, reset);
	}
}

void InputDevice::unlock(int count)
{
	_lockCount -= count;
	for (Sources::iterator itr = _sources.begin(), end = _sources.end(); itr != end; ++itr)
	{
		InputSource* source = *itr;
		source->unlock(count);
	}
}

void InputDevice::attach(InputSource* source)
{
	if (source == NULL || source->_device == this) return;

	source->_device = this;
	_sources.insert(source);
	source->lock(_lockCount, true);
}

void InputDevice::detach(InputSource* source)
{
	if (source == NULL || source->_device != this) return;

	source->_device = NULL;
	source->unlock(_lockCount);
	_sources.erase(source);
}

void InputDevice::find(const String& pattern, vector<InputSource*>::type& outResults)
{
	for (Sources::iterator itr = _sources.begin(), end = _sources.end(); itr != end; ++itr)
	{
		InputSource* source = *itr;
		if (Wildcard::match(pattern, source->getName()))
			outResults.push_back(source);
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
