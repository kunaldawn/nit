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

#include "nit/math/Solver.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

int RootSolver::solveLinear(float c[2], float s[1])
{
	if (Math::isZero(c[1]))
		return 0;

	s[0] = -c[0] / c[1];
	return 1;
}

int RootSolver::solveQuadric(float c[3], float s[2])
{
	// normal form: x^2 + px + q = 0

	float p = c[1] / (2.0f * c[2]);
	float q = c[0] / c[2];

	float D = p * p - q;

	if (Math::isZero(D))
	{
		s[0] = -p;
		return 1;
	}
	else if (D > 0)
	{
		float sqrtD = Math::sqrt(D);

		s[0] = sqrtD - p;
		s[1] = -sqrtD - p;
		return 2;
	}
	else // D < 0
	{
		return 0;
	}
}

int RootSolver::solveCubic(float c[4], float s[3])
{
	// normal form: x^3 + Ax^2 + Bx + C = 0
	float A = c[2] / c[3];
	float B = c[1] / c[3];
	float C = c[0] / c[3];

	// substitute x = y - a/3 to eliminate quadric term:
	// x^3 + px + q = 0

	float sq_A = A * A;
	float p = 1.0f / 3.0f * (-1.0f / 3.0f * sq_A + B);
	float q = 1.0f / 2.0f * (2.0f / 27.0f * A * sq_A - 1.0f / 3.0f * A * B + C);

	// use Cardano's formula

	float cb_P = p * p * p;
	float D = q * q + cb_P;

	int num; // num of solutions

	if (Math::isZero(D))
	{
		if (Math::isZero(q))
		{
			// one triple solution
			s[0] = 0.0f;
			num = 1;
		}
		else
		{
			// one single and one double solution
			float u = Math::cubicRoot(-q);
			s[0] = 2.0f * u;
			s[1] = -u;
			num = 2;
		}
	}
	else if (D < 0)
	{
		// Casus irreducibilis: three real solutions
		float phi = 1.0f / 3.0f * Math::acos(-q / Math::sqrt(-cb_P));
		float t = 2.0f * Math::sqrt(-p);

		s[0] = t * Math::cos(phi);
		s[1] = -t * Math::cos(phi + Math::PI / 3);
		s[2] = -t * Math::cos(phi - Math::PI / 3);

		num = 3;
	}
	else 
	{
		// one real solution
		float sqrtD = Math::sqrt(D);
		float u = Math::cubicRoot(sqrtD - q);
		float v = -Math::cubicRoot(sqrtD + q);

		s[0] = u + v;
		num = 1;
	}

	// resubstitute
	float sub = 1.0f / 3.0f * A;

	for (int i = 0; i < num; ++i)
	{
		s[i] -= sub;
	}

	return num;
}

int RootSolver::solveQuartic(float c[5], float s[4])
{
	float  coeffs[4];
	float  z, u, v, sub;
	float  A, B, C, D;
	float  sq_A, p, q, r;
	int     i, num;

	/* normal form: x^4 + Ax^3 + Bx^2 + Cx + D = 0 */

	A = c[ 3 ] / c[ 4 ];
	B = c[ 2 ] / c[ 4 ];
	C = c[ 1 ] / c[ 4 ];
	D = c[ 0 ] / c[ 4 ];

	/*  substitute x = y - A/4 to eliminate cubic term:
	x^4 + px^2 + qx + r = 0 */

	sq_A = A * A;
	p = - 3.0f/8.0f * sq_A + B;
	q = 1.0f/8.0f * sq_A * A - 1.0f/2.0f * A * B + C;
	r = - 3.0f/256.0f*sq_A*sq_A + 1.0f/16.0f*sq_A*B - 1.0f/4.0f*A*C + D;

	if (Math::isZero(r))
	{
		/* no absolute term: y(y^3 + py + q) = 0 */

		coeffs[ 0 ] = q;
		coeffs[ 1 ] = p;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = 1;

		num = solveCubic(coeffs, s);

		s[ num++ ] = 0;
	}
	else
	{
		/* solve the resolvent cubic ... */

		coeffs[ 0 ] = 1.0f/2.0f * r * p - 1.0f/8.0f * q * q;
		coeffs[ 1 ] = - r;
		coeffs[ 2 ] = - 1.0f/2.0f * p;
		coeffs[ 3 ] = 1;

		(void) solveCubic(coeffs, s);

		/* ... and take the one real solution ... */

		z = s[ 0 ];

		/* ... to build two quadric equations */

		u = z * z - r;
		v = 2 * z - p;

		if (Math::isZero(u))
			u = 0;
		else if (u > 0)
			u = Math::sqrt(u);
		else
			return 0;

		if (Math::isZero(v))
			v = 0;
		else if (v > 0)
			v = Math::sqrt(v);
		else
			return 0;

		coeffs[ 0 ] = z - u;
		coeffs[ 1 ] = q < 0 ? -v : v;
		coeffs[ 2 ] = 1;

		num = solveQuadric(coeffs, s);

		coeffs[ 0 ]= z + u;
		coeffs[ 1 ] = q < 0 ? v : -v;
		coeffs[ 2 ] = 1;

		num += solveQuadric(coeffs, s + num);
	}

	/* resubstitute */

	sub = 1.0f/4.0f * A;

	for (i = 0; i < num; ++i)
		s[ i ] -= sub;

	return num;
}

int RootSolver::solve(float* c, float* s, int nargs)
{
	for (int i=nargs-1; i>=0; --i)
	{
		if (!Math::isZero(c[i]))
		{
			nargs = i+1;
			break;
		}
	}

	int n = 0;
	switch (nargs)
	{
	case 2: n = RootSolver::solveLinear(c, s); break;
	case 3: n = RootSolver::solveQuadric(c, s); break;
	case 4: n = RootSolver::solveCubic(c, s); break;
	case 5: n = RootSolver::solveQuartic(c, s); break;
	}

	return n;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
