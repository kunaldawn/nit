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

#include "nit/event/EventAutomata.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputCommand : public RefCounted
{
public:
	InputCommand(const String& command) : _command(command)					{ }
	InputCommand(const String& command, DataValue param) : _command(command), _param(param) { }

public:
	const String&						getCommand()							{ return _command; }
	DataValue							getParam()								{ return _param; }

protected:
	String								_command;
	DataValue							_param;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputCommandEvent : public InputEvent
{
public:
	InputCommandEvent()															{ }
	InputCommandEvent(InputUser* user, InputCommand* cmd);
	InputCommandEvent(InputDevice* dev, InputCommand* cmd);
	InputCommandEvent(InputSource* src, InputCommand* cmd);

public:
	virtual InputUser*					getUser() const							{ return _user; }
	virtual InputDevice*				getDevice()	const						{ return _device; }
	virtual InputSource*				getSource()	const						{ return _source; }
	virtual InputCommand*				getCommand() const						{ return _command; }

protected:
	Ref<InputUser>						_user;
	Weak<InputDevice>					_device;
	Ref<InputSource>					_source;
	Ref<InputCommand>					_command;
};

NIT_EVENT_DECLARE(NIT_API, OnInputCommand, InputCommandEvent);

////////////////////////////////////////////////////////////////////////////////

class NIT_API InputAutomata : public InputSource, public IEventSink
{
public:
	class NIT_API State : public WeakSupported
	{
	public:
		InputCommand*					getEnter()								{ return _enter; }
		InputCommand*					getExit()								{ return _exit; }

		State*							setEnter(InputCommand* action);
		State*							setExit(InputCommand* action);
		State*							dispatch(InputCommand* command, State* next);

	protected:
		friend class InputAutomata;
		typedef map<Ref<InputCommand>, State*>::type DispatchMap;
		DispatchMap						_dispatchMap;

		InputAutomata*					_automata;
		Ref<InputCommand>				_enter;
		Ref<InputCommand>				_exit;
	};

public:
	InputAutomata(const String& name);
	virtual ~InputAutomata();

	// TODO: Implement the IADL (input automata definition language)
	// TODO: Automata set + InputCommand save & load

public:
	State*								getInitial()							{ return _initial; }
	State*								getTerminal()							{ return _terminal; }
	State*								getCurrent()							{ return _current; }

	State*								newState(InputCommand* enterAction = NULL, InputCommand* exitAction = NULL);

	EventHandler*						commandHandler()						{ return _commandHandler; }

public:									// IEventSink Impl
	virtual bool						isEventActive()							{ return isEnabled(); }
	virtual bool						isDisposed()							{ return false; }

protected:								// InputSource Impl
	virtual void						onReset();
	virtual void						onUpdate(int frame)						{ /* do nothing */ }

private:
	vector<State*>::type				_states;

	Ref<EventHandler>					_commandHandler;

	State*								_initial;
	State*								_terminal;
	State*								_current;

	void								onInputCommand(const InputCommandEvent* evt);
	void								enter(State* state);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
