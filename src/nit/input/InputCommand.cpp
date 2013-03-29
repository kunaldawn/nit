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

#include "nit/input/InputCommand.h"

#include "nit/input/InputUser.h"
#include "nit/input/InputDevice.h"
#include "nit/input/InputSource.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(INPUT_COMMAND, InputCommandEvent);

InputCommandEvent::InputCommandEvent(InputUser* user, InputCommand* cmd) 
: _user(user), _command(cmd)
{

}

InputCommandEvent::InputCommandEvent(InputDevice* dev, InputCommand* cmd) 
: _user(dev->getUser()), _device(dev), _command(cmd)
{

}

InputCommandEvent::InputCommandEvent(InputSource* src, InputCommand* cmd) 
: _user(src->getDevice()->getUser()), _device(src->getDevice()), _source(src), _command(cmd)
{

}

////////////////////////////////////////////////////////////////////////////////

InputAutomata::State* InputAutomata::State::setEnter(InputCommand* action)
{
	_enter = action;
	return this;
}

InputAutomata::State* InputAutomata::State::setExit(InputCommand* action)
{
	_exit = action;
	return this;
}

InputAutomata::State* InputAutomata::State::dispatch(InputCommand* command, State* next)
{
	ASSERT_THROW(next == NULL || next->_automata == this->_automata, EX_INVALID_PARAMS);
	_dispatchMap.insert(std::make_pair(command, next));
	return this;
}

////////////////////////////////////////////////////////////////////////////////

InputAutomata::InputAutomata(const String& name)
: InputSource(name)
{
	_initial = newState();
	_terminal = newState();
	_current = NULL;

	_commandHandler = createEventHandler(this, &InputAutomata::onInputCommand);

	reset();
}

InputAutomata::~InputAutomata()
{
	for (vector<State*>::type::iterator itr = _states.begin(), end = _states.end(); itr != end; ++itr)
	{
		delete *itr;
	}
}

void InputAutomata::onReset()
{
	enter(_current);
}

InputAutomata::State* InputAutomata::newState(InputCommand* enterAction, InputCommand* exitAction)
{
	State* state = new State();

	if (enterAction) state->setEnter(enterAction);
	if (exitAction) state->setExit(exitAction);

	_states.push_back(state);

	return state;
}

void InputAutomata::enter(State* state)
{
	State* old = _current;

	InputService* input = svc_Input;

	if (old && old->_exit)
	{
		input->post(new InputCommandEvent(this, old->_exit));
	}

	_current = state;

	if (state && state->_enter)
	{
		input->post(new InputCommandEvent(this, state->_enter));
	}
}

void InputAutomata::onInputCommand(const InputCommandEvent* evt)
{
	State::DispatchMap::iterator itr = _current->_dispatchMap.find(evt->getCommand());

	if (itr == _current->_dispatchMap.end())
		return;

	State* next = itr->second;

	if (next == NULL)
		next = _terminal;

	enter(next);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
