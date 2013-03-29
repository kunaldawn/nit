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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputSource : public RefCounted
{
public:
	InputSource(const String& name);

public:
	const String&						getName()								{ return _name; }
	InputDevice*						getDevice()								{ return _device; }

public:
	bool								isEnabled()								{ return _enabled; }
	void								setEnabled(bool flag);

public:
	bool								isLockable()							{ return _lockable; }
	void								setLockable(bool flag);

	bool								isLocked()								{ return _lockable && _lockCount > 0; }
	void								lock(bool reset = true)					{ lock(1, reset); }
	void								unlock()								{ unlock(1); }

public:
	void								reset();
	void								update(int frame);	// called by InputDevice or other InputSource

public:
	bool								hasChannel()							{ return _channel != NULL; }
	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

protected:
	virtual void						onReset() = 0;
	virtual void						onUpdate(int frame) = 0;

protected:
	String								_name;
	Ref<EventChannel>					_channel;
	Weak<InputDevice>					_device;
	int									_lockCount;
	int									_frame;

	bool								_enabled : 1;
	bool								_lockable : 1;

private:
	friend class						InputDevice;
	void								lock(int count, bool reset);
	void								unlock(int count);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputSourceEvent : public InputEvent
{
public:
	InputSourceEvent()															{ }
	InputSourceEvent(InputSource* source) : _source(source)					{ }

public:									// InputEvent Impl
	virtual InputSource*				getSource()	const						{ return _source; }

protected:
	Ref<InputSource>					_source;
};

NIT_EVENT_DECLARE(NIT_API, INPUT_LOCKED,	InputSourceEvent);
NIT_EVENT_DECLARE(NIT_API, INPUT_UNLOCKED,	InputSourceEvent);

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputTrigger : public InputSource
{
public:
	InputTrigger(const String& name) : InputSource(name)						{ }

public:
	bool								isPushed()								{ return _value > 0.0f; }
	bool								isRepeatable()							{ return _repeatable; }

	// TODO: IMPL THIS

protected:
	float								_value;
	bool								_repeatable;
};

NIT_EVENT_DECLARE(NIT_API, TRIGGER_DOWN,	InputSourceEvent);
NIT_EVENT_DECLARE(NIT_API, TRIGGER_UP,		InputSourceEvent);
NIT_EVENT_DECLARE(NIT_API, TRIGGER_CANCEL,	InputSourceEvent);
NIT_EVENT_DECLARE(NIT_API, TRIGGER_REPEAT,	InputSourceEvent);

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputKeypad : public InputSource
{
	// TODO: IMPL THIS
};

NIT_EVENT_DECLARE(NIT_API, KEYPAD_DOWN,	InputSourceEvent);
NIT_EVENT_DECLARE(NIT_API, KEYPAD_UP,		InputSourceEvent);
NIT_EVENT_DECLARE(NIT_API, KEYPAD_CANCEL,	InputSourceEvent);
NIT_EVENT_DECLARE(NIT_API, KEYPAD_REPEAT,	InputSourceEvent);

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputSlider : public InputSource
{
public:
	InputSlider(const String& name) : InputSource(name)							{ }

public:
	float								getValue()								{ return _value; }
	bool								isAwake()								{ return !(_sleepMin < _value && _value < _sleepMax); }

	// TODO: IMPL THIS

protected:
	float								_value;

	float								_sleepMin;
	float								_sleepMax;
};

NIT_EVENT_DECLARE(NIT_API, SLIDER_MOVE,		InputSourceEvent);
NIT_EVENT_DECLARE(NIT_API, SLIDER_SLEEP,		InputSourceEvent);

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputStick : public InputSource
{
public:
	InputStick(const String& name) : InputSource(name)							{ }

public:
	const Vector3&						getValue()								{ return _value; }
	bool								isAwake()								{ return _sleepVector.absDotProduct(_value) < _sleepThreshold; }

	// TODO: IMPL THIS

public:
	class NIT_API StickTrigger : public InputTrigger
	{
		// TODO: IMPL THIS
	};

	StickTrigger*						createTrigger(const Vector3& target, float threshold);

protected:
	Vector3								_value;

	Vector3								_sleepVector;
	float								_sleepThreshold;
};

NIT_EVENT_DECLARE(NIT_API, STICK_MOVE,		InputSourceEvent);
NIT_EVENT_DECLARE(NIT_API, STICK_SLEEP,	InputSourceEvent);

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputPointer : public InputSource
{
public:
	InputPointer(const String& name) : InputSource(name)						{ _on = false; _value = Vector3::ZERO; _transform = Matrix4::IDENTITY; }

public:
	bool								isOn()									{ return _on; }
	const Vector3&						getValue()								{ return _value; }

protected:
	Vector3								_value;
	bool								_on;
	Matrix4								_transform;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputPointerEvent : public InputSourceEvent
{
public:
	InputPointerEvent()															{ }
	InputPointerEvent(InputPointer* source, const Vector3& prevPos, const Vector3& pos) : InputSourceEvent(source), _prevPos(prevPos), _pos(pos) { }

	InputPointer*						getSource() const						{ return static_cast<InputPointer*>(_source.get()); }

	const Vector3&						getPrevPos() const						{ return _prevPos; }
	const Vector3&						getPos() const							{ return _pos; }

protected:
	Vector3								_prevPos;
	Vector3								_pos;
};

NIT_EVENT_DECLARE(NIT_API, POINTER_MOVE,	InputPointerEvent);
NIT_EVENT_DECLARE(NIT_API, POINTER_ON,		InputPointerEvent);
NIT_EVENT_DECLARE(NIT_API, POINTER_DRAG,	InputPointerEvent);
NIT_EVENT_DECLARE(NIT_API, POINTER_OFF,	InputPointerEvent);
NIT_EVENT_DECLARE(NIT_API, POINTER_CANCEL,	InputPointerEvent);

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputAccel : public InputSource
{
	// TODO: IMPL THIS
};

NIT_EVENT_DECLARE(NIT_API, ACCEL_MOVE,		InputSourceEvent);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
