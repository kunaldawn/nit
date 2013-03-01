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

#include "nit/logic/Component.h"

#include "nit/logic/World.h"
#include "nit/logic/Object.h"
#include "nit/logic/Feature.h"

#include "nit/data/DataSchema.h"

#include "nit/app/Package.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Component::Component()
{
	_name = "Component";
	_status.set(GCS_ACTIVE, true);

	_feature = NULL;
}

Component::~Component()
{
	if (!isDisposed())
	{
		LOG(0, "*** '%s': destroy before disposed\n", _name.c_str());
		_disposeByObject();
	}
}

World* Component::getWorld()
{
	return _object ? _object->getWorld() : NULL;
}

void Component::setName(const String& name)
{
	if (_feature)
	{
		error(String("cannot rename: marked by ") + _feature->getName());
		return;
	}

	Ref<RefCounted> lock = this;

	bool ok = (_object == NULL) || _object->_renameComponent(this, name);

	if (ok) _name = name;
}

void Component::setActive(bool flag)
{
	if (isActive() == flag) return;

	_status.set(GCS_ACTIVE, flag);

	_reactivate();
}

// Dispose() call by user
bool Component::dispose()
{
	if (isDisposed()) return true;

	if (_feature)
	{
		error(String("Cannot dispose: marked by ") + _feature->getName());
		return false;
	}

	Ref<RefCounted> safe = this;

	bool ok = _object == NULL || _object->_detachComponent(this);

	if (!ok) return false;

	_feature = NULL;
	_object = NULL;

	_dispose();

	return true;
}

// Object want this to be disposed
void Component::_disposeByObject()
{
	// Simulate detached and disposed smoothly
	_deactivate();

	_feature = NULL;
	_object = NULL;

	_dispose();
}

void Component::_dispose()
{
	// Clear all status
	_status = GCS_DISPOSING;

	onDispose();

	_status = GCS_DISPOSED;
}

bool Component::checkValid()
{
	if (isDisposed()) return false;

	return onValidate();
}

void Component::_reactivate()
{
	if (!isAttached() || !isActive() || !checkValid()) return _deactivate();

	if (isActivated()) return;

	_status.set(GCS_REACTIVATING, true);

	_status.set(GCS_ACTIVATED, true);

	onActivate();

	_status.set(GCS_REACTIVATING, false);
}

void Component::_deactivate()
{
	if (!isActivated()) return;

	_status.set(GCS_REACTIVATING, true);

	_status.set(GCS_ACTIVATED, false);

	onDeactivate();

	_status.set(GCS_REACTIVATING, false);
}

void Component::info(const String& msg, const String& prefix)
{
	if (_object == NULL) return;

	_object->info(msg, getName() + (prefix.empty() ? "" : ".") + prefix);
}

void Component::error(const String& msg, const String& prefix)
{
	if (_object == NULL) return;

	_object->error(msg, getName() + (prefix.empty() ? "" : ".") + prefix);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
