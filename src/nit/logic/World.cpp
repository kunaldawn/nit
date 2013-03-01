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

#include "nit/logic/World.h"
#include "nit/logic/Object.h"
#include "nit/logic/Component.h"
#include "nit/logic/Feature.h"

#include "nit/app/Package.h"
#include "nit/app/Session.h"
#include "nit/app/AppBase.h"
#include "nit/app/Plugin.h"

#include "nit/script/ScriptRuntime.h"

#include "nit/event/Timer.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

World::World()
{
	_name = "World";
}

World::~World()
{

	if (!isDisposed())
	{
		dispose();
	}
}

bool World::dispose()
{
	if (isDisposed()) return true;

	_status = GWS_DISPOSING;

	for (ObjectSet::iterator itr = _attached.begin(), end = _attached.end(); itr != end; ++itr)
	{
		Object* obj = *itr;
		obj->_disposeByWorld();
	}

	_attached.clear();

	_status = GWS_DISPOSED;

	return true;
}

Object* World::attach(Object* obj)
{
	return attach(obj->getName(), obj);
}

Object* World::attach(const String& name, Object* obj)
{
	if (isDisposed()) return NULL;

	if (obj == NULL)
	{
		LOG(0, "*** Cannot attach '%s' : object is null\n", name.c_str());
		return NULL;
	}

	if (obj->isReactivating() || obj->isEditing())
	{
		LOG(0, "*** Cannot attach '%s' : object is now busy\n", name.c_str());
		return NULL;
	}

	if (obj->_world == this)
		return obj;

	if (obj->_world)
	{
		LOG(0, "*** Cannot attach '%s' : already attached to '%s'\n", name.c_str(), obj->_world->getName().c_str());
		return NULL;
	}

	Ref<RefCounted> safe = obj;

	obj->_name = name;

	bool ok = _attachObject(obj);

	if (!ok) return NULL;

	obj->_world = this;
	g_Session->removeIsolated(obj);

	obj->_reactivateAll();

	obj->onAttach();

	return obj;
}

bool World::_attachObject(Object* object)
{
	if (isDisposed()) return false;

	_attached.insert(object);

	return true;
}

bool World::_detachObject(Object* object)
{
	if (isDisposed()) return false;

	ObjectSet::iterator itr = _attached.find(object);

	_attached.erase(itr);

	return true;
}

bool World::_renameObject(Object* object, const String& name)
{
	if (isDisposed()) return false;

	// Nothing to do yet

	return true;
}

void World::onTick(const TimeEvent* evt)
{
	// TODO: integrate with physics
}

void World::setName(const String& name)
{
	_name = name;
}

void World::all(ObjectResultSet& outResults)
{
	ObjectSet::iterator iter = _attached.begin();
	while (iter != _attached.end())
	{
		outResults.push_back(*iter++);
	}
}

void World::find(const char* wildcard, ObjectResultSet& outResults, bool activeOnly, bool ignoreCase)
{
	for (ObjectSet::iterator itr = _attached.begin(), end = _attached.end(); itr != end; ++itr)
	{
		Object* o = *itr;

		if (activeOnly && !o->isActivated())
			continue;

		if (Wildcard::match(wildcard, o->getName(), ignoreCase))
			outResults.push_back(o);
	}
}

void World::findComponents(const char* wildcard, ComponentResultSet& outResults, bool activeOnly, bool ignoreCase)
{
	for (ObjectSet::iterator itr = _attached.begin(), end = _attached.end(); itr != end; ++itr)
	{
		Object* o = *itr;

		if (activeOnly && !o->isActivated())
			continue;

		for (Object::Components::iterator itr = o->_components.begin(), end = o->_components.end(); itr != end; ++itr)
		{
			Component* c = itr->second;

			if (activeOnly && !c->isActivated())
				continue;

			if (Wildcard::match(wildcard, c->getName(), ignoreCase))
				outResults.push_back(c);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
