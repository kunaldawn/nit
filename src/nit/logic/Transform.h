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

#include "nit/logic/Component.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API Transform : public Component
{
public:
	struct OffsetTransform
	{
		Vector3							position;
		Quat							rotation;
		Quat							invRotation;
	};

public:
	Transform();

public:
	void								beginTransform()						{ if (_transformCount++ == 0) { onBeginTransform(); } }
	void								endTransform()							{ if (--_transformCount == 0) { onEndTransform(); _synchronize(); ++_updateStamp; } }
	bool								isUpdating()							{ return _transformCount > 0; }
	uint								getUpdateStamp()						{ return _updateStamp; }

	const Vector3&						getPosition()							{ return _position; }
	const Quat&							getRotation()							{ return _rotation; }

	void								setPosition(const Vector3& pos)			{ beginTransform(); _position = pos; endTransform(); }
	void								setRotation(const Quat& rot)			{ beginTransform(); _rotation = rot; endTransform(); }

public:
	void								move(const Vector3& delta);
	void								rotate(const Quat& delta);

public:
	Vector3								getFront();
	Vector3								getRight();
	Vector3								getUp();

	void								alignUpFront(const Vector3& up, const Vector3& front);
	void								alignUpRight(const Vector3& up, const Vector3& right);
	void								alignFrontUp(const Vector3& front, const Vector3& up);
	void								alignFrontRight(const Vector3& front, const Vector3& right);
	void								alignRightUp(const Vector3& right, const Vector3& up);
	void								alignRightFront(const Vector3& right, const Vector3& front);

	Vector3								toWorldPos(const Vector3& localPos)		{ return _rotation * localPos + _position; }
	Vector3								toLocalPos(const Vector3& worldPos)		{ return _rotation.Inverse() * (worldPos - _position); }

	Vector3								toWorldDir(const Vector3& localDir)		{ return _rotation * localDir; }
	Vector3								toLocalDir(const Vector3& worldDir)		{ return _rotation.Inverse() * worldDir; }

public:
	void								clearOffset();

	Vector3								getPosOffset()							{ return _offset ? _offset->position : Vector3::ZERO; }
	Quat								getRotOffset()							{ return _offset ? _offset->rotation : Quat::IDENTITY; }
	Quat								getInvRotOffset()						{ return _offset ? _offset->invRotation : Quat::IDENTITY; }

	void								setPosOffset(const Vector3& pos);
 	void								setRotOffset(const Quat& rot);

	void								applyOffset(Vector3& pos, Quat& rot);
	void								applyInvOffset(Vector3& pos, Quat& rot);

protected:
	virtual void						onActivate();
	
public:
	class SyncGroup : public RefCounted
	{
	public:
		typedef set<Transform*>::type SyncSet;

		SyncGroup() : _synchronizing(false) { }

		void							add(Transform* member)				{ _syncSet.insert(member); }
		void							remove(Transform* member);
		void							syncFrom(Transform* from);

		SyncSet							_syncSet;
		bool							_synchronizing;
	};

	void								joinSyncGroup(Transform* target);
	SyncGroup*							getSyncGroup()							{ return _syncGroup; }

protected:
	virtual void						onDispose();

protected:
	Quat								_rotation;
	Vector3								_position;
	short								_transformCount;
	short								_updateStamp;

	OffsetTransform*					_offset;

protected:
	virtual void						onBeginTransform()						{ }
	virtual void						onEndTransform()						{ }
	virtual void						onSyncFrom(SyncGroup* group, Transform* from);
	virtual void						onSyncGroupIsolated(SyncGroup* group);

	void								_synchronize();

private:
	friend class						TransformAnchor;
	Ref<SyncGroup>						_syncGroup;
};

////////////////////////////////////////////////////////////////////////////////

class TransformAnchor : public Transform
{
public:
	TransformAnchor(Transform* source = NULL, Transform* target = NULL);

public:
	Transform*							getSource() 							{ return _source; }
	Transform*							getTarget() 							{ return _target; }

	void								setSource(Transform* source);
	void								setTarget(Transform* target);

	const Vector3*						getDelta()								{ return _deltaValid ? &_delta : NULL; }

protected:
	virtual void						onDispose();
	virtual void						onSyncFrom(SyncGroup* group, Transform* from);
	virtual void						onSyncGroupIsolated(SyncGroup* group);
	virtual void						onEndTransform();

private:
	Weak<Transform>						_source;
	Weak<Transform>						_target;

	Vector3								_delta;
	Ref<SyncGroup>						_sourceSyncGroup;
	bool								_deltaValid;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
