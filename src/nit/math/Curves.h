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

#include "nit/math/NitMath.h"

#include "nit/math/Solver.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

template <class TVector>
class TBezierCurve : public RefCounted
{
public:
	// Construct with copy of TVector array
	//    +---+
	//   /1   2\
	//  /       \
	// +         +        +
	// 0         3\      /6
	//             \4   /
	//              +--+5
	// numPoints should be 3 * n + 1, n = 1, 2, 3, ...

	TBezierCurve(const TVector* points, uint numPoints);
	virtual								~TBezierCurve();

protected:
	TBezierCurve()						{ }

public:
	void								interpolate(float t, TVector* outPos, TVector* outTan, TVector* outAcc);
	TVector								pos(float t);
	TVector								tan(float t);

	float								getTotalLength();

	float								toLength(float t);
	float								fromLength(float length);

public:
	uint								getNumSections()												{ return _numSections; }
	TVector*							getSection(int section)											{ return (0 <= section && section < (int)_numSections) ? &_points[section * 3] : NULL; }

	uint								getNumPoints()													{ return _numPoints; }

	const TVector&						getPoint(uint index)											{ return (index < _numPoints) ? _points[index] : TVector::ZERO; }
	void								setPoint(uint index, const TVector& pt);

	bool								isLooping()														{ return Math::isZero((_points[0] - _points[_numPoints-1]).squaredLength()); }

public:									// Common functionality
	void								findNearestPoint(const TVector& P, float& outT, TVector& outP);
	void								sphereIntersect(float t, float r, float& outT1, float& outT2);

protected:
	void								bezier(int section, float normT, TVector* outPos, TVector* outTan, TVector* outAcc);

	void								toNorm(float t, int& outSection, float& outNormT);
	float								fromNorm(int section, float normT);

	void								updateLength();

	float								speedAt(int section, float normT);
	float								lengthAt(int section, float normT);

	uint								_numSections;
	uint								_numPoints;
	TVector*							_points;

	float								_totalLength;
	float*								_partialLength;
};

class Curve2D;
class Curve3D;

////////////////////////////////////////////////////////////////////////////////

class Curve2D : public TBezierCurve<Vector2>
{
public:
	enum PlaneAxis						{ AXIS_X, AXIS_Y, AXIS_Z };

	// Construct with copy of Vector2 array
	Curve2D(const Vector2* points, uint numPoints);

	// Construct from 3d curve by a plane projection
	Curve2D(Curve3D* c, PlaneAxis axis);

	static Curve2D*						ratio(Curve2D* a, Curve2D* b, float r);

protected:
	Curve2D()							{ }

public:									// Curve2D only functionality
	bool								findNearestCrossing(const Vector2& P, const Vector2& D, float& outT, Vector2& outP);
};

////////////////////////////////////////////////////////////////////////////////

class Curve3D : public TBezierCurve<Vector3>
{
public:
	// Construct with copy of Vector3 array
	Curve3D(const Vector3* points, uint numPoints);

	static Curve3D*						ratio(Curve3D* a, Curve3D* b, float r);

protected:
	Curve3D()							{ }

public:									// Curve3D only functionality
	bool								nearestRayPoint(const Ray& r, float& outT, float& outRT);
	bool								closestPlaneIntersect(const Vector3& P, const Vector3& n, float& outT, Vector3& outP);
	bool								findTrackLine(Curve3D* other, const Vector3& P, const Vector3& N, float& outT, Vector3& outVa, Vector3& outVb);
	void								findAllTrackLine(Curve3D* other, const Vector3& P, const Vector3& N, vector<float>::type& outTArray);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;

#include "nit/math/Curves.inl"