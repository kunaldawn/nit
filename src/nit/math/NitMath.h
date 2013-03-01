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

#include <math.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class Degree;
class Radian;
class Vector2;
class Vector3;
class Vector4;
class Matrix3;
class Matrix4;
class Ray;
class Plane;
class Sphere;
class AxisAlignedBox;
class Quaternion;

typedef Quaternion Quat;

////////////////////////////////////////////////////////////////////////////////

class NIT_API MathBase
{
public:									// Basic constants
	static const float					PI;				// PI = 3.14159...
	static const float					HALF_PI;		// PI / 2
	static const float					TWO_PI;			// 2 * PI
	static const float					E;				// E = 2.718...
	static const float					EPSILON;		// smallest float value which satisfies : (x < x + EPSILON)
	static const float					MAX_FLOAT;		// Positive Infinity
	static const float					MIN_FLOAT;		// Negative Infinity
	static const float					SQRT_2;

public:									// Zero compare
	static bool							isZero(float d)										{ return -EPSILON < d && d < EPSILON; }
	static bool							nearEqual(float a, float b)							{ return isZero(a - b); }

public:									// Unit conversion
	static float						rad(float deg)										{ return deg * (PI / 180.0f); }
	static float						deg(float rad)										{ return rad * (180.0f / PI); }

public:									// Trigonometrical functions
	static float						sqrt(float d)										{ return sqrtf(d); }
	static float						cubicRoot(float d)									{ return (d == 0) ? 0.0f : (d > 0) ? powf(d, 1.0f / 3.0f) : -powf(-d, 1.0f / 3.0f); }

	static float						sin(float v)										{ return sinf(v); }
	static float						cos(float v)										{ return cosf(v); }
	static float						tan(float v)										{ return tanf(v); }

	static float						acos(float v)										{ return acosf(v); }
	static float						asin(float v)										{ return asinf(v); }
	static float						atan(float v)										{ return atanf(v); }
	static float						atan2(float y, float x)								{ return atan2f(y, x); }

public:									// Exponencial functions
	static float						pow(float f, float p)								{ return powf(f, p); }
	static float						exp(float p)										{ return expf(p); }
	static float						log(float f)										{ return log(f, E); }
	static float						log(float f, float p)								{ return logf(f) / logf(p); }
	static float						log10(float f)										{ return logf(f); }

public:									// Float-Integer conversions
	static float						ceil(float f)										{ return ceilf(f); }
	static float						floor(float f)										{ return floorf(f); }
	static float						round(float f)										{ return floorf(f + 0.5f); }
	static int							ceilToInt(float f)									{ return (int)ceilf(f); }
	static int							floorToInt(float f)									{ return (int)floorf(f); }
	static int							roundToInt(float f)									{ return (int)floorf(f + 0.5f); }

	template <typename T>
	static inline const T				sign(const T& v)									{ return v > T(0) ? T(1) : v < T(0) ? T(-1) : T(0); }

	template <typename T>
	static inline const T				abs(const T& v)										{ return v > T(0) ? v : -v; }

	template <typename T>
	static inline const T&				max(const T& a, const T& b)							{ return a > b ? a : b; }

	template <typename T>
	static inline const T&				min(const T& a, const T& b)							{ return a < b ? a : b; }

	template <typename T>
	static inline const T&				clamp(const T& value, const T& min, const T& max)	{ return value < min ? min : value > max ? max : value; }

	template <typename T>
	static inline const T&				clamp01(const T& value)								{ return clamp(value, T(0), T(1)); }

	template <typename T>
	static inline T						wrap(const T& value, const T& min, const T& max)	{ float l = max - min; return fmod(value - min, l) + min + (value < min ? l : 0); }

public:									// Interpolation functions
	template <typename T>
	static inline T						lerp(float t, const T& a, const T& b)				{ return T(a + t * (b - a)); }

	template <typename T>
	static inline T						easeInOut(float t, const T& a, const T& b);

	template <typename V>
	static inline void					bezier(const V& v0, const V& v1, const V& v2, const V& v3, float t, V* outPos, V* outTan = NULL, V* outAcc = NULL);

	template <typename V>
	static inline void					hermite(const V& p0, const V& p1, const V& t0, const V& t1, float t, V* outPos, V* outTan = NULL, V* outAcc = NULL);

public:									// Vector functions
	template <typename V>
	static inline V						proj(const V& d, const V& n);

	template <typename V>
	static inline V						perp(const V& d, const V& n);

	template <typename V>
	static inline V						reflect(const V& d, const V& n);

	static inline Vector3				quatRotate(const Quat& r, const Vector3& v);
	static inline Quat					shortestArc(Vector3 d0, Vector3 d1);

	static Vector3						front		(const Matrix3& m);
	static Vector3						right		(const Matrix3& m);
	static Vector3						up			(const Matrix3& m);

	static Matrix3						upFront		(const Vector3& up, Vector3 front);
	static Matrix3						upRight		(const Vector3& up, Vector3 right);
	static Matrix3						frontUp		(const Vector3& front, Vector3 up);
	static Matrix3						frontRight	(const Vector3& front, Vector3 right);
	static Matrix3						rightUp		(const Vector3& right, Vector3 up);
	static Matrix3						rightFront	(const Vector3& right, Vector3 front);

	static Matrix3						pinUp		(const Vector3& dir);
	static Matrix3						pinFront	(const Vector3& dir);
	static Matrix3						pinRight	(const Vector3& dir);
};

////////////////////////////////////////////////////////////////////////////////

// from ogre 1.7.1 OgreMath.h

/*
-----------------------------------------------------------------------------
This source file is from part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

/** Wrapper class which indicates a given angle value is in Radians.
@remarks
Radian values are interchangeable with Degree values, and conversions
will be done automatically between them.
*/

class Radian
{
	Real mRad;

public:
	explicit Radian ( Real r=0 ) : mRad(r) {}
	Radian ( const Degree& d );
	Radian& operator = ( const Real& f ) { mRad = f; return *this; }
	Radian& operator = ( const Radian& r ) { mRad = r.mRad; return *this; }
	Radian& operator = ( const Degree& d );

	Real valueDegrees() const; // see bottom of this file
	Real valueRadians() const { return mRad; }
	Real valueAngleUnits() const;

    const Radian& operator + () const { return *this; }
	Radian operator + ( const Radian& r ) const { return Radian ( mRad + r.mRad ); }
	Radian operator + ( const Degree& d ) const;
	Radian& operator += ( const Radian& r ) { mRad += r.mRad; return *this; }
	Radian& operator += ( const Degree& d );
	Radian operator - () const { return Radian(-mRad); }
	Radian operator - ( const Radian& r ) const { return Radian ( mRad - r.mRad ); }
	Radian operator - ( const Degree& d ) const;
	Radian& operator -= ( const Radian& r ) { mRad -= r.mRad; return *this; }
	Radian& operator -= ( const Degree& d );
	Radian operator * ( Real f ) const { return Radian ( mRad * f ); }
    Radian operator * ( const Radian& f ) const { return Radian ( mRad * f.mRad ); }
	Radian& operator *= ( Real f ) { mRad *= f; return *this; }
	Radian operator / ( Real f ) const { return Radian ( mRad / f ); }
	Radian& operator /= ( Real f ) { mRad /= f; return *this; }

	bool operator <  ( const Radian& r ) const { return mRad <  r.mRad; }
	bool operator <= ( const Radian& r ) const { return mRad <= r.mRad; }
	bool operator == ( const Radian& r ) const { return mRad == r.mRad; }
	bool operator != ( const Radian& r ) const { return mRad != r.mRad; }
	bool operator >= ( const Radian& r ) const { return mRad >= r.mRad; }
	bool operator >  ( const Radian& r ) const { return mRad >  r.mRad; }
};

/** Wrapper class which indicates a given angle value is in Degrees.
@remarks
    Degree values are interchangeable with Radian values, and conversions
    will be done automatically between them.
*/
class Degree
{
	Real mDeg; // if you get an error here - make sure to define/typedef 'Real' first

public:
	explicit Degree ( Real d=0 ) : mDeg(d) {}
	Degree ( const Radian& r ) : mDeg(r.valueDegrees()) {}
	Degree& operator = ( const Real& f ) { mDeg = f; return *this; }
	Degree& operator = ( const Degree& d ) { mDeg = d.mDeg; return *this; }
	Degree& operator = ( const Radian& r ) { mDeg = r.valueDegrees(); return *this; }

	Real valueDegrees() const { return mDeg; }
	Real valueRadians() const; // see bottom of this file
	Real valueAngleUnits() const;

	const Degree& operator + () const { return *this; }
	Degree operator + ( const Degree& d ) const { return Degree ( mDeg + d.mDeg ); }
	Degree operator + ( const Radian& r ) const { return Degree ( mDeg + r.valueDegrees() ); }
	Degree& operator += ( const Degree& d ) { mDeg += d.mDeg; return *this; }
	Degree& operator += ( const Radian& r ) { mDeg += r.valueDegrees(); return *this; }
	Degree operator - () const { return Degree(-mDeg); }
	Degree operator - ( const Degree& d ) const { return Degree ( mDeg - d.mDeg ); }
	Degree operator - ( const Radian& r ) const { return Degree ( mDeg - r.valueDegrees() ); }
	Degree& operator -= ( const Degree& d ) { mDeg -= d.mDeg; return *this; }
	Degree& operator -= ( const Radian& r ) { mDeg -= r.valueDegrees(); return *this; }
	Degree operator * ( Real f ) const { return Degree ( mDeg * f ); }
    Degree operator * ( const Degree& f ) const { return Degree ( mDeg * f.mDeg ); }
	Degree& operator *= ( Real f ) { mDeg *= f; return *this; }
	Degree operator / ( Real f ) const { return Degree ( mDeg / f ); }
	Degree& operator /= ( Real f ) { mDeg /= f; return *this; }

	bool operator <  ( const Degree& d ) const { return mDeg <  d.mDeg; }
	bool operator <= ( const Degree& d ) const { return mDeg <= d.mDeg; }
	bool operator == ( const Degree& d ) const { return mDeg == d.mDeg; }
	bool operator != ( const Degree& d ) const { return mDeg != d.mDeg; }
	bool operator >= ( const Degree& d ) const { return mDeg >= d.mDeg; }
	bool operator >  ( const Degree& d ) const { return mDeg >  d.mDeg; }
};

/** Wrapper class which identifies a value as the currently default angle 
    type, as defined by Math::setAngleUnit.
@remarks
    Angle values will be automatically converted between radians and degrees,
    as appropriate.
*/
class Angle
{
	Real mAngle;
public:
	explicit Angle ( Real angle ) : mAngle(angle) {}
	operator Radian() const;
	operator Degree() const;
};

// these functions could not be defined within the class definition of class
// Radian because they required class Degree to be defined
inline Radian::Radian ( const Degree& d ) : mRad(d.valueRadians()) {
}
inline Radian& Radian::operator = ( const Degree& d ) {
	mRad = d.valueRadians(); return *this;
}
inline Radian Radian::operator + ( const Degree& d ) const {
	return Radian ( mRad + d.valueRadians() );
}
inline Radian& Radian::operator += ( const Degree& d ) {
	mRad += d.valueRadians();
	return *this;
}
inline Radian Radian::operator - ( const Degree& d ) const {
	return Radian ( mRad - d.valueRadians() );
}
inline Radian& Radian::operator -= ( const Degree& d ) {
	mRad -= d.valueRadians();
	return *this;
}

/** Class to provide access to common mathematical functions.
    @remarks
        Most of the maths functions are aliased versions of the C runtime
        library functions. They are aliased here to provide future
        optimisation opportunities, either from faster RTLs or custom
        math approximations.
    @note
        <br>This is based on MgcMath.h from
        <a href="http://www.geometrictools.com/">Wild Magic</a>.
*/
class NIT_API Math : public MathBase
{
public:
   /** The angular units used by the API. This functionality is now deprecated in favor
       of discreet angular unit types ( see Degree and Radian above ). The only place
	   this functionality is actually still used is when parsing files. Search for
	   usage of the Angle class for those instances
   */
   enum AngleUnit
   {
       AU_DEGREE,
       AU_RADIAN
   };

protected:
   // angle units used by the api
   static AngleUnit msAngleUnit;

    /// Size of the trig tables as determined by constructor.
    static int mTrigTableSize;

    /// Radian -> index factor value ( mTrigTableSize / 2 * PI )
    static Real mTrigTableFactor;
    static Real* mSinTable;
    static Real* mTanTable;

    /** Private function to build trig tables.
    */
    void buildTrigTables();

	static Real SinTable (Real fValue);
	static Real TanTable (Real fValue);
public:
    /** Default constructor.
        @param
            trigTableSize Optional parameter to set the size of the
            tables used to implement Sin, Cos, Tan
    */
    Math(unsigned int trigTableSize = 4096);

    /** Default destructor.
    */
    ~Math();

	static inline int IAbs (int iValue) { return ( iValue >= 0 ? iValue : -iValue ); }
	static inline int ICeil (float fValue) { return int(ceil(fValue)); }
	static inline int IFloor (float fValue) { return int(floor(fValue)); }
    static int ISign (int iValue);

	static inline Real abs (Real fValue) { return Real(fabs(fValue)); }
	static inline Degree abs (const Degree& dValue) { return Degree(fabs(dValue.valueDegrees())); }
	static inline Radian abs (const Radian& rValue) { return Radian(fabs(rValue.valueRadians())); }
	static Radian ACos (Real fValue);
	static Radian ASin (Real fValue);
	static inline Radian ATan (Real fValue) { return Radian(atan(fValue)); }
	static inline Radian ATan2 (Real fY, Real fX) { return Radian(atan2(fY,fX)); }
	static inline Real ceil (Real fValue) { return Real(::ceil(fValue)); }
	static inline bool isNaN(Real f)
	{
		// std::isnan() is C99, not supported by all compilers
		// However NaN always fails this next test, no other number does.
		return f != f;
	}

    /** Cosine function.
        @param
            fValue Angle in radians
        @param
            useTables If true, uses lookup tables rather than
            calculation - faster but less accurate.
    */
    static inline Real cos (const Radian& fValue, bool useTables = false) {
		return (!useTables) ? Real(::cos(fValue.valueRadians())) : SinTable(fValue.valueRadians() + HALF_PI);
	}
    /** Cosine function.
        @param
            fValue Angle in radians
        @param
            useTables If true, uses lookup tables rather than
            calculation - faster but less accurate.
    */
    static inline Real cos (Real fValue, bool useTables = false) {
		return (!useTables) ? Real(::cos(fValue)) : SinTable(fValue + HALF_PI);
	}

	static inline Real exp (Real fValue) { return Real(::exp(fValue)); }

	static inline Real floor (Real fValue) { return Real(::floor(fValue)); }

	static inline Real log (Real fValue) { return Real(::log(fValue)); }

	/// Stored value of log(2) for frequent use
	static const Real LOG2;

	static inline Real Log2 (Real fValue) { return Real(::log(fValue)/LOG2); }

	static inline Real LogN (Real base, Real fValue) { return Real(::log(fValue)/::log(base)); }

	static inline Real pow (Real fBase, Real fExponent) { return Real(::pow(fBase,fExponent)); }

    static Real sign (Real fValue);
	static inline Radian sign ( const Radian& rValue )
	{
		return Radian(sign(rValue.valueRadians()));
	}
	static inline Degree sign ( const Degree& dValue )
	{
		return Degree(sign(dValue.valueDegrees()));
	}

    /** Sine function.
        @param
            fValue Angle in radians
        @param
            useTables If true, uses lookup tables rather than
            calculation - faster but less accurate.
    */
    static inline Real sin (const Radian& fValue, bool useTables = false) {
		return (!useTables) ? Real(::sin(fValue.valueRadians())) : SinTable(fValue.valueRadians());
	}
    /** Sine function.
        @param
            fValue Angle in radians
        @param
            useTables If true, uses lookup tables rather than
            calculation - faster but less accurate.
    */
    static inline Real sin (Real fValue, bool useTables = false) {
		return (!useTables) ? Real(::sin(fValue)) : SinTable(fValue);
	}

	static inline Real Sqr (Real fValue) { return fValue*fValue; }

	static inline Real sqrt (Real fValue) { return Real(::sqrt(fValue)); }

	static inline Radian sqrt (const Radian& fValue) { return Radian(::sqrt(fValue.valueRadians())); }

	static inline Degree sqrt (const Degree& fValue) { return Degree(::sqrt(fValue.valueDegrees())); }

    /** Inverse square root i.e. 1 / Sqrt(x), good for vector
        normalisation.
    */
	static Real InvSqrt(Real fValue);

    static Real UnitRandom ();  // in [0,1]

    static Real RangeRandom (Real fLow, Real fHigh);  // in [fLow,fHigh]

    static Real SymmetricRandom ();  // in [-1,1]

    /** Tangent function.
        @param
            fValue Angle in radians
        @param
            useTables If true, uses lookup tables rather than
            calculation - faster but less accurate.
    */
	static inline Real tan (const Radian& fValue, bool useTables = false) {
		return (!useTables) ? Real(::tan(fValue.valueRadians())) : TanTable(fValue.valueRadians());
	}
    /** Tangent function.
        @param
            fValue Angle in radians
        @param
            useTables If true, uses lookup tables rather than
            calculation - faster but less accurate.
    */
	static inline Real tan (Real fValue, bool useTables = false) {
		return (!useTables) ? Real(tan(fValue)) : TanTable(fValue);
	}

	static inline Real DegreesToRadians(Real degrees) { return degrees * fDeg2Rad; }
    static inline Real RadiansToDegrees(Real radians) { return radians * fRad2Deg; }

   /** These functions used to set the assumed angle units (radians or degrees) 
        expected when using the Angle type.
   @par
        You can set this directly after creating a new Root, and also before/after resource creation,
        depending on whether you want the change to affect resource files.
   */
   static void setAngleUnit(AngleUnit unit);
   /** Get the unit being used for angles. */
   static AngleUnit getAngleUnit(void);

   /** Convert from the current AngleUnit to radians. */
   static Real AngleUnitsToRadians(Real units);
   /** Convert from radians to the current AngleUnit . */
   static Real RadiansToAngleUnits(Real radians);
   /** Convert from the current AngleUnit to degrees. */
   static Real AngleUnitsToDegrees(Real units);
   /** Convert from degrees to the current AngleUnit. */
   static Real DegreesToAngleUnits(Real degrees);

   /** Checks whether a given point is inside a triangle, in a
        2-dimensional (Cartesian) space.
        @remarks
            The vertices of the triangle must be given in either
            trigonometrical (anticlockwise) or inverse trigonometrical
            (clockwise) order.
        @param
            p The point.
        @param
            a The triangle's first vertex.
        @param
            b The triangle's second vertex.
        @param
            c The triangle's third vertex.
        @returns
            If the point resides in the triangle, <b>true</b> is
            returned.
        @par
            If the point is outside the triangle, <b>false</b> is
            returned.
    */
    static bool pointInTri2D(const Vector2& p, const Vector2& a, 
		const Vector2& b, const Vector2& c);

   /** Checks whether a given 3D point is inside a triangle.
   @remarks
        The vertices of the triangle must be given in either
        trigonometrical (anticlockwise) or inverse trigonometrical
        (clockwise) order, and the point must be guaranteed to be in the
		same plane as the triangle
    @param
        p The point.
    @param
        a The triangle's first vertex.
    @param
        b The triangle's second vertex.
    @param
        c The triangle's third vertex.
	@param 
		normal The triangle plane's normal (passed in rather than calculated
			on demand since the caller may already have it)
    @returns
        If the point resides in the triangle, <b>true</b> is
        returned.
    @par
        If the point is outside the triangle, <b>false</b> is
        returned.
    */
    static bool pointInTri3D(const Vector3& p, const Vector3& a, 
		const Vector3& b, const Vector3& c, const Vector3& normal);
    /** Ray / plane intersection, returns boolean result and distance. */
    static std::pair<bool, Real> intersects(const Ray& ray, const Plane& plane);

    /** Ray / sphere intersection, returns boolean result and distance. */
    static std::pair<bool, Real> intersects(const Ray& ray, const Sphere& sphere, 
        bool discardInside = true);
    
    /** Ray / box intersection, returns boolean result and distance. */
    static std::pair<bool, Real> intersects(const Ray& ray, const AxisAlignedBox& box);

    /** Ray / box intersection, returns boolean result and two intersection distance.
    @param
        ray The ray.
    @param
        box The box.
    @param
        d1 A real pointer to retrieve the near intersection distance
            from the ray origin, maybe <b>null</b> which means don't care
            about the near intersection distance.
    @param
        d2 A real pointer to retrieve the far intersection distance
            from the ray origin, maybe <b>null</b> which means don't care
            about the far intersection distance.
    @returns
        If the ray is intersects the box, <b>true</b> is returned, and
        the near intersection distance is return by <i>d1</i>, the
        far intersection distance is return by <i>d2</i>. Guarantee
        <b>0</b> <= <i>d1</i> <= <i>d2</i>.
    @par
        If the ray isn't intersects the box, <b>false</b> is returned, and
        <i>d1</i> and <i>d2</i> is unmodified.
    */
    static bool intersects(const Ray& ray, const AxisAlignedBox& box,
        Real* d1, Real* d2);

    /** Ray / triangle intersection, returns boolean result and distance.
    @param
        ray The ray.
    @param
        a The triangle's first vertex.
    @param
        b The triangle's second vertex.
    @param
        c The triangle's third vertex.
	@param 
		normal The triangle plane's normal (passed in rather than calculated
			on demand since the caller may already have it), doesn't need
            normalised since we don't care.
    @param
        positiveSide Intersect with "positive side" of the triangle
    @param
        negativeSide Intersect with "negative side" of the triangle
    @returns
        If the ray is intersects the triangle, a pair of <b>true</b> and the
        distance between intersection point and ray origin returned.
    @par
        If the ray isn't intersects the triangle, a pair of <b>false</b> and
        <b>0</b> returned.
    */
    static std::pair<bool, Real> intersects(const Ray& ray, const Vector3& a,
        const Vector3& b, const Vector3& c, const Vector3& normal,
        bool positiveSide = true, bool negativeSide = true);

    /** Ray / triangle intersection, returns boolean result and distance.
    @param
        ray The ray.
    @param
        a The triangle's first vertex.
    @param
        b The triangle's second vertex.
    @param
        c The triangle's third vertex.
    @param
        positiveSide Intersect with "positive side" of the triangle
    @param
        negativeSide Intersect with "negative side" of the triangle
    @returns
        If the ray is intersects the triangle, a pair of <b>true</b> and the
        distance between intersection point and ray origin returned.
    @par
        If the ray isn't intersects the triangle, a pair of <b>false</b> and
        <b>0</b> returned.
    */
    static std::pair<bool, Real> intersects(const Ray& ray, const Vector3& a,
        const Vector3& b, const Vector3& c,
        bool positiveSide = true, bool negativeSide = true);

    /** Sphere / box intersection test. */
    static bool intersects(const Sphere& sphere, const AxisAlignedBox& box);

    /** Plane / box intersection test. */
    static bool intersects(const Plane& plane, const AxisAlignedBox& box);

    /** Ray / convex plane list intersection test. 
    @param ray The ray to test with
    @param plaeList List of planes which form a convex volume
    @param normalIsOutside Does the normal point outside the volume
    */
    static std::pair<bool, Real> intersects(
        const Ray& ray, const vector<Plane>::type& planeList, 
        bool normalIsOutside);
    /** Ray / convex plane list intersection test. 
    @param ray The ray to test with
    @param plaeList List of planes which form a convex volume
    @param normalIsOutside Does the normal point outside the volume
    */
    static std::pair<bool, Real> intersects(
        const Ray& ray, const list<Plane>::type& planeList, 
        bool normalIsOutside);

    /** Sphere / plane intersection test. 
    @remarks NB just do a plane.getDistance(sphere.getCenter()) for more detail!
    */
    static bool intersects(const Sphere& sphere, const Plane& plane);

    /** Compare 2 reals, using tolerance for inaccuracies.
    */
    static bool RealEqual(Real a, Real b,
        Real tolerance = std::numeric_limits<Real>::epsilon());

    /** Calculates the tangent space vector for a given set of positions / texture coords. */
    static Vector3 calculateTangentSpaceVector(
        const Vector3& position1, const Vector3& position2, const Vector3& position3,
        Real u1, Real v1, Real u2, Real v2, Real u3, Real v3);

    /** Build a reflection matrix for the passed in plane. */
    static Matrix4 buildReflectionMatrix(const Plane& p);
    /** Calculate a face normal, including the w component which is the offset from the origin. */
    static Vector4 calculateFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3);
    /** Calculate a face normal, no w-information. */
    static Vector3 calculateBasicFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3);
    /** Calculate a face normal without normalize, including the w component which is the offset from the origin. */
    static Vector4 calculateFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3);
    /** Calculate a face normal without normalize, no w-information. */
    static Vector3 calculateBasicFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3);

	/** Generates a value based on the Gaussian (normal) distribution function
		with the given offset and scale parameters.
	*/
	static Real gaussianDistribution(Real x, Real offset = 0.0f, Real scale = 1.0f);

	/** Clamp a value within an inclusive range. */
	template <typename T>
	static T clamp(T val, T minval, T maxval)
	{
		assert (minval <= maxval && "Invalid clamp range");
		return std::max(std::min(val, maxval), minval);
	}

	static Matrix4 makeViewMatrix(const Vector3& position, const Quaternion& orientation, 
		const Matrix4* reflectMatrix = 0);

	/** Get a bounding radius value from a bounding box. */
	static Real boundingRadiusFromAABB(const AxisAlignedBox& aabb);

    static const Real POS_INFINITY;
    static const Real NEG_INFINITY;
    static const Real PI;
    static const Real TWO_PI;
    static const Real HALF_PI;
	static const Real fDeg2Rad;
	static const Real fRad2Deg;
};

////////////////////////////////////////////////////////////////////////////////

// these functions must be defined down here, because they rely on the
// angle unit conversion functions in class Math:

inline Real Radian::valueDegrees() const
{
	return Math::RadiansToDegrees ( mRad );
}

inline Real Radian::valueAngleUnits() const
{
	return Math::RadiansToAngleUnits ( mRad );
}

inline Real Degree::valueRadians() const
{
	return Math::DegreesToRadians ( mDeg );
}

inline Real Degree::valueAngleUnits() const
{
	return Math::DegreesToAngleUnits ( mDeg );
}

inline Angle::operator Radian() const
{
	return Radian(Math::AngleUnitsToRadians(mAngle));
}

inline Angle::operator Degree() const
{
	return Degree(Math::AngleUnitsToDegrees(mAngle));
}

inline Radian operator * ( Real a, const Radian& b )
{
	return Radian ( a * b.valueRadians() );
}

inline Radian operator / ( Real a, const Radian& b )
{
	return Radian ( a / b.valueRadians() );
}

inline Degree operator * ( Real a, const Degree& b )
{
	return Degree ( a * b.valueDegrees() );
}

inline Degree operator / ( Real a, const Degree& b )
{
	return Degree ( a / b.valueDegrees() );
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;

#include "nit/math/NitMath.inl"