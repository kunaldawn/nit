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

// TODO: I have to admit that I need much more english study 
// to translate comments into english in this file. (ellongrey)

#include "nit_pch.h"

#include "nit/math/Curves.h"

#include "nit/math/Solver.h"
#include "nit/math/Ray.h"

NS_NIT_BEGIN

////////////////////////////////////////////////////////////////////////////////

Curve2D::Curve2D(const Vector2* points, uint numPoints)
: TBezierCurve<Vector2>(points, numPoints)
{
}

Curve2D::Curve2D(Curve3D* c, PlaneAxis axis)
{
	_numSections = c->getNumSections();
	_numPoints = c->getNumPoints();

	_points = new Vector2[_numPoints];

	for (uint i=0; i<_numPoints; ++i)
	{
		// convert to 2d point
		const Vector3& p = c->getPoint(i);
		Vector2& v = _points[i];

		switch (axis)
		{
		case AXIS_X: v.x = p.z; v.y = p.y; break;
		case AXIS_Y: v.x = p.x; v.y = p.z; break;
		case AXIS_Z: v.x = p.x; v.y = p.y; break;
		default: ASSERT(false);
		}
	}

	_totalLength = -1.0f;
	_partialLength = NULL;
}

Curve2D* Curve2D::ratio(Curve2D* a, Curve2D* b, float r)
{
	if (a->_numSections != b->_numSections || a->_numPoints != b->_numPoints)
		return NULL;

	uint numPoints = a->_numPoints;

	Curve2D* c = new Curve2D();
	c->_points = new Vector2[numPoints];

	for (uint i=0; i<numPoints; ++i)
		c->_points[i] = r * (a->_points[i] + b->_points[i]);

	c->_numPoints = numPoints;
	c->_numSections = a->_numSections;

	c->_totalLength = -1.0f;
	c->_partialLength = NULL;

	return c;
}

// FindNearestCrossing

//      P
//      |
//     /|
//    /.+----> perpD
//   +. V ..
//  .V      .
// .         .

// V is crossing if PV * perpD = 0
// Because b = PV * D is a dot product, b[i] can form 1D cubic bezier polynomial:
// b = c[0] + c[1] * t + c[2] * t^2 + c[3] * t^3
// we can find solution where b = 0

bool Curve2D::findNearestCrossing(const Vector2& P, const Vector2& D, float& outT, Vector2& outP)
{
	float b[4];
	float c[4];
	float t[3];

	bool hasSolution = false;
	float dist = Math::MAX_FLOAT;

	Vector2 perpD(D.y, -D.x);

	for (uint section = 0; section < _numSections; ++section)
	{
		Vector2* V = getSection(section);

		// calc bernstein coefficents
		// b[i] = (V[i] - P) * perpD;
		for (int i=0; i<4; ++i)
		{
			b[i] = (V[i] - P).dotProduct(perpD);
		}

		// convert to power basis polynomial and solve
		BezierSolver<3>::toPowerBasis(b, c);

		int numSol = RootSolver::solveCubic(c, t);

		for (int i=0; i<numSol; ++i)
		{
			if (t[i] < 0.0f || t[i] > 1.0f)
				continue;

			Vector2 p; Math::bezier(V[0], V[1], V[2], V[3], t[i], &p);
			float nd = (P - p).squaredLength();

			if (nd < dist)
			{
				dist = nd;
				outT = fromNorm(section, t[i]);
				outP = p;
				hasSolution = true;
			}
		}
	}

	return hasSolution;
}

////////////////////////////////////////////////////////////////////////////////

Curve3D::Curve3D(const Vector3* points, uint numPoints)
: TBezierCurve<Vector3>(points, numPoints)
{
}

Curve3D* Curve3D::ratio(Curve3D* a, Curve3D* b, float r)
{
	if (a->_numSections != b->_numSections || a->_numPoints != b->_numPoints)
		return NULL;

	uint numPoints = a->_numPoints;
	
	Curve3D* c = new Curve3D();
	c->_points = new Vector3[numPoints];

	for (uint i=0; i<numPoints; ++i)
		c->_points[i] = r * (a->_points[i] + b->_points[i]);

	c->_numPoints = numPoints;
	c->_numSections = a->_numSections;

	c->_totalLength = -1.0f;
	c->_partialLength = NULL;

	return c;
}

/*
 * NearestRayPoints
 *
 * step 1.
 * given t, R(n, o):
 *   Qd(t)  = norm(Q'(t))				(Vector3)
 *   C(t)   = Qd(t) x R.n				(Vector3)
 *   P(t).n = norm(C(t) x R.n)			(Vector3)
 *   P(t).d = -P(t).n * R.o				(float)
 *
 *   d(t)   = P(t).n * Q(t) + P(t).d	(float)
 *            ----------------
 *               P(t).n * Qd(t)			(float)  (생략 가능)
 *
 *   find t where d(t) = 0
 *
 * step 2.
 * for found t:
 *   u = (Q(t) - R.o) * R.n
 *   d = sqdist(R(u) - Q(t))
 *
 *   find u with smallest d
 *
 * 설명:
 * 1단계.
 *   곡선상 점 Q(t)는 방향벡터 Qd(t)를 가짐. 
 *   Q(t)와 Qd(t)로 반직선 Q를 만들 수 있음.
 *   그러면 반직선 교차 공식을 이용하여 문제를 풀 수 있는데,
 *   R과 Q를 잇는 최단거리 직선의 방향벡터 C(t)는 Qd(t) x R.n이 됨.
 *   이때 점 R.o, 두 방향벡터 R.n과 C(t)가 만드는 평면을 P(t)라 하면,
 *   Q와 P(t)의 교점과 Q(t) 사이의 거리 d(t)를 계산할 수 있다.
 *
 *   만일 이 거리가 0이라면, Q(t)는 최단거리 점이 된다.
 *   따라서 d(t) = 0이 되는 t를 구하면 되는 것이다.
 *
 * 구현:
 *   곡선 상의 점에서 유도되는 Q(t), Q'(t) -> Qd(t)는 번스타인 계수로 만들어야 하며,
 *   Q(t)와 Q'(t)에 관계된 연산이므로 4 x 3 계수 테이블이 필요하다.
 *
 * 2단계.
 *   구한 t의 점 P = Q(t)와 R 사이의 최단 거리를 구하는데,
 *   R.o -> P로 가는 벡터와 N의 내적이 R.o에서 P사이의 거리 u가 된다.
 *   R(u)와 P 사이의 거리를 구하여, 최소 값을 가지는 것을 리턴한다.
 */

bool Curve3D::nearestRayPoint(const Ray& r, float& outT, float& outRT)
{
	float w[6];
	float t[5 + 2]; // include 0, 1 (to avoid numeric error)

	const Vector3& R_n = r.getDirection();
	const Vector3& R_o = r.getOrigin();

	float dist = Math::MAX_FLOAT;
	bool hasSolution = false;

	for (uint section = 0; section < _numSections; ++section)
	{
		Vector3* Q = getSection(section);

		// Qd(t) = norm(Q'(t))
		Vector3 Qd[3];
		for (int i=0; i<3; ++i)
		{
			Qd[i] = 3.0f * (Q[i] - Q[i+1]);
			Qd[i].normalise();
		}

		// C(t) = Qd(t) x R.n
		Vector3 C[3];
		for (int i=0; i<3; ++i)
		{
			C[i] = Qd[i].crossProduct(R_n);
		}

		// P(t).n = norm(C(t) x R.n)
		// P(t).d = -P(t).n * R.o
		Vector3 P_n[3];
		float P_d[3];
		for (int i=0; i<3; ++i)
		{
			P_n[i] = C[i].crossProduct(R_n);
			P_n[i].normalise();
			P_d[i] = -P_n[i].dotProduct(R_o);
		}

		// d(t) = (P(t).n * Q(t) + P(t).d) / (P(t).n * Qd(t))
		// b(t) = z * d(t)
		float b[3 * 4];

		const float z[3 * 4] =
		{
			1.0f,	0.6f,	0.3f,	0.1f,
			0.4f,	0.6f,	0.6f,	0.4f,
			0.1f,	0.3f,	0.6f,	1.0f,
		};

		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<3; ++j)
			{
				float d = (P_n[j].dotProduct(Q[i]) + P_d[j]) / (P_n[j].dotProduct(Qd[j]));
				b[i + j*4] = d * z[i + j*4];
			}
		}

		// merge bernstein coefficients into bezier coefficients
		BezierSolver<5>::mergeTable(4, 3, b, w);

		// Find all possible roots of 5th-degree equation
		int numSolutions = BezierSolver<5>::solveBernsteinBasis(w, t);

		// include 0, 1
		t[numSolutions++] = 0.0f;
		t[numSolutions++] = 1.0f;

		// Find minimal distance
		for (int i=0; i<numSolutions; ++i)
		{
			Vector3 P; Math::bezier(Q[0], Q[1], Q[2], Q[3], t[i], &P);
			float rt = (P - R_o).dotProduct(R_n);
			if (rt < 0.0f) continue;
			float d = r.getPoint(rt).squaredDistance(P);
			if (d < dist)
			{
				dist = d;
				outT = fromNorm(section, t[i]);
				outRT = rt;
				hasSolution = true;
			}
		}
	}

	return hasSolution;
}

bool Curve3D::closestPlaneIntersect(const Vector3& P, const Vector3& n, float& outT, Vector3& outP)
{
	// P is a point on the plane
	// n is normal of the plane

	float b[4];
	float c[4];
	float t[3];

	// calc plane constant
	float pd = P.dotProduct(n);

	bool hasSolution = false;
	float dist = Math::MAX_FLOAT;

	for (uint section = 0; section < _numSections; ++section)
	{
		Vector3* V = getSection(section);

		// calculate b[i] = V[i] * n - pd;
		for (int i=0; i<4; ++i)
		{
			b[i] = V[i].dotProduct(n) - pd;
		}

		// convert to power basis polynomial and solve
		BezierSolver<3>::toPowerBasis(b, c);

		int numSol = RootSolver::solveCubic(c, t);

		for (int i=0; i<numSol; ++i)
		{
			if (t[i] < 0.0f || t[i] > 1.0f)
				continue;

			Vector3 p; Math::bezier(V[0], V[1], V[2], V[3], t[i], (Vector3*)&p);
			float nd = (P - p).squaredLength();
			if (nd < dist)
			{
				dist = nd;
				outT = fromNorm(section, t[i]);
				outP = p;
				hasSolution = true;
			}
		}
	}

	return hasSolution;
}

// FindTrackLine:

//          N                         + B
//      (X)  +--             (O)     /|
//           |\ --                N / |
//           | \  --               +  |
//           |  +----+ B          / \ |
//           | / P               /   \|
//           |/				    +-----+
//         A +                 A       P

// Find t where (N x PA) dot PB = 0

// In other word, we will find PB coplanar to plane P-A-N

// 설명:
// 직선 PB가 평면 P-A-N에 눕게되는 시점 t를 찾는다.
// 점 P는 주어진 N으로 AB 직선 위에 있게 되며, A와 B는 곡선의 두 점이므로, 이것이 트랙을 가로지르는 선이 된다.

// A와 B는 각각 3차 다항식으로 유도되므로, 중간의 내적연산에 의해 전체는 6차 베지어 다항식이 된다.
// 베지어 다항식을 계산하기 위해 2차원 테이블을 만들고, 이것을 1차원 번스타인 계수로 변환하여 값을 구한다.

// param other : the other side curve which compose track together this
//               number of sections must be same with this one
// param P     : The position player standing on
// param N     : player's up vector

bool Curve3D::findTrackLine(Curve3D* other, const Vector3& P, const Vector3& N, float& outT, Vector3& outVa, Vector3& outVb)
{
	if (other->_numSections != _numSections) return false;

	float w[7];
	float t[6];

	float dist = Math::MAX_FLOAT;

	for (uint section = 0; section < _numSections; ++section)
	{
		Vector3* Qa = getSection(section);
		Vector3* Qb = other->getSection(section);

		// Convert polynomial to 6th-degree bezier form

		// Precalculated table : z[i, j] = binomial(n, i) * binomial(m, j) / binomial(m+n, i+j)
		const float z[4 * 4] =
		{
			1.0f,  0.5f,  0.2f,  0.05f,
			0.5f,  0.6f,  0.45f, 0.2f,
			0.2f,  0.45f, 0.6f,  0.5f,
			0.05f, 0.2f,  0.5f,  1.0f
		};

		// Calculate z * (N x (Qa(t) - P) dot (Qb(t).U - P))
		float b[4 * 4];

		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<4; ++j)
			{
				b[i + j*4] = (N.crossProduct(Qa[i] - P).dotProduct(Qb[j] - P)) * z[i + j*4];
			}
		}

		// merge table into bezier coefficients
		BezierSolver<6>::mergeTable(4, 4, b, w);

		// Find all possible roots of 6th-degree equation
		int numSolutions = BezierSolver<6>::solveBernsteinBasis(w, t);

		// Find distance for candidate points
		for (int i=0; i<numSolutions; ++i)
		{
			if (t[i] < 0.0f || t[i] > 1.0f) 
				continue;

			Vector3 Va; Math::bezier(Qa[0], Qa[1], Qa[2], Qa[3], t[i], &Va);
			Vector3 Vb; Math::bezier(Qb[0], Qb[1], Qb[2], Qb[3], t[i], &Vb);

			float nd = Math::min((P - Va).squaredLength(), (P - Vb).squaredLength());

			if (nd < dist)
			{
				dist = nd;
				outT = fromNorm(section, t[i]);
				outVa = Va;
				outVb = Vb;
			}
		}
	}

	return dist < Math::MAX_FLOAT;
}

void Curve3D::findAllTrackLine(Curve3D* other, const Vector3& P, const Vector3& N, vector<float>::type& outTArray)
{
	if (other->_numSections != _numSections) return;

	float w[7];
	float t[6];

	for (uint section = 0; section < _numSections; ++section)
	{
		Vector3* Qa = getSection(section);
		Vector3* Qb = other->getSection(section);

		// Convert polynomial to 6th-degree bezier form

		// Precalculated table : z[i, j] = binomial(n, i) * binomial(m, j) / binomial(m+n, i+j)
		const float z[4 * 4] =
		{
			1.0f,  0.5f,  0.2f,  0.05f,
			0.5f,  0.6f,  0.45f, 0.2f,
			0.2f,  0.45f, 0.6f,  0.5f,
			0.05f, 0.2f,  0.5f,  1.0f
		};

		// Calculate z * (N x (Qa(t) - P)) dot (Qb(t).U - P)
		float b[4 * 4];

		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<4; ++j)
			{
				b[i + j*4] = (N.crossProduct(Qa[i] - P).dotProduct(Qb[j] - P)) * z[i + j*4];
			}
		}

		// merge table into bezier coefficients
		BezierSolver<6>::mergeTable(4, 4, b, w);

		// Find all possible roots of 6th-degree equation
		int numSolutions = BezierSolver<6>::solveBernsteinBasis(w, t);

		// Find distance for candidate points
		for (int i=0; i<numSolutions; ++i)
		{
			if (t[i] < 0.0f || t[i] > 1.0f) 
				continue;

			outTArray.push_back(fromNorm(section, t[i]));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;