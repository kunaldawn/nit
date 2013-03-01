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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API RootSolver
{
public:
	static int 							solve(float* c, float* s, int nargs);

	static int 							solveLinear(float c[2], float s[1]);
	static int 							solveQuadric(float c[3], float s[2]);
	static int 							solveCubic(float c[4], float s[3]);
	static int 							solveQuartic(float c[5], float s[4]);
};

////////////////////////////////////////////////////////////////////////////////

class Vector2;

template <int D>
class BezierSolver
{
public:
	static int							solvePowerBasis(float* c, float* s);					// can solve only 0 <= t <= 1
	static int							solveBernsteinBasis(float* c, float* s);

public:
	static void							toPowerBasis(float* b, float* p);						// b: array of bernstein basis, p: output power array
	static void							mergeTable(int col, int row, float* table, float* c);	// table: [i + j*row] form, c: output merged array

private:
	static int							findRoots(Vector2 w[], float t[], int depth, int maxDepth = 64);
	static float						computeXIntercept(Vector2 V[]);
	static bool							controlPolygonFlatEnough(Vector2 V[]);
	static int							crossingCount(Vector2 V[]);
	static Vector2						bezier(Vector2 V[], float t, Vector2 outLeft[], Vector2 outRight[]);
};

////////////////////////////////////////////////////////////////////////////////

template <int D>
int BezierSolver<D>::solvePowerBasis(float* c, float* s)
{
	Vector2 w[D+1];

	for (int i=0; i<=D; ++i)
	{
		w[i].x = float(i) / D;
		w[i].y = c[i];
	}

	// Convert power basis to bernstein basis
	// for given power basis c[i] => p0 ~ pn, divide by binomial coefficent (n i) and sum up
	//  p0     p0 / (n 0)    
	//  p1     p1 / (n 1)
	//  p2  => p2 / (n 2)  => sum up  => use top row as bernstein basis bi
	//  p3     p3 / (n 3)
	//  p4     p4 / (n 4)
	//  p5     p5 / (n 5)

	// ex) for p0 ~ pn = 1 5 10 10 5 1
	//  1      1 /  1      1  2  4  8 16 32  => b0 ~ b5 = 1 2 4 8 16 32  => w[i].y
	//  5      5 /  5      1  2  4  8 16
	// 10  => 10 / 10  =>  1  2  4  8
	// 10     10 / 10      1  2  4
	//  5      5 /  5      1  2
	//  1      1 /  1      1

	for (int j=1; j<=D; ++j)
	{
		float p = 1.0f / float(D + 1 - j);
		float q = 1.0f;
		float r = p;

		for (int i=D; i>=j; --i)
		{
			w[i].y = q * w[i].y + r * w[i-1].y;
			q -= p;
			r += p;
		}
	}

	return findRoots(w, s, 0);
}

template <int D>
inline int BezierSolver<D>::solveBernsteinBasis(float* c, float* s)
{
	Vector2 w[D+1];

	// simply assign c[i] as w[i].y
	for (int i=0; i<=D; ++i)
	{
		w[i].x = float(i) / D;
		w[i].y = c[i];
	}

	return findRoots(w, s, 0);
}

template <int D>
inline int BezierSolver<D>::findRoots(Vector2 w[], float t[], int depth, int maxDepth)
{
	switch (crossingCount(w))
	{
	case 0: return 0;

	case 1:
		if (depth >= maxDepth)
		{
			t[0] = (w[0].x + w[D].x) / 2.0f;
			return 1;
		}
		if (controlPolygonFlatEnough(w))
		{
			t[0] = computeXIntercept(w);
			return 1;
		}
		break;
	}

	// Otherwise, Solve Recusively after
	// subdividing control polygon

	Vector2 left[D+1];
	Vector2 right[D+1];
	float left_t[D+1];
	float right_t[D+1];

	bezier(w, 0.5f, left, right);
	int leftCount = findRoots(left, left_t, depth + 1, maxDepth);
	int rightCount = findRoots(right, right_t, depth + 1, maxDepth);

	// Gather solutions together
	for (int i=0; i<leftCount; ++i)
	{
		t[i] = left_t[i];
	}
	for (int i=0; i<rightCount; ++i)
	{
		t[i + leftCount] = right_t[i];
	}

	// send back total number of solutions
	return leftCount + rightCount;
}

template <int D>
inline float BezierSolver<D>::computeXIntercept(Vector2 V[])
{
	float x0 = V[0].x;
	float y0 = V[0].y;
	float dx = V[D].x - x0;
	float dy = V[D].y - y0;

	float s = -(dx * y0 - dy * x0) / dy;

	return s;
}

template <int D>
inline bool BezierSolver<D>::controlPolygonFlatEnough(Vector2 V[])
{
	// Derive the implicit equation for line connecting first and last control points
	float a = V[0].y - V[D].y;
	float b = V[D].x - V[0].x;
	float c = V[0].x * V[D].y - V[D].x * V[0].y;

	// Find the largest distance
	float maxDistanceAbove = 0.0f;
	float maxDistanceBelow = 0.0f;

	for (int i=1; i<D; ++i)
	{
		float value = a * V[i].x + b * V[i].y + c;

		if (value > maxDistanceAbove)
			maxDistanceAbove = value;
		else if (value < maxDistanceBelow)
			maxDistanceBelow = value;
	}

	float det = b;
	float dInv = 1.0f / det;

	// Implicit equation for "above" line
	float intercept1 = (c - maxDistanceAbove) * dInv;

	// Implicit equation for "below" line
	float intercept2 = (c - maxDistanceBelow) * dInv;

	// Compute intercepts of bounding box
	float leftIntercept = Math::min(intercept1, intercept2);
	float rightIntercept = Math::max(intercept1, intercept2);

	float error = rightIntercept - leftIntercept;

	return error < Math::EPSILON;
}

template <int D>
inline int BezierSolver<D>::crossingCount(Vector2 V[])
{
	int sign, oldSign;

	int numCrossings = 0;

	sign = oldSign = (int)Math::sign(V[0].y);
	for (int i=1; i<=D; ++i)
	{
		sign = (int) Math::sign(V[i].y);
		if (sign != oldSign)
			++numCrossings;
		oldSign = sign;
	}

	return numCrossings;
}

template <int D>
inline Vector2 BezierSolver<D>::bezier(Vector2 V[], float t, Vector2 outLeft[], Vector2 outRight[])
{
	Vector2 temp[D+1][D+1];

	for (int j=0; j<=D; ++j)
	{
		temp[0][j] = V[j];
	}

	// compute triangle
	for (int i = 1; i <= D; ++i)
	{
		for (int j = 0; j <= D - i; ++j)
		{
			temp[i][j] = (1.0f - t) * temp[i-1][j] + t * temp[i-1][j+1];
		}
	}

	// fill left array if not empty
	if (outLeft != NULL)
	{
		for (int j=0; j<=D; ++j)
		{
			outLeft[j] = temp[j][0];
		}
	}

	// fill right array if not empty
	if (outRight != NULL)
	{
		for (int j=0; j<=D; ++j)
		{
			outRight[j] = temp[D-j][j];
		}
	}

	return temp[D][0];
}

template <int D>
inline void BezierSolver<D>::mergeTable(int col, int row, float* table, float* c)
{
	const int n = col - 1;
	const int m = row - 1;

	for (int i=0; i<=n+m; ++i)
		c[i] = 0.0f;

	for (int k=0; k<=n+m; ++k)
	{
		int lb = Math::max(0, k-m);
		int ub = Math::min(k, n);

		for (int i=lb; i<=ub; ++i)
		{
			int j = k-i;

			c[i+j] += table[i + j*col];
		}
	}
}

template <>
inline void BezierSolver<2>::toPowerBasis(float* b, float* p)
{
	// derive quadric bezier basis as following:
	// 1 *   b0            b1          b2    
	// 2 *   b1-b0         b2-b1    
	// 1 *   b2-2b1+b0
	// and apply some optimization to reduce arithmetic instrunctions

	p[0] = b[0];
	p[1] = 2 * (b[1] - b[0]);
	p[2] = b[2] - b[1] - b[1] + b[0];
}


template <>
inline void BezierSolver<3>::toPowerBasis(float* b, float* p)
{
	// derive cubic bezier basis as following:
	// 1 *   b0            b1          b2         b3
	// 3 *   b1-b0         b2-b1       b3-b2
	// 3 *   b2-2b1+b0     b3-2b2+b1
	// 1 *   b3-3b2+3b1-b0
	// and apply some optimization to reduce arithmetic instrunctions

	p[0] = b[0];
	p[1] = 3 * (b[1] - b[0]);
	p[2] = 3 * (b[2] - b[1]) - p[1];
	p[3] = (b[3] - b[0]) + 3 * (b[1] - b[2]);
}

template <>
inline void BezierSolver<4>::toPowerBasis(float* b, float* p)
{
	// derive cubic bezier basis as following:
	// 1 *   b0            b1            b2           b3          b4
	// 4 *   b1-b0         b2-b1         b3-b2        b4-b3
	// 6 *   b2-2b1+b0     b3-2b2+b1     b4-2b3+b2
	// 4 *   b3-3b2+3b1-b0 b4-3b3+3b2-b1
	// 1 *   b4-3b3+3b2-b1-(b3-3b2+3b1-b0)

	// TODO: optimize

	p[0] = b[0];
	p[1] = 4 * (b[1] - b[0]);
	p[2] = 6 * (b[2] - 2*b[1] + b[0]);
	p[3] = 4 * (b[3] - 3*b[2] + 3*b[1] - b[0]);
	p[4] = (b[4] - 3*b[3] + 3*b[2] - b[1]) - (b[3] - 3*b[2] + 3*b[1] - b[0]);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
