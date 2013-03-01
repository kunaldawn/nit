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

#include "nit/logic/Transform.h"
#include "nit/logic/Object.h"
#include "nit/logic/Feature.h"
#include "nit/logic/World.h"

#include "nit/data/DataSchema.h"

#include "nit/app/Package.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Transform::Transform() 
: _transformCount(0)
{
	_name = "Transform";
	_position = Vector3::ZERO;
	_rotation = Quat::IDENTITY;
	_offset = NULL;
}

void Transform::move(const Vector3& delta)
{
	setPosition(getPosition() + delta);
}

void Transform::rotate(const Quat& delta)
{
	setRotation(getRotation() * delta);
}

Vector3 Transform::getFront()
{
	Matrix3 rot;
	getRotation().ToRotationMatrix(rot);
	return -rot.GetColumn(1);
}

Vector3 Transform::getRight()
{
	Matrix3 rot;
	getRotation().ToRotationMatrix(rot);
	return -rot.GetColumn(0);
}

Vector3 Transform::getUp()
{
	Matrix3 rot;
	getRotation().ToRotationMatrix(rot);
	return rot.GetColumn(2);
}

void Transform::alignUpFront(const Vector3& up, const Vector3& front)
{
	Quat q; q.FromRotationMatrix(Math::upFront(up, front));
	setRotation(q);
}

void Transform::alignUpRight(const Vector3& up, const Vector3& right)
{
	Quat q; q.FromRotationMatrix(Math::upRight(up, right));
	setRotation(q);
}

void Transform::alignFrontUp(const Vector3& front, const Vector3& up)
{
	Quat q; q.FromRotationMatrix(Math::frontUp(front, up));
	setRotation(q);
}

void Transform::alignFrontRight(const Vector3& front, const Vector3& right)
{
	Quat q; q.FromRotationMatrix(Math::frontRight(front, right));
	setRotation(q);
}

void Transform::alignRightUp(const Vector3& right, const Vector3& up)
{
	Quat q; q.FromRotationMatrix(Math::rightUp(right, up));
	setRotation(q);
}

void Transform::alignRightFront(const Vector3& right, const Vector3& front)
{
	Quat q; q.FromRotationMatrix(Math::rightFront(right, front));
	setRotation(q);
}

void Transform::onActivate()
{
	Component::onActivate();
	
	// Calls Begin ~ EndTransform without _Synchronize() to initialize subclass components' peer

	if (_transformCount++ == 0)
	{
		onBeginTransform();
	}
	
	if (--_transformCount == 0)
	{
		onEndTransform();
		++_updateStamp;
	}
}

void Transform::clearOffset()
{
	beginTransform();

	safeDelete(_offset);

	endTransform();
}

void Transform::setPosOffset(const Vector3& pos)
{
	beginTransform();

	if (_offset == NULL)
	{
		_offset = new OffsetTransform();
		_offset->rotation = Quat::IDENTITY;
		_offset->invRotation = Quat::IDENTITY;
	}

	_offset->position = pos;

	endTransform();
}

void Transform::setRotOffset(const Quat& rot)
{
	beginTransform();

	if (_offset == NULL)
	{
		_offset = new OffsetTransform();
		_offset->position = Vector3::ZERO;
	}

	_offset->rotation = rot;
	_offset->invRotation = rot.Inverse();

	endTransform();
}

void Transform::_synchronize()
{
	if (_transformCount > 0) return;
	if (!isActivated()) return;
	if (_syncGroup == NULL) return;
	if (_syncGroup->_synchronizing) return;

	_syncGroup->syncFrom(this);
}

void Transform::onSyncFrom(SyncGroup* group, Transform* from)
{
	_position = from->_position;
	_rotation = from->_rotation;
}

void Transform::onSyncGroupIsolated(SyncGroup* group)
{
	if (group == _syncGroup)
		_syncGroup = NULL;
}

void Transform::SyncGroup::syncFrom(Transform* from)
{
	if (_synchronizing) return;
	if (!from->isActivated()) return;

	_synchronizing = true;

	for (SyncSet::iterator itr = _syncSet.begin(), end = _syncSet.end(); itr != end; ++itr)
	{
		Transform* t = *itr;
		if (t == from) continue;
		if (!t->isActivated()) continue;

		t->beginTransform();
		t->onSyncFrom(this, from);
		t->endTransform();
	}

	_synchronizing = false;
}

void Transform::SyncGroup::remove(Transform* member)
{
	_syncSet.erase(member);

	if (_syncSet.size() == 1)
	{
		this->incRefCount();

		Ref<Transform> last = *_syncSet.begin();
		last->onSyncGroupIsolated(this);

		this->decRefCount();
	}
}

void Transform::applyOffset(Vector3& pos, Quat& rot)
{
	if (_offset == NULL) return;

	pos += rot * _offset->position;
	rot = rot * _offset->rotation;
}

void Transform::applyInvOffset(Vector3& pos, Quat& rot)
{
	if (_offset == NULL) return;

	rot = rot * _offset->invRotation;
	pos -= rot * _offset->position;
}

void Transform::onDispose()
{
	if (_syncGroup)
	{
		_syncGroup->remove(this);
		_syncGroup = NULL;
	}

	safeDelete(_offset);

	Component::onDispose();
}

void Transform::joinSyncGroup(Transform* target)
{
	if (isDisposed()) return;

	if (target == this) 
	{
		LOG(0, "*** Cannot sync with self\n");
		return;
	}

	if (_syncGroup)
		_syncGroup->remove(this);

	if (target == NULL || target->isDisposed())
	{
		_syncGroup = NULL;
		return;
	}

	if (target->_syncGroup)
	{
		target->_syncGroup->add(this);
		_syncGroup = target->_syncGroup;
	}
	else
	{
		_syncGroup = new SyncGroup();
		target->_syncGroup = _syncGroup;
		_syncGroup->add(this);
		_syncGroup->add(target);
	}

	// sync myself to them now
	target->_synchronize();
}

////////////////////////////////////////////////////////////////////////////////

TransformAnchor::TransformAnchor(Transform* source, Transform* target)
{
	_deltaValid = false;
	_delta = Vector3::ZERO;

	setSource(source);
	setTarget(target);
}

void TransformAnchor::onDispose()
{
	_source = NULL;
	_target = NULL;
	_deltaValid = false;

	if (_sourceSyncGroup)
	{
		_sourceSyncGroup->remove(this);
		_sourceSyncGroup = NULL;
	}

	Transform::onDispose();
}

void TransformAnchor::setSource(Transform* source)
{
	if (isDisposed()) return;
	if (_source == source) return;
	if (source->isDisposed()) source = NULL;

	ASSERT(source != this);
	ASSERT(_target == NULL || source != _target);

	if (_sourceSyncGroup)
		_sourceSyncGroup->remove(this);

	if (source == NULL)
	{
		_sourceSyncGroup = NULL;
		return;
	}

	if (source->_syncGroup)
	{
		source->_syncGroup->add(this);
		_sourceSyncGroup = source->_syncGroup;
	}
	else
	{
		_sourceSyncGroup = new SyncGroup();
		source->_syncGroup = _sourceSyncGroup;
		_sourceSyncGroup->add(this);
		_sourceSyncGroup->add(source);
	}

	_deltaValid = false;

	beginTransform();

	_source = source;

	endTransform();
}

void TransformAnchor::setTarget(Transform* target)
{
	if (isDisposed()) return;
	if (_target == target) return;
	if (target->isDisposed()) target = NULL;

	ASSERT(target != this);
	ASSERT(_source == NULL || target != _source);

	_deltaValid = false;

	beginTransform();

	_target = target;

	endTransform();
}

void TransformAnchor::onSyncFrom(SyncGroup* group, Transform* from)
{
	if (group == _sourceSyncGroup) 
	{
		if (_target && _source)
		{
			_delta = _target->getPosition() - _source->getPosition();
			_deltaValid = true;
		}
	}
	else if (group == _syncGroup)
	{
		_position = from->getPosition();
	}
}

void TransformAnchor::onSyncGroupIsolated(SyncGroup* group)
{
	if (group == _sourceSyncGroup)
		_sourceSyncGroup = NULL;

	Transform::onSyncGroupIsolated(group);
}

void TransformAnchor::onEndTransform()
{
	if (_source == NULL || _target == NULL) return;

	if (!_deltaValid) return;

	Vector3 delta = _delta;

	Vector3 pos = _position;
	Quat rot = _rotation;
	applyOffset(pos, rot);

	_target->beginTransform();
	_target->setPosition(pos + delta);
	_target->setRotation(rot);
	_target->endTransform();
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
