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

#include "nit/math/NitMath.h"
#include "nit/math/Vector2.h"
#include "nit/math/Vector3.h"
#include "nit/math/Vector4.h"
#include "nit/math/Matrix3.h"
#include "nit/math/Matrix4.h"
#include "nit/math/Quaternion.h"
#include "nit/data/Color.h"
#include "nit/math/Sphere.h"
#include "nit/math/Plane.h"
#include "nit/math/AxisAlignedBox.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// Calc proj, perp vector

//          d
// perp ^  ^  
//      | /
//      |/
// -----+-->----> n
//      proj

template <typename V>
inline V MathBase::proj(const V& d, const V& n)
{
	return (d * n) * n;   // dot(d,n) * n
}

template <typename V>
inline V MathBase::perp(const V& d, const V& n)
{
	return d - proj(d, n);
}

////////////////////////////////////////////////////////////////////////////////

// Calc reflect vector

// d   n   r
//  \  ^  ^
//   \ | /
//    v|/
//  ---+---

template <typename V>
inline V MathBase::reflect(const V& d, const V& n)
{
	return d - 2.0f * proj(d, n);
}

////////////////////////////////////////////////////////////////////////////////

// Calc 1D-bezier ease-in ease-out

template <typename T>
inline T MathBase::easeInOut(float t, const T& a, const T& b)
{
	// 	float u = t;
	// 	float v = 1.0f - u;
	// 
	// 	float u2 = u*u;
	// 	float v2 = v*v;
	// 
	// 	return T( (v2*v + 3*v2*u) * a + (3*v*u2 + u2*u) * b );

	float u = (t*t*(3-2*t));
	return T(a + (b-a)*u );
}

////////////////////////////////////////////////////////////////////////////////

// Calc bezier interpolated point

//     v1     v2
//      +-----+
//     /  -+-  \
//    / -- p -- \
//   /--       --\
//  +             +
// v0             v3

// 	u = 1 - t;
// 
// 	p = v0 * 1 * (u * u * u) + 
// 		v1 * 3 * (u * u * t) +
// 		v2 * 3 * (u * t * t) +
// 		v3 * 1 * (t * t * t);

template <typename V>
inline void MathBase::bezier(const V& v0, const V& v1, const V& v2, const V& v3, float t, V* outPos, V* outTan, V* outAcc)
{
	V a = 3.0f * (v1 - v0);
	V b = 3.0f * (v2 - v1) - a;
	V c = (v3 - v0) + 3.0f * (v1 - v2);

	if (outPos)
		*outPos = v0 + (a + (b + c * t) * t) * t;

	if (outTan)
		*outTan = a + (2.0f * b + 3.0f * c * t) * t;

	if (outAcc)
		*outAcc = 2.0f * b + 6.0f * c * t;
}

////////////////////////////////////////////////////////////////////////////////

// Calc hermite interpolated point

//      t0 (dir)
//      ^
//     / --+--
//    /--  p  --
//   +          + p1
//  p0           \
//                v t2 (dir)

// Same with Bezier, when interpret as follow:
// p0 = v0
// p1 = v3
// t0 = 3 * (v1 - v0)
// t1 = 3 * (v3 - v2)

template <typename V>
inline void MathBase::hermite(const V& p0, const V& p1, const V& t0, const V& t1, float t, V* outPos, V* outTan, V* outAcc)
{
	V a = 3.0f * (p1 - p0) - (t1 + 2.0f * t0);
	V b = t0 + t1 - 2.0f * (p1 - p0);

	if (outPos)
		*outPos = p0 + (t0 + (a + b * t) * t) * t;

	if (outTan)
		*outTan = t0 + (2.0f * a + 3.0f * b * t) * t;

	if (outAcc)
		*outAcc = 2.0f * a + 6.0f * b * t;
}

////////////////////////////////////////////////////////////////////////////////

inline Vector3 MathBase::quatRotate(const Quat& r, const Vector3& v)
{
	// 	float rx = r.GetX();
	// 	float ry = r.GetY();
	// 	float rz = r.GetZ();
	// 	float rw = r.GetW();

	float rx = r.x;
	float ry = r.y;
	float rz = r.z;
	float rw = r.w;

	float qx = rw * v.x + ry * v.z - rz * v.y;
	float qy = rw * v.y + rz * v.x - rx * v.z;
	float qz = rw * v.z + rx * v.y - ry * v.x;
	float qw = -rx * v.x - ry * v.y - rz * v.z;

	float ox = - qw * rx + qx * rw - qy * rz + qz * ry;
	float oy = - qw * ry + qy * rw - qz * rx + qx * rz;
	float oz = - qw * rz + qz * rw - qx * ry + qy * rx;

	return Vector3(ox, oy, oz);
}

// Game Programming Gems 1.10
inline Quat MathBase::shortestArc(Vector3 d0, Vector3 d1)
{
	d0.normalise();
	d1.normalise();

	Vector3 c = d0.crossProduct(d1);
	float   d = d0.dotProduct(d1);

	if (d < -1.0f + EPSILON)
		return Quat(1, 0, 0, 0);

	float s = sqrt((1.0f + d) * 2.0f);
	float rs = 1.0f / s;

	return Quat(s * 0.5f, c.x * rs, c.y * rs, c.z * rs);
}


inline Matrix3 MathBase::upFront(const Vector3& up, Vector3 front)
{
	Vector3 right = front.crossProduct(up);
	front = up.crossProduct(right);
	Matrix3 ret; ret.FromAxes(-right, -front, up);
	return ret;
}

inline Matrix3 MathBase::upRight(const Vector3& up, Vector3 right)
{
	Vector3 front = up.crossProduct(right);
	right = front.crossProduct(up);
	Matrix3 ret; ret.FromAxes(-right, -front, up);
	return ret;
}

inline Matrix3 MathBase::frontUp(const Vector3& front, Vector3 up)
{
	Vector3 right = front.crossProduct(up);
	up = right.crossProduct(front);
	Matrix3 ret; ret.FromAxes(-right, -front, up);
	return ret;
}

inline Matrix3 MathBase::frontRight(const Vector3& front, Vector3 right)
{
	Vector3 up = right.crossProduct(front);
	right = front.crossProduct(up);
	Matrix3 ret; ret.FromAxes(-right, -front, up);
	return ret;
}

inline Matrix3 MathBase::rightUp(const Vector3& right, Vector3 up)
{
	Vector3 front = up.crossProduct(right);
	up = right.crossProduct(front);
	Matrix3 ret; ret.FromAxes(-right, -front, up);
	return ret;
}

inline Matrix3 MathBase::rightFront(const Vector3& right, Vector3 front)
{
	Vector3 up = right.crossProduct(front);
	front = up.crossProduct(right);
	Matrix3 ret; ret.FromAxes(-right, -front, up);
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

// Generate an othogonal reference frame by an arbitary axis using gram-schmidt

inline Matrix3 MathBase::pinUp(const Vector3& dir)
{
	Vector3 up = dir;
	Vector3 right;
	Vector3 front;

	up.normalise();

	if (abs(up.x) > 0.577f)
		right = up.crossProduct(Vector3(-up.y, up.x, 0.0f));
	else
		right = up.crossProduct(Vector3(-up.y, up.z, 0.0f));

	right.normalise();
	front = up.crossProduct(right);

	Matrix3 ret; ret.FromAxes(-right, -front, up);
	return ret;
}

inline Matrix3 MathBase::pinFront(const Vector3& dir)
{
	Vector3 up;
	Vector3 right;
	Vector3 front = dir;

	front.normalise();

	if (abs(front.z) > 0.577f)
		right = front.crossProduct(Vector3(-front.y, front.z, 0.0f));
	else
		right = front.crossProduct(Vector3(-front.y, front.x, 0.0f));

	right.normalise();
	up = right.crossProduct(front);

	Matrix3 ret; ret.FromAxes(-right, -front, up);
	return ret;
}

inline Matrix3 MathBase::pinRight(const Vector3& dir)
{
	Vector3 up;
	Vector3 right = dir;
	Vector3 front;

	right.normalise();

	if (abs(right.z) > 0.577f)
		front = right.crossProduct(Vector3(-right.y, right.z, 0.0f));
	else
		front = right.crossProduct(Vector3(-right.y, right.x, 0.0f));

	front.normalise();
	up = right.crossProduct(front);

	Matrix3 ret; ret.FromAxes(-right, -front, up);
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;