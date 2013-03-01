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

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

#include "nit/math/Solver.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NB_Math : NitBind
{
public:
	static void Register(HSQUIRRELVM v)
	{
		sq_pushroottable(v);					// [root]
		sq_pushstring(v, "math", -1);			// [root] "math"
		sq_get(v, -2);							// [root] [math]

		sq_register_h(v, "log",		Log,		"(v, n=E): float");
		sq_register_h(v, "lerp",	Lerp,		"(t: float, a, b: V): V");
		sq_register_h(v, "ease",	EaseInOut,	"(t: float, a, b: V): V // smoothstep");
		sq_register_h(v, "bezier",	Bezier,		"(v0, v1, v2, v3: V, t: float, out tan, out acc: V): pos: V");
		sq_register_h(v, "hermite", Hermite,	"(p0, p1, t0, t1: V, t: float, out tan, out acc: V): pos: V");
		sq_register_h(v, "rad",		Rad,		"(deg: float): rad: float");
		sq_register_h(v, "deg", 	Deg,		"(rad: float): deg: float");
		sq_register_h(v, "max", 	Max,		"(a[, ...]): max: number");
		sq_register_h(v, "min", 	Min,		"(a[, ...]): min: number");
		sq_register_h(v, "cbrt", 	CubicRoot,	"(value: number): number");
		sq_register_h(v, "sign", 	Sign,		"(value: number): number");
		sq_register_h(v, "proj", 	Proj,		"(d, n: Vector): Vector");
		sq_register_h(v, "perp", 	Perp,		"(d, n: Vector): Vector");
		sq_register_h(v, "reflect", Reflect,	"(d, n: Vector): Vector");
		sq_register_h(v, "clamp",	Clamp,		"(v, min, max: number): number");
		sq_register_h(v, "wrap",	Wrap,		"(v, min, max: number): number");
		sq_register_h(v, "solve",	Solve,		"(c0, c1, c2, c3, c4: number): number[4]\n"
												"(c0, c1, c2, c3: number): number[3]\n"
												"(c0, c1, c2: number): number[2]\n"
												"(c0, c1: number): number[1]\n");

		sq_pop(v, 2);							// empty
	}

	static SQRESULT Log(HSQUIRRELVM v)
	{
		if (isNull(v, 2))
			return push(v, Math::log(getFloat(v, 2)));
		else
			return push(v, Math::LogN(getFloat(v, 2), getFloat(v, 3)));
	}

	static SQRESULT Lerp(HSQUIRRELVM v)
	{
		if (is<Vector2>(v, 3))
			return push(v, Math::lerp(getFloat(v, 2), *get<Vector2>(v, 3), *get<Vector2>(v, 4)));
		if (is<Vector3>(v, 3))
			return push(v, Math::lerp(getFloat(v, 2), *get<Vector3>(v, 3), *get<Vector3>(v, 4)));

		return push(v, Math::lerp(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4)));
	}

	static SQRESULT EaseInOut(HSQUIRRELVM v)
	{
		if (is<Vector2>(v, 3))
			return push(v, Math::easeInOut(getFloat(v, 2), *get<Vector2>(v, 3), *get<Vector2>(v, 4)));
		if (is<Vector3>(v, 3))
			return push(v, Math::easeInOut(getFloat(v, 2), *get<Vector3>(v, 3), *get<Vector3>(v, 4)));

		return push(v, Math::easeInOut(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4)));
	}

	static SQRESULT Bezier(HSQUIRRELVM v)
	{
		if (is<Vector2>(v, 3))
		{
			Vector2* outPos = get<Vector2>(v, 7);
			Math::bezier(
				*get<Vector2>(v, 2), *get<Vector2>(v, 3), *get<Vector2>(v, 4), *get<Vector2>(v, 5), // v0, v1, v2, v3
				getFloat(v, 6), outPos, opt<Vector2>(v, 8, NULL), opt<Vector2>(v, 9, NULL));
			return push(v, *outPos);
		}
		else
		{
			Vector3* outPos = get<Vector3>(v, 7);
			Math::bezier(
				*get<Vector3>(v, 2), *get<Vector3>(v, 3), *get<Vector3>(v, 4), *get<Vector3>(v, 5), // v0, v1, v2, v3
				getFloat(v, 6), get<Vector3>(v, 7), opt<Vector3>(v, 8, NULL), opt<Vector3>(v, 9, NULL));
			return push(v, *outPos);
		}
	}

	static SQRESULT Hermite(HSQUIRRELVM v)
	{
		if (is<Vector2>(v, 3))
		{
			Vector2* outPos = get<Vector2>(v, 7);
			Math::bezier(
				*get<Vector2>(v, 2), *get<Vector2>(v, 3), *get<Vector2>(v, 4), *get<Vector2>(v, 5), // v0, v1, v2, v3
				getFloat(v, 6), outPos, opt<Vector2>(v, 8, NULL), opt<Vector2>(v, 9, NULL));
			return push(v, *outPos);
		}
		else
		{
			Vector3* outPos = get<Vector3>(v, 7);
			Math::bezier(
				*get<Vector3>(v, 2), *get<Vector3>(v, 3), *get<Vector3>(v, 4), *get<Vector3>(v, 5), // v0, v1, v2, v3
				getFloat(v, 6), get<Vector3>(v, 7), opt<Vector3>(v, 8, NULL), opt<Vector3>(v, 9, NULL));
			return push(v, *outPos);
		}
	}

	static SQRESULT Sign(HSQUIRRELVM v)
	{
		if (isInt(v, 2))
			return push(v, MathBase::sign(getInt(v, 2))); // TODO: remove Sign from Math
		else
			return push(v, MathBase::sign(getFloat(v, 2)));
	}

	static SQRESULT Rad(HSQUIRRELVM v)
	{
		return push(v, Math::rad(getFloat(v, 2)));
	}

	static SQRESULT Deg(HSQUIRRELVM v)
	{
		return push(v, Math::deg(getFloat(v, 2)));
	}

	static SQRESULT Max(HSQUIRRELVM v)
	{
		int nargs = sq_gettop(v);

		int intMax = INT_MIN;
		float floatMax = Math::MIN_FLOAT;

		bool allInt = true;

		for (int i=2; i<=nargs; ++i)
		{
			if (allInt && !isInt(v, i))
			{
				allInt = false;
				floatMax = (float)intMax;
			}

			if (allInt)
			{
				int arg = getInt(v, i);
				if (arg > intMax)
					intMax = arg;
			}
			else
			{
				float arg = getFloat(v, i);
				if (arg > floatMax)
					floatMax = arg;
			}
		}

		if (allInt)
			return push(v, intMax);
		else
			return push(v, floatMax);
	}

	static SQRESULT Min(HSQUIRRELVM v)
	{
		int nargs = sq_gettop(v);

		int intMin = INT_MAX;
		float floatMin = Math::MAX_FLOAT;

		bool allInt = true;

		for (int i=2; i<=nargs; ++i)
		{
			if (allInt && !isInt(v, i))
			{
				allInt = false;
				floatMin = (float)intMin;
			}

			if (allInt)
			{
				int arg = getInt(v, i);
				if (arg < intMin)
					intMin = arg;
			}
			else
			{
				float arg = getFloat(v, i);
				if (arg < floatMin)
					floatMin = arg;
			}
		}

		if (allInt)
			return push(v, intMin);
		else
			return push(v, floatMin);
	}

	static SQRESULT Proj(HSQUIRRELVM v)
	{
		if (is<Vector2>(v, 2))
			return push(v, Math::proj(*get<Vector2>(v, 2), *get<Vector2>(v, 3)));
		else
			return push(v, Math::proj(*get<Vector3>(v, 2), *get<Vector3>(v, 3)));
	}

	static SQRESULT Perp(HSQUIRRELVM v)
	{
		if (is<Vector2>(v, 2))
			return push(v, Math::perp(*get<Vector2>(v, 2), *get<Vector2>(v, 3)));
		else
			return push(v, Math::perp(*get<Vector3>(v, 2), *get<Vector3>(v, 3)));
	}

	static SQRESULT Reflect(HSQUIRRELVM v)
	{
		if (is<Vector2>(v, 2))
			return push(v, Math::reflect(*get<Vector2>(v, 2), *get<Vector2>(v, 3)));
		else
			return push(v, Math::reflect(*get<Vector3>(v, 2), *get<Vector3>(v, 3)));
	}

	static SQRESULT CubicRoot(HSQUIRRELVM v)
	{
		return push(v, Math::cubicRoot(getFloat(v, 2)));
	}

	static SQRESULT Solve(HSQUIRRELVM v)
	{
		int nargs = sq_gettop(v) - 1;

		if (nargs < 2 || nargs > 5)
			return sqx_throwfmt(v, "cannot solve %d terms", nargs);

		float c[5];
		float s[4];

		for (int i=0; i<nargs; ++i)
		{
			c[i] = getFloat(v, 2+i);
		}

		int n = RootSolver::solve(c, s, nargs);

		if (n == 0)
			return 0;
		
		sq_newarray(v, 0);
		for (int i=0; i<n; ++i)
		{
			sq_pushfloat(v, s[i]);
			sq_arrayappend(v, -2);
		}
		return 1;
	}

	static SQInteger Clamp(HSQUIRRELVM v)
	{
		return push(v, Math::clamp(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4)));
	}

	static SQInteger Wrap(HSQUIRRELVM v)
	{
		return push(v, Math::wrap(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4)));
	}

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::Vector2, NULL);

class NB_Vector2 : TNitClass<Vector2>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(x),
			PROP_ENTRY	(y),

			PROP_ENTRY_R(length),
			PROP_ENTRY_R(sqrLength),
			PROP_ENTRY_R(perpendicular),
			PROP_ENTRY_R(unit),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(x=0, y=0: float): Vector2"),
			FUNC_ENTRY	(_add),
			FUNC_ENTRY	(_sub),
			FUNC_ENTRY	(_div),
			FUNC_ENTRY	(_unm),
			FUNC_ENTRY	(_mul),
			FUNC_ENTRY	(_tostring),
			
			FUNC_ENTRY_H(normalise,		"(): float // changes this"),
			FUNC_ENTRY_H(dot,			"(v: Vector2): Vector2 // != this * v"),
			FUNC_ENTRY_H(cross,			"(v: Vector2): Vector2"),
			FUNC_ENTRY_H(distance,		"(v: Vector2): Vector2"),
			FUNC_ENTRY_H(sqrDistance,	"(v: Vector2): Vector2"),
			FUNC_ENTRY_H(midPoint,		"(v: Vector2): Vector2"),
			FUNC_ENTRY_H(reflect,		"(v: Vector2): Vector2"),
			
			FUNC_ENTRY_H(toDir3d,		"(front, up: Vector3): Vector3"),
			FUNC_ENTRY_H(toRotate3d,	"(front, up: Vector3): Quat"),

			FUNC_ENTRY_H(nearZero,		"(): bool // test if Sqrlength < 1e-06"),
			FUNC_ENTRY_H(nearEqual,		"(v: Vector2): bool // test if SqrDisance < 1e-06"),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "ZERO",			Vector2::ZERO);
		addStatic(v, "UNIT_X",			Vector2::UNIT_X);
		addStatic(v, "UNIT_Y",			Vector2::UNIT_Y);
		addStatic(v, "UNIT_SCALE",		Vector2::UNIT_SCALE);
	}

	NB_CONS()
	{
		new (self(v)) Vector2(optFloat(v, 2, 0.0f), optFloat(v, 3, 0.0f));

		return 0;
	}

	NB_PROP_GET(x)						{ return push(v, self(v)->x); }
	NB_PROP_GET(y)						{ return push(v, self(v)->y); }
	NB_PROP_GET(length)					{ return push(v, self(v)->length());}
	NB_PROP_GET(sqrLength)				{ return push(v, self(v)->squaredLength()); }
	NB_PROP_GET(perpendicular)			{ return push(v, self(v)->perpendicular()); }
	NB_PROP_GET(unit)					{ return push(v, self(v)->Unit()); }

	NB_PROP_SET(x)						{ self(v)->x = getFloat(v, 2); return 0; }
	NB_PROP_SET(y)						{ self(v)->y = getFloat(v, 2); return 0; }

	NB_FUNC(_add)						{ return push(v, *self(v) + *get<Vector2>(v, 2)); }
	NB_FUNC(_sub)						{ return push(v, *self(v) - *get<Vector2>(v, 2)); }
	NB_FUNC(_div)						
	{
		if (isNumber(v, 2))
			return push(v, *self(v) / getFloat(v, 2));
		return push(v, *self(v) / *get<Vector2>(v, 2));
	}

	NB_FUNC(_unm)						{ return push(v, -*self(v)); }

	NB_FUNC(_mul)
	{
		if (isNumber(v, 2))
			return push(v, *get<Vector2>(v, 1) * getFloat(v, 2));
		return push(v, *get<Vector2>(v, 1) * *get<Vector2>(v, 2));
	}

	NB_FUNC(_tostring)
	{
		Vector2* vec = self(v);
		pushFmt(v, "Vector2(%.3f, %.3f)", vec->x, vec->y);
		return 1;
	}
	
	static Vector3 vector2ToDir3d(const Vector2& v, const Vector3& front, const Vector3& up)
	{
		if (v.isZeroLength()) return Vector3::ZERO;

		float phi = Math::acos(-v.y);
		if (v.x > 0.0f)
			phi = -phi;

		Quat rot(Radian(phi), up);

		Vector3 dir = Math::perp(front, up);
		dir.normalise();

		dir = rot * dir;
		
		return dir;
	}
	
	static Quat vector2ToRotate3d(const Vector2& v, const Vector3& front, const Vector3& up)
	{
		if (v.isZeroLength()) return Quat::IDENTITY;
		
		Vector3 dir = vector2ToDir3d(v, front, up);
		
		return Quat(up.crossProduct(dir), -dir, up);
	}
	
	NB_FUNC(toDir3d)					{ return push(v, vector2ToDir3d(*self(v), *get<Vector3>(v, 2), *get<Vector3>(v, 3))); }
	NB_FUNC(toRotate3d)					{ return push(v, vector2ToRotate3d(*self(v), *get<Vector3>(v, 2), *get<Vector3>(v, 3))); }

	NB_FUNC(normalise)					{ return push(v, self(v)->normalise()); }
	NB_FUNC(dot)						{ return push(v, self(v)->dotProduct(*get<Vector2>(v, 2)) ); }
	NB_FUNC(cross)						{ return push(v, self(v)->crossProduct(*get<Vector2>(v, 2)) ); }
	NB_FUNC(distance)					{ return push(v, self(v)->distance(*get<Vector2>(v, 2)) ); }
	NB_FUNC(sqrDistance)				{ return push(v, self(v)->squaredDistance(*get<Vector2>(v, 2)) ); }
	NB_FUNC(midPoint)					{ return push(v, self(v)->midPoint(*get<Vector2>(v, 2)) ); }
	NB_FUNC(reflect)					{ return push(v, self(v)->reflect(*get<Vector2>(v, 2)) ); }
	NB_FUNC(nearZero)					{ return push(v, self(v)->squaredLength() < 1e-06f); }
	NB_FUNC(nearEqual)					{ return push(v, self(v)->squaredDistance(*get<Vector2>(v, 2)) < 1e-06f); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::Vector3, NULL);

class NB_Vector3 : TNitClass<Vector3>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(x),
			PROP_ENTRY	(y),
			PROP_ENTRY	(z),

			PROP_ENTRY_R(length),
			PROP_ENTRY_R(sqrLength),
			PROP_ENTRY_R(perpendicular),
			PROP_ENTRY_R(unit),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(x=0, y=0, z=0: float): Vector3"),

			FUNC_ENTRY	(_add),
			FUNC_ENTRY	(_sub),
			FUNC_ENTRY	(_div),
			FUNC_ENTRY	(_unm),
			FUNC_ENTRY	(_mul),
			FUNC_ENTRY	(_tostring),

			FUNC_ENTRY_H(normalise,		"(): float // changes this"),
			FUNC_ENTRY_H(dot,			"(v: Vector3): Vector3 // != this * v"),
			FUNC_ENTRY_H(absDot,		"(v: Vector3): Vector3"),
			FUNC_ENTRY_H(cross,			"(v: Vector3): Vector3"),
			FUNC_ENTRY_H(distance,		"(v: Vector3): Vector3"),
			FUNC_ENTRY_H(sqrDistance,	"(v: Vector3): Vector3"),
			FUNC_ENTRY_H(midPoint,		"(v: Vector3): Vector3"),
			FUNC_ENTRY_H(reflect,		"(v: Vector3): Vector3"),

			FUNC_ENTRY_H(nearZero,		"(): bool // test if Sqrlength < 1e-06"),
			FUNC_ENTRY_H(nearEqual,		"(v: Vector3): bool // test if SqrDisance < 1e-06"),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "ZERO",			Vector3::ZERO);
		addStatic(v, "UNIT_X", 			Vector3::UNIT_X);
		addStatic(v, "UNIT_Y", 			Vector3::UNIT_Y);
		addStatic(v, "UNIT_Z", 			Vector3::UNIT_Z);
		addStatic(v, "UNIT_SCALE",		Vector3::UNIT_SCALE);

		addStatic(v, "FRONT",			Vector3( 0, -1,  0));
		addStatic(v, "BACK",			Vector3( 0,  1,  0));
		addStatic(v, "LEFT",			Vector3( 1,  0,  0));
		addStatic(v, "RIGHT",			Vector3(-1,  0,  0));
		addStatic(v, "UP",				Vector3( 0,  0,  1));
		addStatic(v, "DOWN",			Vector3( 0,  0, -1));
	}

	NB_CONS()
	{
		new (self(v)) Vector3(optFloat(v, 2, 0.0f), optFloat(v, 3, 0.0f), optFloat(v, 4, 0.0f));

		return 0;
	}

	NB_PROP_GET(x)						{ return push(v, self(v)->x); }
	NB_PROP_GET(y)						{ return push(v, self(v)->y); }
	NB_PROP_GET(z)						{ return push(v, self(v)->z); }
	NB_PROP_GET(length)					{ return push(v, self(v)->length());}
	NB_PROP_GET(sqrLength)				{ return push(v, self(v)->squaredLength()); }

	NB_PROP_GET(perpendicular)			{ return push(v, self(v)->perpendicular()); }
	NB_PROP_GET(unit)					{ return push(v, self(v)->Unit()); }

	NB_PROP_SET(x)						{ self(v)->x = getFloat(v, 2); return 0; }
	NB_PROP_SET(y)						{ self(v)->y = getFloat(v, 2); return 0; }
	NB_PROP_SET(z)						{ self(v)->z = getFloat(v, 2); return 0; }

	NB_FUNC(_add)						{ return push(v, *self(v) + *get<Vector3>(v, 2)); }
	NB_FUNC(_sub)						{ return push(v, *self(v) - *get<Vector3>(v, 2)); }
	NB_FUNC(_div)						
	{
		if (isNumber(v, 2))
			return push(v, *self(v) / getFloat(v, 2));
		return push(v, *self(v) / *get<Vector3>(v, 2));
	}

	NB_FUNC(_unm)						{ return push(v, -*self(v)); }

	NB_FUNC(_mul)
	{
		if (isNumber(v, 2))
			return push(v, *get<Vector3>(v, 1) * getFloat(v, 2));
		return push(v, *get<Vector3>(v, 1) * *get<Vector3>(v, 2));
	}

	NB_FUNC(_tostring)
	{
		Vector3* vec = self(v);
		pushFmt(v, "Vector3(%.3f, %.3f, %.3f)", vec->x, vec->y, vec->z);
		return 1;
	}

	NB_FUNC(dot)						{ return push(v, self(v)->dotProduct(*get<Vector3>(v, 2)) ); }
	NB_FUNC(absDot)						{ return push(v, self(v)->absDotProduct(*get<Vector3>(v, 2)) ); }
	NB_FUNC(cross)						{ return push(v, self(v)->crossProduct(*get<Vector3>(v, 2)) ); }
	NB_FUNC(distance)					{ return push(v, self(v)->distance(*get<Vector3>(v, 2)) ); }
	NB_FUNC(sqrDistance)				{ return push(v, self(v)->squaredDistance(*get<Vector3>(v, 2)) ); }
	NB_FUNC(midPoint)					{ return push(v, self(v)->midPoint(*get<Vector3>(v, 2)) ); }
	NB_FUNC(reflect)					{ return push(v, self(v)->reflect(*get<Vector3>(v, 2)) ); }
	NB_FUNC(normalise)					{ return push(v, self(v)->normalise()); }
	NB_FUNC(nearZero)					{ return push(v, self(v)->squaredLength() < 1e-06f); }
	NB_FUNC(nearEqual)					{ return push(v, self(v)->squaredDistance(*get<Vector3>(v, 2)) < 1e-06f); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::Vector4, NULL);

class NB_Vector4 : TNitClass<Vector4>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(x),
			PROP_ENTRY	(y),
			PROP_ENTRY	(z),
			PROP_ENTRY	(w),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(x=0, y=0, z=0, w=0: float): Vector4"),

			FUNC_ENTRY	(_add),
			FUNC_ENTRY	(_sub),
			FUNC_ENTRY	(_div),
			FUNC_ENTRY	(_unm),
			FUNC_ENTRY	(_mul),
			FUNC_ENTRY	(_tostring),

			FUNC_ENTRY_H(dot,			"(v: Vector4): Vector4 // != this * v"),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "ZERO",	Vector4::ZERO);
	}
	
	NB_CONS()
	{
		new (self(v)) Vector4(optFloat(v, 2, 0.0f), optFloat(v, 3, 0.0f), optFloat(v, 4, 0.0f), optFloat(v, 5, 0.0f));

		return 0;
	}

	NB_PROP_GET(x)						{ return push(v, self(v)->x); }
	NB_PROP_GET(y)						{ return push(v, self(v)->y); }
	NB_PROP_GET(z)						{ return push(v, self(v)->z); }
	NB_PROP_GET(w)						{ return push(v, self(v)->w); }

	NB_PROP_SET(x)						{ self(v)->x = getFloat(v, 2); return 0; }
	NB_PROP_SET(y)						{ self(v)->y = getFloat(v, 2); return 0; }
	NB_PROP_SET(z)						{ self(v)->z = getFloat(v, 2); return 0; }
	NB_PROP_SET(w)						{ self(v)->w = getFloat(v, 2); return 0; }

	NB_FUNC(_add)						{ return push(v, *self(v) + *get<Vector4>(v, 2)); }
	NB_FUNC(_sub)						{ return push(v, *self(v) - *get<Vector4>(v, 2)); }
	NB_FUNC(_div)						
	{
		if (isNumber(v, 2))
			return push(v, *self(v) / getFloat(v, 2));
		return push(v, *self(v) / *get<Vector4>(v, 2));
	}

	NB_FUNC(_unm)						{ return push(v, -*self(v)); }

	NB_FUNC(_mul)
	{
		if (isNumber(v, 2))
			return push(v, *get<Vector4>(v, 1) * getFloat(v, 2));
		return push(v, *get<Vector4>(v, 1) * *get<Vector4>(v, 2));
	}

	NB_FUNC(_tostring)
	{
		Vector4* vec = self(v);
		pushFmt(v, "Vector4(%.3f, %.3f, %.3f, %.3f)", vec->x, vec->y, vec->z, vec->w);
		return 1;
	}

	NB_FUNC(dot)						{ return push(v, self(v)->dotProduct(*get<Vector4>(v, 2)) ); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::Matrix3, NULL);

class NB_Matrix3 : TNitClass<Matrix3>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(forward),
			PROP_ENTRY_R(up),
			PROP_ENTRY_R(right),
			PROP_ENTRY_R(column0),
			PROP_ENTRY_R(column1),
			PROP_ENTRY_R(column2),
			NULL,
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(m00, m01, m02, m10, m11, m12, m20, m21, m22: float): Matrix3 // m[row][column]\n"
										"(fwd, up: Vector3): Matrix3 // == 'Matrix3.ForwardUp(fwd, up)'\n"
										"(fwd, up, right: Vector3): Matrix3 // does not orthonormalize\n"
										"(radian_angle: float, axis: Vector3): Matrix3\n"
										"(from: Quat): Matrix3\n"
										"(): Matrix3 // identity"),
			FUNC_ENTRY	(_add),
			FUNC_ENTRY	(_sub),
			FUNC_ENTRY	(_mul),
			FUNC_ENTRY	(_tostring),
			
			FUNC_ENTRY_H(setColumn,		"(col: int, v: Vector3)"),
			FUNC_ENTRY_H(getColumn,		"(col: int): Vector3"),
			FUNC_ENTRY_H(hasScale,		"(): bool"),
			FUNC_ENTRY_H(swap,			"(with: Matrix3)"),
			FUNC_ENTRY_H(fromAxes,		"(xAxis, yAxis, zAxis: Vector3): Matrix3 // changes this"),
			FUNC_ENTRY_H(inverse,		"(tolerance: float = 1e-06): Vector3"),
			FUNC_ENTRY_H(transpose,		"(): Matrix3"),
			FUNC_ENTRY_H(determinant,	"(): float"),
			FUNC_ENTRY_H(spectralNorm,	"(): float"),
			FUNC_ENTRY_H(toAngleAxis,	"(out axis: Vector3): float // returns radian angle"),
			FUNC_ENTRY_H(fromAngleAxis,	"(radian_angle: float, axis: Vector3) // changes this"),
			FUNC_ENTRY_H(upFront,		"[class] (up, fwd: Vector3): Matrix3"),
			FUNC_ENTRY_H(upRight,		"[class] (up, right: Vector3): Matrix3"),
			FUNC_ENTRY_H(frontUp,		"[class] (fwd, up: Vector3): Matrix3"),
			FUNC_ENTRY_H(frontRight,	"[class] (fwd, right: Vector3): Matrix3"),
			FUNC_ENTRY_H(rightUp,		"[class] (right, up: Vector3): Matrix3"),
			FUNC_ENTRY_H(rightFront,	"[class] (right, fwd: Vector3): Matrix3"),
			FUNC_ENTRY_H(pinUp,			"[class] (up: Vector3): Matrix3"),
			FUNC_ENTRY_H(pinRight,		"[class] (right: Vector3): Matrix3"),
			FUNC_ENTRY_H(pinFront,		"[class] (front: Vector3): Matrix3"),

			FUNC_ENTRY_H(singularValueDecomposition,	"(out L: Matrix3, out S: Vector3, out R: Matrix3)"),
			FUNC_ENTRY_H(singularValueComposition,		"(L: Matrix3, S: Vector3, R: Matrix3)"),
			FUNC_ENTRY_H(orthonormalize,				"(): Matrix3"),
			FUNC_ENTRY_H(qduDecomposition,				"(out Q: Matrix3, out D: Vector3, out U: Vector3)"),
			NULL,
		};

		bind(v, props, funcs);

		addStatic(v, "ZERO",			Matrix3::ZERO);
		addStatic(v, "EPSILON",			Matrix3::EPSILON);
		addStatic(v, "IDENTITY",		Matrix3::IDENTITY);
	}

	NB_CONS()
	{
		int nargs = sq_gettop(v) - 1;

		if (nargs == 0)
		{
			*self(v) = Matrix3::IDENTITY;
		}
		else if (nargs == 1 && is<Quat>(v, 2))
		{
			get<Quat>(v, 2)->ToRotationMatrix(*self(v));
		}
		else if (nargs == 3)
		{
			Matrix3& s = *self(v);
			s.SetColumn(1, -*get<Vector3>(v, 2)); // fwd
			s.SetColumn(2, *get<Vector3>(v, 3));  // up
			s.SetColumn(0, -*get<Vector3>(v, 4)); // right
		}
		else if (nargs == 2 && isNumber(v, 2))
		{
			(*self(v)).FromAxisAngle(*get<Vector3>(v, 3), Radian(getFloat(v, 2)));
		}
		else if (nargs == 2)
		{
			*self(v) = Math::frontUp(*get<Vector3>(v, 2), *get<Vector3>(v, 3));
		}
		else if (nargs == 9)
		{
			new (self(v)) Matrix3(
				getFloat(v, 2), getFloat(v, 3), getFloat(v, 4),
				getFloat(v, 5), getFloat(v, 6), getFloat(v, 7),
				getFloat(v, 8), getFloat(v, 9), getFloat(v, 10));
		}
		else
		{
			sqx_throw(v, "invalid num args");
		}

		return 0;
	}
	
	NB_FUNC(_add)						{ return push(v, *self(v) + *get<Matrix3>(v, 2)); }
	NB_FUNC(_sub)				
	{ 
		Matrix3 m;
		if (isNone(v, 2))
			m = -(*self(v));
		else
			m = *self(v) - *get<Matrix3>(v, 2);

		return push(v, m); 
	}
	NB_FUNC(_mul)				
	{
		Matrix3 m;
		if (is<Vector3>(v, 2))
			return push(v, *self(v) * *get<Vector3>(v, 2));
		else if (isFloat(v, 2))
			m = *self(v) * getFloat(v, 2);
		else if (is<Matrix3>(v, 2))
			m = *self(v) * *get<Matrix3>(v, 2);

		return push(v, m);
	}

	NB_FUNC(_tostring)
	{
		Radian r;
		Vector3 v3;
		self(v)->ToAxisAngle(v3, r);
		pushFmt(v, "Matrix3(%.3f, Vector3(%.3f, %.3f, %.3f))", r.valueRadians(), v3.x, v3.y, v3.z);
		return 1;
	}

	NB_PROP_GET(right)					{ return push(v, -(self(v)->GetColumn(0))); }
	NB_PROP_GET(forward)				{ return push(v, -(self(v)->GetColumn(1))); }
	NB_PROP_GET(up)						{ return push(v, self(v)->GetColumn(2)); }

	NB_PROP_GET(column0)				{ return push(v, self(v)->GetColumn(0)); }
	NB_PROP_GET(column1)				{ return push(v, self(v)->GetColumn(1)); }
	NB_PROP_GET(column2)				{ return push(v, self(v)->GetColumn(2)); }

	NB_FUNC(hasScale)					{ return push(v, self(v)->hasScale()); }

	NB_FUNC(swap)						{ self(v)->swap(*get<Matrix3>(v, 2)); return 0; }

	NB_FUNC(setColumn)					{ self(v)->SetColumn(getInt(v, 2), *get<Vector3>(v, 3)); return 0; }
	NB_FUNC(getColumn)					{ return push(v, self(v)->GetColumn(getInt(v, 2))); }
	
	NB_FUNC(fromAxes)
	{ 
		self(v)->FromAxes(*get<Vector3>(v, 2), *get<Vector3>(v, 3), *get<Vector3>(v, 4));
		return 0;
	}

	NB_FUNC(inverse)
	{
		return push(v, self(v)->Inverse(optFloat(v, 2, Real(1e-06))));
	}

	NB_FUNC(transpose)					{ return push(v, self(v)->Transpose()); }

	NB_FUNC(determinant)				{ return push(v, self(v)->Determinant()); }
	
	NB_FUNC(singularValueDecomposition)
	{
		self(v)->SingularValueDecomposition(
			*get<Matrix3>(v, 2), *get<Vector3>(v, 3), *get<Matrix3>(v, 4));
		return 0;
	}

	NB_FUNC(singularValueComposition)
	{
		self(v)->SingularValueComposition(
			*get<Matrix3>(v, 2), *get<Vector3>(v, 3), *get<Matrix3>(v, 4));
		return 0;
	}

	NB_FUNC(orthonormalize)				{ self(v)->Orthonormalize(); return 0; }

	NB_FUNC(qduDecomposition)	
	{ 
		self(v)->QDUDecomposition(
			*get<Matrix3>(v, 2), *get<Vector3>(v, 3), *get<Vector3>(v, 4));
		return 0;
	}

	NB_FUNC(spectralNorm)				{ return push(v, self(v)->SpectralNorm()); }

	NB_FUNC(toAngleAxis)
	{
		Radian ret;
		self(v)->ToAxisAngle(*get<Vector3>(v, 2), ret);
		return push(v, ret.valueRadians());
	}

	NB_FUNC(fromAngleAxis)				{ self(v)->FromAxisAngle(*get<Vector3>(v, 3), Radian(getFloat(v, 2))); return 0; }

	NB_FUNC(upFront)					{ return push(v, Math::upFront(*get<Vector3>(v, 2), *get<Vector3>(v, 3))); }
	NB_FUNC(upRight)					{ return push(v, Math::upRight(*get<Vector3>(v, 2), *get<Vector3>(v, 3))); }
	NB_FUNC(frontUp)					{ return push(v, Math::frontUp(*get<Vector3>(v, 2), *get<Vector3>(v, 3))); }
	NB_FUNC(frontRight)					{ return push(v, Math::frontRight(*get<Vector3>(v, 2), *get<Vector3>(v, 3))); }
	NB_FUNC(rightUp)					{ return push(v, Math::rightUp(*get<Vector3>(v, 2), *get<Vector3>(v, 3))); }
	NB_FUNC(rightFront)					{ return push(v, Math::rightFront(*get<Vector3>(v, 2), *get<Vector3>(v, 3))); }

	NB_FUNC(pinUp)						{ return push(v, Math::pinUp(*get<Vector3>(v, 2))); }
	NB_FUNC(pinRight)					{ return push(v, Math::pinRight(*get<Vector3>(v, 2))); }
	NB_FUNC(pinFront)					{ return push(v, Math::pinFront(*get<Vector3>(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

 NB_TYPE_VALUE(NIT_API, nit::Quat, NULL);

class NB_Quat : TNitClass<Quat>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(x),
			PROP_ENTRY	(y),
			PROP_ENTRY	(z),
			PROP_ENTRY	(w),

			PROP_ENTRY_R(axisX),
			PROP_ENTRY_R(axisY),
			PROP_ENTRY_R(axisZ),
			
			PROP_ENTRY_R(roll),
			PROP_ENTRY_R(pitch),
			PROP_ENTRY_R(yaw),

			PROP_ENTRY_R(norm),
			PROP_ENTRY_R(normalise),
			PROP_ENTRY_R(inverse),
			PROP_ENTRY_R(unitInverse),
			PROP_ENTRY_R(exp),
			PROP_ENTRY_R(log),
			
			NULL,
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(w, x, y, z: float): Quat\n"
										"(fwd, up: Vector3): Quat // uses Matrix3\n"
										"(fwd, up, right): Quat // uses Matrix3\n"
										"(radian_angle: float, axis: Vector3): Quat\n"
										"(from: Matrix3): Quat\n"
										"(): Quat // identity\n"),

			FUNC_ENTRY	(_add),
			FUNC_ENTRY	(_sub),
			FUNC_ENTRY	(_mul),
			FUNC_ENTRY	(_tostring),

			FUNC_ENTRY_H(swap,			"(other: Quat)"),
			FUNC_ENTRY_H(dot,			"(q: Quat): Quat"),
			FUNC_ENTRY_H(isNan,			"(): bool"),
			FUNC_ENTRY_H(equals,		"(q: Quat, radian_angle: float): bool"),
			
			FUNC_ENTRY_H(fromRotationMatrix,"(rot: Matrix3) // changes this"),
			FUNC_ENTRY_H(toRotationMatrix,	"(out rot: Matrix3)"),
			FUNC_ENTRY_H(fromAngleAxis,		"(radian_angle: float, axis: Vector3) // changes this"),
			FUNC_ENTRY_H(toAngleAxis,		"(out axis: Vector3): float // returns radian angle"),
			FUNC_ENTRY_H(fromAxes,			"(xAxis: Vector3, yAxis: Vector3, zAxis: Vector3) // changes this"),
			FUNC_ENTRY_H(toAxes,			"(out xAxis: Vector3, out yAxis: Vector3, out zAxis: Vector3)"),
			
			FUNC_ENTRY_H(slerp,				"[class] (t: float, p, q: Quat, shortest=false: bool): Quat"),
			FUNC_ENTRY_H(slerpExtraSpins,	"[class] (t: float, p, q: Quat, spin: int): Quat "),
			FUNC_ENTRY_H(intermediate,		"[class] (q0, q1, q2: Quat, out a, out b: Quat)"),
			FUNC_ENTRY_H(squad,				"[class] (t: float, p, a, b, q: Quat, shortest=false: bool): Quat"),
			FUNC_ENTRY_H(nlerp,				"[class] (t: float, p, q: Quat, shortest=false: bool): Quat"),

			FUNC_ENTRY_H(shortestArc,		"[class] (d0, d1: Vector3): Quat"),

			NULL,
		};

		bind(v, props, funcs);

		addStatic(v, "ZERO",			Quat::ZERO);
		addStatic(v, "IDENTITY",		Quat::IDENTITY);
		addStatic(v, "Z_180",			Quat(Degree(180), Vector3::UNIT_Z));
	}

	NB_CONS()
	{
		int nargs = sq_gettop(v) - 1;
		if (nargs == 0)
		{
			*self(v) = Quat::IDENTITY;
		}
		else if (nargs == 1)
		{
			new (self(v)) Quat(*get<Matrix3>(v, 2));
		}
		else if (nargs == 2 && is<Vector3>(v, 2))
		{
			new (self(v)) Quat(Math::frontUp(*get<Vector3>(v, 2), *get<Vector3>(v, 3)));
		}
		else if (nargs == 2 && isNumber(v, 2))
		{
			new (self(v)) Quat(Radian(getFloat(v, 2)), *get<Vector3>(v, 3));
		}
		else if (nargs == 3)
		{
			Matrix3 rot;
			rot.SetColumn(1, -*get<Vector3>(v, 2)); // fwd
			rot.SetColumn(2, *get<Vector3>(v, 3));  // up
			rot.SetColumn(3, -*get<Vector3>(v, 4)); // right
			new (self(v)) Quat(rot);
		}
		else if (nargs == 4)
		{
			new (self(v)) Quat(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4), getFloat(v, 5));
		}
		else
		{
			sqx_throw(v, "invalid num args");
		}

		return 0;
	}

	NB_FUNC(_add)						{ return push(v, *self(v) + *get<Quat>(v, 2)); }
	NB_FUNC(_sub)				
	{ 
		Quat q;
		if (isNone(v, 2))
			q = -(*self(v));
		else
			q = *self(v) - *get<Quat>(v, 2);

		return push(v, q); 
	}
	NB_FUNC(_mul)				
	{
		Quat q;
		if (isFloat(v, 2))
			q = *self(v) * getFloat(v, 2);
		else if (is<Vector3>(v, 2))
			return push(v, *self(v) * *get<Vector3>(v, 2));
		else if (is<Quat>(v, 2))
			q = *self(v) * *get<Quat>(v, 2);

		return push(v, q);
	}

	NB_FUNC(_tostring)
	{
		Radian r;
		Vector3 v3;

		self(v)->ToAngleAxis(r, v3);
		pushFmt(v, "Quat(%.3f, Vector3(%.3f, %.3f, %.3f))", r.valueRadians(), v3.x, v3.y, v3.z);
		return 1;
	}

	NB_PROP_SET(x)						{ self(v)->x = getFloat(v, 2); return 0; }
	NB_PROP_SET(y)						{ self(v)->y = getFloat(v, 2); return 0; }
	NB_PROP_SET(z)						{ self(v)->z = getFloat(v, 2); return 0; }
	NB_PROP_SET(w)						{ self(v)->w = getFloat(v, 2); return 0; }

	NB_PROP_GET(x)						{ return push(v, self(v)->x); }
	NB_PROP_GET(y)						{ return push(v, self(v)->y); }
	NB_PROP_GET(z)						{ return push(v, self(v)->z); }
	NB_PROP_GET(w)						{ return push(v, self(v)->w); }

	NB_PROP_GET(axisX)					{ return push(v, self(v)->xAxis()); }
	NB_PROP_GET(axisY)					{ return push(v, self(v)->yAxis()); }
	NB_PROP_GET(axisZ)					{ return push(v, self(v)->zAxis()); }

	NB_PROP_GET(roll)					{ return push(v, self(v)->getRoll().valueRadians()); }
	NB_PROP_GET(yaw)					{ return push(v, self(v)->getYaw().valueRadians()); }
	NB_PROP_GET(pitch)					{ return push(v, self(v)->getPitch().valueRadians()); }

	NB_PROP_GET(norm)					{ return push(v, self(v)->Norm()); }
	NB_PROP_GET(normalise)				{ return push(v, self(v)->normalise()); }
	NB_PROP_GET(inverse)				{ return push(v, self(v)->Inverse()); }
	NB_PROP_GET(unitInverse)			{ return push(v, self(v)->UnitInverse()); }
	NB_PROP_GET(exp)					{ return push(v, self(v)->Exp()); }
	NB_PROP_GET(log)					{ return push(v, self(v)->Log()); }

	NB_FUNC(swap)						{ self(v)->swap(*get<Quat>(v, 2)); return 0; }
	NB_FUNC(dot)						{ return push(v, self(v)->Dot(*get<Quat>(v, 2))); }
	NB_FUNC(isNan)						{ return push(v, self(v)->isNaN()); }
	NB_FUNC(equals)						{ return push(v, self(v)->equals(*get<Quat>(v, 2), Radian(getFloat(v, 3)))); }

	NB_FUNC(fromRotationMatrix)			{ self(v)->FromRotationMatrix(*get<Matrix3>(v, 2)); return 0; }
	NB_FUNC(toRotationMatrix)			{ Matrix3 m; self(v)->ToRotationMatrix(m); return push(v, m); }
	NB_FUNC(fromAngleAxis)				{ self(v)->FromAngleAxis(Radian(getFloat(v, 2)), *get<Vector3>(v, 3)); return 0; }

	NB_FUNC(toAngleAxis)
	{
		Radian angle;
		Vector3 axis;

		self(v)->ToAngleAxis(angle, axis);

		sq_newarray(v, 0);
		arrayAppend(v, -1, angle.valueRadians());
		arrayAppend(v, -1, axis);

		return 1;
	}

	NB_FUNC(fromAxes)					{ self(v)->FromAxes(*get<Vector3>(v, 2), *get<Vector3>(v, 3), *get<Vector3>(v, 4)); return 0; }	
	NB_FUNC(toAxes)
	{
		Vector3 x, y, z;
		self(v)->ToAxes(x, y, z);

		sq_newarray(v, 0);
		arrayAppend(v, -1, x);
		arrayAppend(v, -1, y);
		arrayAppend(v, -1, z);

		return 1;
	}

	NB_FUNC(slerp)						{ return push(v, Quat::Slerp(getFloat(v, 2), *get<Quat>(v, 3), *get<Quat>(v, 4), optBool(v, 5, false))); }
	NB_FUNC(slerpExtraSpins)			{ return push(v, Quat::SlerpExtraSpins(getFloat(v, 2), *get<Quat>(v, 3), *get<Quat>(v, 4), getInt(v, 5))); }
	NB_FUNC(nlerp)						{ return push(v, Quat::nlerp(getFloat(v, 2), *get<Quat>(v, 3), *get<Quat>(v, 4), optBool(v, 5, false))); }

	NB_FUNC(intermediate)
	{
		Quat::Intermediate(
			*get<Quat>(v, 2), *get<Quat>(v, 3), *get<Quat>(v, 4), // q0, q1, q2
			*get<Quat>(v, 5), *get<Quat>(v, 6) ); // out a, out b
		return 0;
	}

	NB_FUNC(squad)
	{
		return push(v, Quat::Squad(
			getFloat(v, 2), // t
			*get<Quat>(v, 3), *get<Quat>(v, 4), *get<Quat>(v, 5), *get<Quat>(v, 6), // p, a, b, q
			optBool(v, 7, false) // shortest
			) ); 
	}

	NB_FUNC(shortestArc)				{ return push(v, Math::shortestArc(*get<Vector3>(v, 2), *get<Vector3>(v, 3))); }
};


////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::Color, NULL);

class NB_Color : TNitClass<Color>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(r),
			PROP_ENTRY	(g),
			PROP_ENTRY	(b),
			PROP_ENTRY	(a),

			PROP_ENTRY	(rgb),
			PROP_ENTRY	(rgba),
			PROP_ENTRY	(argb),

			NULL,
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(): Color // == Color(1,1,1,1)\n"
										"(r, g, b: float, a=0: float)\n"
										"(argb: int)"),

			FUNC_ENTRY	(_tostring),
			FUNC_ENTRY_H(saturate,		"() // changes this"),
			FUNC_ENTRY_H(saturateCopy,	"(): Color"),
			NULL,
		};

		bind(v, props, funcs);

		addStatic(v, "ZERO",			Color::ZERO);
		addStatic(v, "BLACK",			Color::Black);
		addStatic(v, "WHITE",			Color::White);
		addStatic(v, "RED",				Color::Red);
		addStatic(v, "GREEN",			Color::Green);
		addStatic(v, "BLUE",			Color::Blue);
	}

	NB_CONS()
	{
		int nargs = sq_gettop(v) - 1;
		if (nargs == 0)
		{
			new (self(v)) Color();
		}
		else if (nargs == 1)
		{
			self(v)->setAsARGB(ARGB(getInt(v, 2)));
		}
		else
		{
			new (self(v)) Color(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4), optFloat(v, 5, 0.0f));
		}

		return 0;
	}

	NB_PROP_GET(r)						{ return push(v, self(v)->r); }
	NB_PROP_GET(g)						{ return push(v, self(v)->g); }
	NB_PROP_GET(b)						{ return push(v, self(v)->b); }
	NB_PROP_GET(a)						{ return push(v, self(v)->a); }
	NB_PROP_GET(rgba)					{ return push(v, int(self(v)->getAsRGBA())); }
	NB_PROP_GET(argb)					{ return push(v, int(self(v)->getAsARGB())); }
	NB_PROP_GET(rgb)					{ return push(v, self(v)->getAsARGB() & 0x00FFFFFF); }
	
	NB_PROP_SET(r)						{ self(v)->r = getFloat(v, 2); return 0; }
	NB_PROP_SET(g)						{ self(v)->g = getFloat(v, 2); return 0; }
	NB_PROP_SET(b)						{ self(v)->b = getFloat(v, 2); return 0; }
	NB_PROP_SET(a)						{ self(v)->a = getFloat(v, 2); return 0; }
	NB_PROP_SET(rgba)					{ self(v)->setAsRGBA(RGBA(getInt(v, 2))); return 0; }
	NB_PROP_SET(argb)					{ self(v)->setAsARGB(RGBA(getInt(v, 2))); return 0; }
	NB_PROP_SET(rgb)					{ type* o = self(v); o->setAsARGB((int(o->a * 255) << 24) | (getInt(v, 2) & 0x00FFFFFF)); return 0; }
	
	NB_FUNC(_tostring)					
	{ 
		Color* colour = self(v);
		pushFmt(v, "Color(%.3f, %.3f, %.3f, %.3f)", colour->r, colour->g, colour->b, colour->a);
		return 1;
	}

	NB_FUNC(saturate)					{ self(v)->saturate(); return 0; }
	NB_FUNC(saturateCopy)				{ return push(v, self(v)->saturateCopy()); }
};

////////////////////////////////////////////////////////////////////////////////

// TODO: Touch Below

NB_TYPE_VALUE(NIT_API, nit::Matrix4, NULL);

class NB_Matrix4 : TNitClass<Matrix4>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(affine),
			PROP_ENTRY_R(hasScale),
			PROP_ENTRY_R(hasNegativeScale),

			PROP_ENTRY	(trans),
			PROP_ENTRY	(scale),
			NULL,
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY (),

			FUNC_ENTRY	(_add),
			FUNC_ENTRY	(_sub),
			FUNC_ENTRY	(_mul),

			FUNC_ENTRY	(swap),
			FUNC_ENTRY	(transpose),
			FUNC_ENTRY	(makeTrans),
			FUNC_ENTRY	(extract3x3Matrix),
			FUNC_ENTRY	(extractQuat),
			FUNC_ENTRY	(adjoint),
			FUNC_ENTRY	(determinant),
			FUNC_ENTRY	(inverse),
			FUNC_ENTRY	(makeTransform),
			FUNC_ENTRY	(makeInverseTransform),
			FUNC_ENTRY	(inverseAffine),
			FUNC_ENTRY	(concatenateAffine),
			FUNC_ENTRY	(transformAffine),
			FUNC_ENTRY_H(getAt, "(row, col)"),
			FUNC_ENTRY_H(setAt, "(row, col, value)"),
			NULL,
		};

		bind(v, props, funcs);

		addStatic(v, "ZERO",			Matrix4::ZERO);
		addStatic(v, "IDENTITY",		Matrix4::IDENTITY);
		addStatic(v, "CLIPSPACE2DTOIMAGESPACE", Matrix4::CLIPSPACE2DTOIMAGESPACE);
	}

	NB_PROP_GET(affine)					{ return push(v, self(v)->isAffine()); }
	NB_PROP_GET(hasScale)				{ return push(v, self(v)->hasScale()); }
	NB_PROP_GET(hasNegativeScale)		{ return push(v, self(v)->hasNegativeScale()); }

	NB_PROP_GET(trans)					{ return push(v, self(v)->getTrans()); }
	NB_PROP_SET(trans)					{ self(v)->setTrans(*get<Vector3>(v, 2)); return 0; }

	NB_PROP_GET(scale)					{ Matrix4* m = self(v); return push(v, Vector3((*m)[0][0], (*m)[1][1], (*m)[2][2])); }
	NB_PROP_SET(scale)					{ self(v)->setScale(*get<Vector3>(v, 2)); return 0; }

	NB_CONS()
	{
		if (is<Matrix3>(v, 2))
		{
			new (self(v)) Matrix4(*get<Matrix3>(v, 2));
		}
		else if (is<Quat>(v, 2))
		{
			new (self(v)) Matrix4(*get<Quat>(v, 2));
		}
		else if (sq_gettop(v) == 17)
		{
			new (self(v)) Matrix4(
				getFloat(v, 2), getFloat(v, 3), getFloat(v, 4), getFloat(v, 5),
				getFloat(v, 6), getFloat(v, 7), getFloat(v, 8), getFloat(v, 9),
				getFloat(v, 10), getFloat(v, 11), getFloat(v, 12), getFloat(v, 13),
				getFloat(v, 14), getFloat(v, 15), getFloat(v, 16), getFloat(v, 17));
		}
		else
		{
			new (self(v)) Matrix4();
		}

		return 0;
	}	

	NB_FUNC(_add)						{ return push(v, *self(v) + *get<Matrix4>(v, 2)); }
	NB_FUNC(_sub)						{ return push(v, *self(v) - *get<Matrix4>(v, 2)); }
	NB_FUNC(_mul)				
	{
		if (is<Vector3>(v, 2))
			return push(v, *self(v) * *get<Vector3>(v, 2));
		else if (is<Vector4>(v, 2))
			return push(v, *self(v) * *get<Vector4>(v, 2));
		else if (is<Matrix4>(v, 2))
			return push(v, *self(v) * *get<Matrix4>(v, 2));
		else if (is<Plane>(v, 2))
			return push(v, *self(v) * *get<Plane>(v, 2));
		else
			return sq_throwerror(v, "Matrix4 _mul arg type error");

		return 0;
	}

	NB_FUNC(swap)						{ self(v)->swap(*get<Matrix4>(v, 2)); return 0; }
	NB_FUNC(transpose)					{ return push(v, self(v)->transpose()); }
	NB_FUNC(makeTrans)					{ self(v)->makeTrans(*get<Vector3>(v, 2)); return 0; }
	NB_FUNC(extract3x3Matrix)			{ self(v)->extract3x3Matrix(*get<Matrix3>(v, 2)); return 0; }
	NB_FUNC(extractQuat)				{ return push(v, self(v)->extractQuaternion()); }
	NB_FUNC(adjoint)					{ return push(v, self(v)->adjoint()); }
	NB_FUNC(determinant)				{ return push(v, self(v)->determinant()); }
	NB_FUNC(inverse)					{ return push(v, self(v)->inverse()); }
	NB_FUNC(makeTransform)				{ self(v)->makeTransform(*get<Vector3>(v, 2), *get<Vector3>(v, 3), *get<Quat>(v, 4)); return 0; }
	NB_FUNC(makeInverseTransform)		{ self(v)->makeTransform(*get<Vector3>(v, 2), *get<Vector3>(v, 3), *get<Quat>(v, 4)); return 0; }
	NB_FUNC(inverseAffine)				{ return push(v, self(v)->inverseAffine()); }
	NB_FUNC(concatenateAffine)			{ return push(v, self(v)->concatenateAffine(*get<Matrix4>(v, 2))); }	

	NB_FUNC(transformAffine)		
	{ 
		if (is<Vector3>(v, 2))
		{
			return push(v, self(v)->transformAffine(*get<Vector3>(v, 2)));
		}
		else if (is<Vector4>(v, 2))
		{
			return push(v, self(v)->transformAffine(*get<Vector4>(v, 2)));
		}
		else
			return sq_throwerror(v, "TransformAffine arg type error");
	}

	NB_FUNC(getAt)
	{
		int row = getInt(v, 2);
		int col = getInt(v, 3);
		if (row > 3 || row < 0 || col > 3 || col < 0)
			return push(v, NULL);
		
		return push(v, *(self(v)[row][col]));
	}

	NB_FUNC(setAt)
	{
		int row = getInt(v, 2);
		int col = getInt(v, 3);
		if (row > 3 || row < 0 || col > 3 || col < 0)
			return 0;

		*(self(v)[row][col]) = getFloat(v, 4);

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::AxisAlignedBox, NULL);

class NB_AxisAlignedBox : TNitClass<AxisAlignedBox>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(minimum),
			PROP_ENTRY	(maximum),
			PROP_ENTRY_R(volume),
			PROP_ENTRY_R(center),
			PROP_ENTRY_R(size),
			PROP_ENTRY_R(halfSize),
			NULL,
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(min, max: Vector3)"
			"\n"							"(minX, minY, minZ, maxX, maxY, maxZ: float)"),

			FUNC_ENTRY_H(isNull,			"(): bool"),
			FUNC_ENTRY_H(isInfinite,		"(): bool"),
			FUNC_ENTRY_H(isFinite,			"(): bool"),
			FUNC_ENTRY_H(merge,				"(point: Vector3)"),
			FUNC_ENTRY_H(transform,			"(mat: Matrix4)"),
			FUNC_ENTRY_H(transformAffine,	"(mat: Matrix4)"),
			FUNC_ENTRY_H(intersects,		"(other: AxisAlignedBox): bool"),
			FUNC_ENTRY_H(intersection,		"(other: AxisAlignedBox): AxisAlignedBox"),
			FUNC_ENTRY_H(contains,			"(point: Vector3): bool\n(other: AxisAlignedBox): bool"),

			FUNC_ENTRY	(_tostring),
			NULL,
		};

		bind(v, props, funcs);

		addStatic(v, "INFINITE",		AxisAlignedBox::BOX_INFINITE);
		addStatic(v, "NULL",			AxisAlignedBox::BOX_NULL);
	}

	NB_PROP_GET(minimum)				{ return push(v, self(v)->getMinimum()); }
	NB_PROP_GET(maximum)				{ return push(v, self(v)->getMaximum()); }
	NB_PROP_GET(volume)					{ return push(v, self(v)->volume()); }
	NB_PROP_GET(center)					{ return push(v, self(v)->getCenter()); }
	NB_PROP_GET(size)					{ return push(v, self(v)->getSize()); }
	NB_PROP_GET(halfSize)				{ return push(v, self(v)->getHalfSize()); }

	NB_PROP_SET(minimum)				{ self(v)->setMinimum(*get<Vector3>(v, 2)); return 0; }
	NB_PROP_SET(maximum)				{ self(v)->setMaximum(*get<Vector3>(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNumber(v, 2))
			new (self(v)) AxisAlignedBox(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4), getFloat(v, 5), getFloat(v, 6), getFloat(v, 7));
		else
			new (self(v)) AxisAlignedBox(*get<Vector3>(v, 2), *get<Vector3>(v, 3));
		return 0;
	}

	NB_FUNC(isNull)						{ return push(v, self(v)->isNull()); }
	NB_FUNC(isInfinite)					{ return push(v, self(v)->isInfinite()); }
	NB_FUNC(isFinite)					{ return push(v, self(v)->isFinite()); }

	NB_FUNC(merge)						{ self(v)->merge(*get<Vector3>(v, 2)); return 0; }
	NB_FUNC(transform)					{ self(v)->transform(*get<Matrix4>(v, 2)); return 0; }
	NB_FUNC(transformAffine)			{ self(v)->transformAffine(*get<Matrix4>(v, 2)); return 0; }
	NB_FUNC(intersects)					{ return push(v, self(v)->intersects(*get<AxisAlignedBox>(v, 2))); return 0; }
	NB_FUNC(intersection)				{ return push(v, self(v)->intersection(*get<AxisAlignedBox>(v, 2))); return 0; }

	NB_FUNC(contains)
	{ 
		if (is<Vector3>(v, 2))
			return push(v, self(v)->contains(*get<Vector3>(v, 2)));
		else
			return push(v, self(v)->contains(*get<AxisAlignedBox>(v, 2)));
	}

	NB_FUNC(_tostring)
	{
		AxisAlignedBox* aabb = self(v);

		if (aabb->isInfinite())
			push(v, "AxisAlignedBox.INFINITE");
		else if (aabb->isNull())
			push(v, "AxisAlignedBox.NULL");
		else
		{
			const Vector3& p0 = aabb->getMinimum();
			const Vector3& p1 = aabb->getMaximum();
			pushFmt(v, "AxisAlignedBox(%.3f, %.3f, %.3f, %.3f, %.3f, %.3f)", 
				p0.x, p0.y, p0.z,
				p1.x, p1.y, p1.z);
		}
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::Sphere, NULL);

class NB_Sphere : TNitClass<Sphere>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL,
		};

		FuncEntry funcs[] =
		{
			NULL,
		};

		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::Plane, NULL);

class NB_Plane : TNitClass<Plane>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(normal),
			PROP_ENTRY	(d),
			NULL,
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY(),
			FUNC_ENTRY	(getDistance),
			FUNC_ENTRY	(getSide),
			FUNC_ENTRY	(redefine),
			FUNC_ENTRY	(projectVector),
			FUNC_ENTRY	(normalise),
			NULL,
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(normal)					{ return push(v, self(v)->normal); }
	NB_PROP_SET(normal)					{ self(v)->normal = getFloat(v, 2); return 0; }

	NB_PROP_GET(d)						{ return push(v, self(v)->d); }
	NB_PROP_SET(d)						{ self(v)->d = getFloat(v, 2); return 0; }

	NB_CONS()
	{
		if (is<Plane>(v, 2))
		{
			new (self(v)) Plane(*get<Plane>(v, 2));
		}
		else if (is<Vector3>(v, 2) && isFloat(v, 3))
		{
			new (self(v)) Plane(*get<Vector3>(v, 2), getFloat(v, 3));
		}
		else if (isFloat(v, 2) && isFloat(v, 3) && isFloat(v, 4) && isFloat(v, 5))
		{
			new (self(v)) Plane(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4), getFloat(v, 5));
		}
		else if (is<Vector3>(v, 2) && is<Vector3>(v, 3) && is<Vector3>(v, 4))
		{
			new (self(v)) Plane(*get<Vector3>(v, 2), *get<Vector3>(v, 3), *get<Vector3>(v, 4));
		}
		else if (is<Vector3>(v, 2) && is<Vector3>(v, 3))
		{
			new (self(v)) Plane(*get<Vector3>(v, 2), *get<Vector3>(v, 3));
		}
		else
		{
			new (self(v)) Plane();
		}

		return 0;
	}

	NB_FUNC(getDistance)				{ return push(v, self(v)->getDistance(*get<Vector3>(v, 2))); }

	NB_FUNC(getSide)
	{
		Plane::Side s;
		if (is<Vector3>(v, 2) && is<Vector3>(v, 3))
			s = self(v)->getSide(*get<Vector3>(v, 2), *get<Vector3>(v, 3));
		else if (is<AxisAlignedBox>(v, 2))
			s = self(v)->getSide(*get<Vector3>(v, 2), *get<Vector3>(v, 3));
		else if (is<Vector3>(v, 2))
			s = self(v)->getSide(*get<Vector3>(v, 2));
		else
			return sq_throwerror(v, "GetSide arg type error");
		
		return push(v, int(s));
	}

	NB_FUNC(redefine)
	{
		if (is<Vector3>(v, 2) && is<Vector3>(v, 3) && is<Vector3>(v, 4))
			self(v)->redefine(*get<Vector3>(v, 2), *get<Vector3>(v, 3), *get<Vector3>(v, 4));
		else if (is<Vector3>(v, 2) && is<Vector3>(v, 3))
			self(v)->redefine(*get<Vector3>(v, 2), *get<Vector3>(v, 3));
		else
			return sq_throwerror(v, "Redefine arg type error");

		return 0;
	}

	NB_FUNC(projectVector)				{ return push(v, self(v)->projectVector(*get<Vector3>(v, 2))); }
	NB_FUNC(normalise)					{ return push(v, self(v)->normalise()); }
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibMath(HSQUIRRELVM v)
{
	NB_Math::Register(v);

	NB_Vector2::Register(v);
	NB_Vector3::Register(v);
	NB_Vector4::Register(v);
	
	NB_Matrix3::Register(v);
	NB_Matrix4::Register(v);
	NB_Quat::Register(v);

	NB_Color::Register(v);
	NB_AxisAlignedBox::Register(v);
	NB_Sphere::Register(v);
	NB_Plane::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
