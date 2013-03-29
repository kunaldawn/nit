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

#include "nit/input/InputSource.h"

#include "nit/input/InputUser.h"
#include "nit/input/InputDevice.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

InputSource::InputSource(const String& name)
: _name(name)
, _lockCount(0)
, _enabled(true)
, _lockable(true)
, _frame(-1)
{
}

void InputSource::setEnabled(bool flag)
{
	if (_enabled == flag) return;

	_enabled = flag;

	if (_enabled)
		reset();
}

void InputSource::update(int frame)
{
	if (frame == _frame)
		return;

	_frame = frame;

	if (_enabled && !isLocked())
		onUpdate(frame);
}

void InputSource::setLockable(bool flag)
{
	if (_lockable == flag) return;

	_lockable = flag;

	if (isLocked())
		reset();
}

void InputSource::lock(int count, bool reset)
{
	if (reset && _lockable && _lockCount <= 0 && (_lockCount + count) > 0)
		this->reset();

	_lockCount += count;
}

void InputSource::unlock(int count)
{
	_lockCount -= count;
}

void InputSource::reset()
{
	_frame = -1;
	onReset();
}

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(POINTER_MOVE,			InputPointerEvent);
NIT_EVENT_DEFINE(POINTER_ON,			InputPointerEvent);
NIT_EVENT_DEFINE(POINTER_DRAG,			InputPointerEvent);
NIT_EVENT_DEFINE(POINTER_OFF,			InputPointerEvent);
NIT_EVENT_DEFINE(POINTER_CANCEL,		InputPointerEvent);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
