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

#include "nit/logic/Object.h"

#include "nit/logic/Component.h"
#include "nit/logic/Feature.h"
#include "nit/logic/World.h"
#include "nit/logic/Component.h"
#include "nit/logic/Transform.h"

#include "nit/script/ScriptRuntime.h"

#include "nit/app/Package.h"
#include "nit/app/Session.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

uint Object::s_LastRuntimeID = 1000;

Object::Object() 
{
	_runtimeID = s_LastRuntimeID++;
	_editCount = 0;
	_name = "Object";
	_transform = NULL;

	_status.set(GOS_ACTIVE, true);

	g_Session->addIsolated(this);
}

Object::~Object()
{
	if (!isDisposed())
	{
		LOG(0, "*** '%s': destroy before disposed\n", _name.c_str());
		_disposeByWorld();
	}
}

void Object::setName(const String& name)
{
	Ref<RefCounted> safe = this;

	bool ok = (_world == NULL) || _world->_renameObject(this, name);

	if (ok) _name = name;
}

void Object::setActive(bool flag)
{
	if (isActive() == flag) return;

	_status.set(GOS_ACTIVE, flag);

	_reactivateAll();
}

// Dispose() call by user
bool Object::dispose()
{
	if (isDisposed()) return true;

	if (isReactivating() || isEditing())
	{
		error("busy: cannot dispose");
		return false;
	}

	Ref<RefCounted> safe = this;

	bool ok = true;

	if (_world)
	{
		ok = _world->_detachObject(this);
	}

	if (!ok) return false;

	_deactivateAll();

	_world = NULL;

	_dispose();

	return true;
}

// World want this to be disposed
void Object::_disposeByWorld()
{
	// Simulate detached and disposed smoothly
	_deactivateAll();

	_world = NULL;

	_dispose();
}

void Object::_dispose()
{
	// Clear all status
	_status = GOS_DISPOSING;

	if (isReactivating() || isEditing())
	{
		// This case can never happen with user Dispose() call
		// Intended for rare case of world disposing
		info("busy: keep disposing");
	}

	// Uninstall all features
	Features::iterator fitr = _features.begin();
	while (fitr != _features.end())
	{
		Feature* feature = (*fitr++).second;
		feature->onUninstall(this);
		Package* fpack = feature->getPackage();
		if (fpack) fpack->decUseCount();
	}
	_features.clear();

	// Dispose all components
	Components::iterator citr = _components.begin();
	while (citr != _components.end())
	{
		(*citr++).second->_disposeByObject();
	}
	_components.clear();

	// Release all links
	_transform = NULL;

	// Call user dispose
	onDispose();

	_status = GOS_DISPOSED;
}

Component* Object::get(const String& name)
{
	std::pair<Components::iterator, Components::iterator> r = _components.equal_range(name);
	return (r.first != r.second) ? r.first->second : NULL;
}

void Object::find(const String& pattern, vector<Component*>::type& varResults)
{
	for (Components::iterator itr = _components.begin(), end = _components.end(); itr != end; ++itr)
	{
		if (Wildcard::match(pattern, itr->first))
			varResults.push_back(itr->second.get());
	}
}

Feature* Object::getFeature(const String& name)
{
	Features::iterator itr = _features.find(name);

	return (itr != _features.end()) ? itr->second : NULL;
}

bool Object::hasFeature(Feature* mod)
{
	Features::iterator itr = _features.find(mod->getName());

	return itr != _features.end() && itr->second == mod;
}

void Object::findFeature(const String& pattern, vector<Feature*>::type& varResults)
{
	for (Features::iterator itr = _features.begin(), end = _features.end(); itr != end; ++itr)
	{
		if (Wildcard::match(pattern, itr->first))
			varResults.push_back(itr->second.get());
	}
}

bool Object::install(Feature* feature)
{
	if (isDisposed()) return false;

	Features::iterator itr = _features.find(feature->getName());

	if (itr != _features.end()) 
	{
		if (itr->second == feature) return true;

		error(String("cannot install ") + feature->getName() + ": feature with same name already exists");
		return false;
	}

	if (isReactivating())
	{
		error(String("reactivating: cannot install ") + feature->getName());
		return false;
	}

	beginEdit();

	bool ok = feature->onInstall(this);

	if (ok) 
	{
		_features.insert(std::make_pair(feature->getName(), feature));
		if (feature->getPackage()) 
			feature->getPackage()->incUseCount();
	}
	else
	{
		error(String("cannot install ") + feature->getName() + ": install fail");
	}

	endEdit();

	return ok;
}

bool Object::uninstall(Feature* feature)
{
	if (isDisposed()) return false;

	if (_features.find(feature->getName()) == _features.end()) 
	{
		error(String("cannot uninstall ") + feature->getName() + ": not installed");
		return false;
	}

	if (isReactivating())
	{
		error(String("reactivating: cannot uninstall ") + feature->getName());
		return false;
	}

	// Call uninstall of the feature (components will remain untouched)

	beginEdit();

	bool ok = feature->onUninstall(this);

	if (ok)
	{
		// Dispose all components of the feature
		Components::iterator itr = _components.begin();

		while (itr != _components.end())
		{
			Component* comp = (*itr++).second;

			if (comp->getFeature() == feature)
			{
				comp->_feature = NULL;
				comp->dispose();
			}
		}

		_features.erase(feature->getName());
	}
	else
	{
		error(String("cannot uninstall ") + feature->getName() + ": uninstall fail");
	}

	if (feature->getPackage())
		feature->getPackage()->decUseCount();

	endEdit();

	return ok;
}

Component* Object::attach(Component* comp)
{
	return attach(comp->getName(), comp);
}

Component* Object::attach(const String& name, Component* comp)
{
	if (isDisposed()) return NULL;

	if (comp == NULL)
	{
		error(String("cannot attach ") + name + ": component is null");
		return NULL;
	}

	if (comp->isDisposed()) 
	{
		error(String("cannot attach ") + name + ": disposed");
		return NULL;
	}

	if (comp->_object == this) 
	{
		error(String("cannot attach ") + name + ": already attached as " + comp->getName());
		return NULL;
	}

	if (comp->_object)
	{
		error(String("cannot attach ") + name + ": already attached to " + comp->_object->getName());
		return NULL;
	}

	Ref<RefCounted> lock = comp;

	comp->_name = name;

	bool ok = _attachComponent(comp);

	return ok ? comp : NULL;
}

bool Object::_attachComponent(Component* comp)
{
	if (isDisposed()) return false;

	if (isReactivating())
	{
		error(String("reactivating : cannot attach ") + comp->getName());
		return false;
	}

	beginEdit();

	_components.insert(std::make_pair(comp->getName(), comp));
	comp->_object = this;

	endEdit();

	return true;
}

bool Object::_detachComponent(Component* comp)
{
	if (isDisposed()) return false;

	if (isReactivating())
	{
		error(String("reactivating: cannot detach ") + comp->getName());
		return false;
	}

	beginEdit();

	if (_transform == comp)
		_transform = NULL;

	std::pair<Components::iterator, Components::iterator> r = _components.equal_range(comp->getName());

	for (Components::iterator itr = r.first; itr != r.second; )
	{
		if (itr->second == comp)
		{
			// Deactivate only that component manually
			_status.set(GOS_REACTIVATING, true);
			comp->_deactivate();
			_components.erase(itr);
			_status.set(GOS_REACTIVATING, false);
			endEdit();
			return true;
		}
		else
		{
			++itr;
		}
	}

	cancelEdit();
	return false;
}

// NOTE: when renaming object, that object might be ignored by object iteration
// In actual game, we have little need to rename a component. 
// This feature is for the designer tool (which never change name during iteration)

bool Object::_renameComponent(Component* comp, const String& name)
{
	if (isDisposed()) return false;

	if (isReactivating())
	{
		error(String("reactivating: cannot rename ") + comp->getName() + " to " + name);
		return false;
	}

	beginEdit();

	std::pair<Components::iterator, Components::iterator> r = _components.equal_range(comp->getName());

	for (Components::iterator itr = r.first; itr != r.second; )
	{
		if (itr->second == comp)
		{
			// reattach - erase and insert
			_components.erase(itr);
			_components.insert(std::make_pair(name, comp));
			endEdit();
			return true;
		}
		else
		{
			++itr;
		}
	}

	cancelEdit();
	return false;
}

void Object::_reactivateAll()
{
	if (!isAttached() || !isActive()) return _deactivateAll();

	if (_editCount > 0) return;
	if (isDisposed()) return;

	_status.set(GOS_REACTIVATING, true);

	_status.set(GOS_ACTIVATED, isActive());

	Components::iterator itr = _components.begin();

	while (itr != _components.end())
	{
		(*itr++).second->_reactivate();
	}

	_status.set(GOS_REACTIVATING, false);
}

void Object::_deactivateAll()
{
	if (_editCount > 0) return;
	if (isDisposed()) return;

	_status.set(GOS_REACTIVATING, true);

	_status.set(GOS_ACTIVATED, false);

	Components::iterator itr = _components.begin();

	while (itr != _components.end())
	{
		(*itr++).second->_deactivate();
	}

	_status.set(GOS_REACTIVATING, false);
}

void Object::_quitEdit()
{
	if (_editCount > 0)
	{
		error(StringUtil::format("non-matched %d edit forced end", _editCount));
		_editCount = 1;
		endEdit();
	}
}

void Object::setTransform(Transform* transformNode)
{
	if (_transform == transformNode) return;

	beginEdit();

	_transform = transformNode;

	endEdit();
}

const Vector3 Object::getPosition()
{
	return _transform ? _transform->getPosition() : Vector3::ZERO;
}

const Quat Object::getRotation()
{
	return _transform ? _transform->getRotation() : Quat::IDENTITY;
}

void Object::beginTransform()
{
	if (_transform) _transform->beginTransform();
}

void Object::setPosition(const Vector3& pos)
{
	if (_transform) _transform->setPosition(pos);
}

void Object::setRotation(const Quat& rot)
{
	if (_transform) _transform->setRotation(rot);
}

void Object::endTransform()
{
	if (_transform) _transform->endTransform();
}

void Object::move(const Vector3& localDelta)
{
	if (_transform) _transform->move(localDelta);
}

void Object::rotate(const Quat& localDelta)
{
	if (_transform) _transform->rotate(localDelta);
}

void Object::info(const String& msg, const String& prefix)
{
#ifndef _SHIPPING
	String log = prefix + ": " + msg;

	LOG(0, "++ %s%s%s\n", getName().c_str(), (prefix.empty() ? "" : "."), log.c_str());
#endif
}

void Object::error(const String& msg, const String& prefix)
{
#ifndef _SHIPPING
	String log = prefix + ": " + msg;

	LOG(0, "*** %s%s%s\n", getName().c_str(), (prefix.empty() ? "" : "."), log.c_str());
#endif
}

////////////////////////////////////////////////////////////////////////////////

/*
- You can't do following during Activate / Deactivate:
  - object.AttachTo(world)
  - object.Detach()
  - object.Dispose()
  - object.Install()
  - object.Uninstall()
  - comp.AttachTo(object)
  - comp.Detach()
  - comp.Dispose()

- Usage of 'Edit':
  - BeginEdit() / EndEdit()
    - When adding many components to an object at the same time.
	- When spawn an object and then add many components at the same time.
	- When scene loads its objects (spawning accompanies)
	- When a tool adds/removes more than one components
  - Uninstall() -> Implicitly begin/ends edit mode.
  - AttachComponent / DetachComponent -> Implicitly begin/ends edit mode for a single component

- When object is not yet attached to world?
  - Every component remains deactivated and will be activated when the object attached to a world.

- When Activate -> Deactivate:
  - Component which is now deactivating can use its parent object and the world
  - But you can't find the object by world.Find() because the object already detached from the world

- When Deactivate -> Activate:
  - Component which is now activating can use its object and world (already attached)

- When Dispose:
  - Component can't use object / module / world
  - Object can't use world / component / module

- Do we have to activate / deactivate a component manually?
  - No, object will handle the job when it is attaching / detaching.
  - Of course, you can manually deactivate a component so keep them deactivated even if object tries to activate it.

- Do we have to activate / deactive an object manually?
  - Yes, although object is active by default, 
  - You should deactivate an object when it has to and later activate it again manually when needed.
*/

NS_NIT_END;
