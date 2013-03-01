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
#include "nit/input/InputSource.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputDevice : public WeakSupported
{
public:
	InputDevice(const String& name);
	virtual ~InputDevice();

public:
	const String&						getName()								{ return _name; }
	InputUser*							getUser()								{ return _user; }

public:
	void								attach(InputSource* source);
	void								detach(InputSource* source);

public:
	void								find(const String& pattern, vector<InputSource*>::type& outResults);

public:
	bool								isLocked()								{ return _lockCount > 0; }
	void								lock(bool reset = true)					{ lock(1, reset); }
	void								unlock()								{ unlock(1); }

public:
	bool								hasChannel()							{ return _channel != NULL; }
	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

protected:
	virtual void						onUpdate(int frame);

protected:
	String								_name;
	Ref<EventChannel>					_channel;
	Weak<InputUser>						_user;

	typedef set<Ref<InputSource> >::type Sources;
	Sources								_sources;
	int									_lockCount;

private:
	friend class InputUser;				// called by InputUser
	void								update(int frame); 
	void								lock(int count, bool reset);
	void								unlock(int count);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
