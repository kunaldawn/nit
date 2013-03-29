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

#include "nit/event/EventAutomata.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

EventAutomata::EventAutomata() : _entering(false), _eventActive(true)
{
}

EventAutomata::~EventAutomata()
{
}

void EventAutomata::setState(EventState* state)
{
	if (_entering)
	{
		LOG(0, "*** Reentry to automata %08x (%s -> %s) ignored!\n", this, _state ? _state->getName().c_str() : "NULL", state ? state->getName().c_str() : "NULL");
		return;
	}

	// Transition to same state is ignored.
	// NOTE: If you really want transition to same state just to invoke 'OnEnter' and 'OnExit' of its own, do following: 
	//       'automata->SetState(NULL); automata->SetState(state);'
	if (_state == state) return;

	// Save reference to curr and new until function exit 
	// (make ref count increased so that they can't be deleted during transition)
	Ref<EventState> currSave = _state;
	Ref<EventState> newSave = state;

	if (_state != NULL)
	{
		_entering = true; // Guard condition
		if (_eventActive) _state->onExit();
		_state->_automata = NULL;
		_entering = false;
	}

	_state = state;

	if (_state != NULL)
	{
		Ref<EventAutomata> other = _state->_automata.get();
		if (other) 
		{
			LOG(0, "*** automata %08x using state %s which belongs to other automata %08x!\n", this, _state->getName().c_str(), other.get());
			other->setState(NULL);
		}

		_state->_automata = this;
		if (_eventActive) _state->onEnter();
	}
}

bool EventAutomata::sendLocal(const Event* e)
{
	if (_state && _state->isEventActive() && !e->isConsumed())
		_state->onEvent(e);
	return true;
}

////////////////////////////////////////////////////////////////////////////////

EventState::EventState()
:	_automata(NULL) 
{
}

EventState::~EventState()
{
}

const String& EventState::getName()
{
	if (_name.empty())
		_name = getClassName();

	return _name;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;