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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

template <class TVector>
inline TBezierCurve<TVector>::TBezierCurve(const TVector* points, uint numPoints)
{
	_numPoints = numPoints;

	_points = new TVector[numPoints];
	for (uint i=0; i<numPoints; ++i)
		_points[i] = points[i];

	_numSections = (numPoints - 1) / 3;
	ASSERT(_numSections >= 1);

	_totalLength = -1.0f;
	_partialLength = NULL;
}

template <class TVector>
inline TBezierCurve<TVector>::~TBezierCurve()
{
	if (_points)
		delete[] _points;

	if (_partialLength)
		delete[] _partialLength;
}

template <class TVector>
inline void TBezierCurve<TVector>::interpolate(float t, TVector* outPos, TVector* outTan, TVector* outAcc)
{
	int section; float normT;
	toNorm(t, section, normT);

	TVector* V = getSection(section);

	bezier(section, normT, outPos, outTan, outAcc);
}

template <class TVector>
inline float TBezierCurve<TVector>::fromNorm(int section, float normT)
{
	return (section + normT) / _numSections;
}

template <class TVector>
inline void TBezierCurve<TVector>::toNorm(float t, int& outSection, float& outNormT)
{
	if (t < 0.0f)
	{
		outSection = 0;
		outNormT = 0.0f;
	}
	else if (t >= 1.0f)
	{
		outSection = _numSections - 1;
		outNormT = 1.0f;
	}
	else
	{
		float f = (float)_numSections;
		outSection = (int)(t * f);
		outNormT = t * f - outSection;
	}
}

template <class TVector>
inline void TBezierCurve<TVector>::setPoint(uint index, const TVector& pt)
{
	if (index < _numPoints) 
	{
		_points[index] = pt;

		// reset total length to recalculate
		_totalLength = -1.0f;
	}
}

template <class TVector>
inline void TBezierCurve<TVector>::bezier(int section, float t, TVector* outPos, TVector* outTan, TVector* outAcc)
{
	TVector* V = getSection(section);
	Math::bezier(V[0], V[1], V[2], V[3], t, outPos, outTan, outAcc);
}

template <class TVector>
inline TVector TBezierCurve<TVector>::tan(float t)
{
	TVector outT; interpolate(t, NULL, &outT, NULL);
	return outT;
}

template <class TVector>
inline TVector TBezierCurve<TVector>::pos(float t)
{
	TVector p; interpolate(t, &p, NULL, NULL);
	return p;
}

template <class TVector>
inline float TBezierCurve<TVector>::speedAt(int section, float normT)
{
	TVector t; bezier(section, normT, NULL, &t, NULL);
	return t.length();
}

template <class TVector>
inline float TBezierCurve<TVector>::lengthAt(int section, float normT)
{
	// Legendre polynomial information for Gaussian quadrature of speed
	// on domain [0, normT], 0 <= normT <= 1
	static float modRoot[5] = 
	{
		// Legendre roots mapped to (root+1)/2
		0.046910077f,
		0.230765345f,
		0.5f,
		0.769234655f,
		0.953089922f
	};
	static float modCoeff[5] =
	{
		// original coefficients divided by 2
		0.118463442f,
		0.239314335f,
		0.284444444f,
		0.239314335f,
		0.118463442f
	};

	// Need to transform domain [0,normT] to [-1,1].  
	// If 0 <= x <= normT and -1 <= t <= 1, then x = normT*(t+1)/2.

	float l = 0.0f;
	for (int i=0; i<5; ++i)
	{
		l += modCoeff[i] * speedAt(section, normT * modRoot[i]);
	}

	return l * normT;
}

template <class TVector>
inline void TBezierCurve<TVector>::updateLength()
{
	if (_totalLength < 0.0f)
	{
		if (_partialLength) delete[] _partialLength;

		_partialLength = new float[_numSections+1];

		float tl = 0.0f;
		_partialLength[0] = 0.0f;

		for (uint i=0; i < _numSections; ++i)
		{
			float l = lengthAt(i, 1.0f);

			tl += l;
			_partialLength[i+1] = tl;
		}

		_totalLength = tl;
	}
}

template <class TVector>
inline float TBezierCurve<TVector>::getTotalLength()
{
	updateLength();
	return _totalLength;
}

template <class TVector>
inline float TBezierCurve<TVector>::toLength(float t)
{
	int section; float normT;
	toNorm(t, section, normT);

	updateLength();

	return _partialLength[section] + lengthAt(section, normT);
}

template <class TVector>
inline float TBezierCurve<TVector>::fromLength(float length)
{
	updateLength();

	if (length < 0.0f)
		return 0.0f;

	if (length > _totalLength)
		return 1.0f;

	// Determine which polynomial segment corresponds to length
	float dist = 0.0f;
	float normT = 0.0f;
	uint section;

	float l0 = 0.0f;
	for (section = 0; section < _numSections; ++section)
	{
		float l = _partialLength[section + 1];

		if (length <= l)
		{
			// distance along segment
			dist = length - l0;

			// initial guess for integral upper limit
			normT = dist / (l - l0);
			break;
		}
		l0 = l;
	}

	// Use Newton's method to invert the path length integral
	const int MAX_ITERATIONS = 32;
	const float TOLERANCE = 1e-04f;

	for (int i=0; i<MAX_ITERATIONS; ++i)
	{
		float diff = lengthAt(section, normT) - dist;
		if (Math::abs(diff) < TOLERANCE)
			break;

		normT -= diff / speedAt(section, normT);
	}

	return fromNorm(section, normT);
}

// FindNearestPoint 설명:
// 곡선 V의 점들 중 P에 가장 가까운 점 V(t)에 해당되는 t 값들을 찾는 함수.
// 기하학적으로 분석해 보면, f(t) = (V(t) - P) dot V'(t) = 0 이라는 방정식이 됨.
// 즉, V(t)에서 P로 향하는 벡터는, V'(t), 즉 V(t)의 t에서의 접선과 수직이 되는 성질을 이용함.

// (V(t) - P)가 3차 (베지어), V'(t)가 2차 (3차의 미분) 이므로, 
// 두 식을 내적하는 f(t)는 5차 방정식이 됨
// 5차 방정식은 일반적 근의 공식으로 구할 수 없으므로 BezierSolver를 이용해 풀어야 함.

// 방정식은 결국 번스타인 다항식 형태가 되므로, 굳이 PowerBasis로 변환 안해주고,
// BezierSolver::SolverBernsteinBasis()를 호출함.

// f(t)의 합산과정에서 합산 결과는 2차원 테이블 형태가 되는데, 이것을 1차원 번스타인 계수로 변환.

template <class TVector>
inline void TBezierCurve<TVector>::findNearestPoint(const TVector& P, float& outT, TVector& outP)
{
	// declare temp variables
	float w[6];
	float t[5 + 2]; // inclue 0, 1

	float dist = 1.0e+20f;

	for (uint section = 0; section < _numSections; ++section)
	{
		TVector* Q = getSection(section);

		// Convert local curve to 5th-degree bezier form

		// precalculated z[i,j] table
		const float z[4 * 3] = 
		{
			1.0f, 0.6f, 0.3f, 0.1f,
			0.4f, 0.6f, 0.6f, 0.4f,
			0.1f, 0.3f, 0.6f, 1.0f
		};

		float b[3 * 4];

		// Calculate Qd(t) from Q'(t)
		TVector Qd[3];

		for (int i=0; i<3; ++i)
		{
			Qd[i] = 3.0f * (Q[i] - Q[i+1]);
		}

		// Calculate ( (Q(t) - P) dot Qd(t) ) * z
		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<3; ++j)
			{
				b[i + j*4] = (Q[i] - P).dotProduct(Qd[j]) * z[i + j*4];
			}
		}

		// merge table into bezier coefficients
		BezierSolver<5>::mergeTable(4, 3, b, w);

		// Find all possible roots of 5th-degree equation
		int numSolutions = BezierSolver<5>::solveBernsteinBasis(w, t);

		// include 0 and 1
		t[numSolutions++] = 0.0f;
		t[numSolutions++] = 1.0f;

		// Find distances for candidate points
		for (int i=0; i<numSolutions; ++i)
		{
			TVector p; Math::bezier(Q[0], Q[1], Q[2], Q[3], t[i], &p);
			float nd = (P - p).squaredLength();
			if (nd < dist)
			{
				dist = nd;
				outT = fromNorm(section, t[i]);
				outP = p;
			}
		}
	}
}

// tc 시점의 점 P에서 같은 거리 r 에 있는 앞 뒷점 찾기
// 결국 점 P와 반지름 r인 구(2차원이라면 원)와 만나는 점 두개를 찾게 됨.
// 즉, (P - Q(t)).SqrLength = r * r 인 t를 찾는 함수임.
// 그런데, SqrLength는 Bezier form으로 변환할 수 없으므로,
// V dot V = v * v * cos(h) = v * v * 1 = v^2 인 점을 이용, 내적 형태로 구함.
// 내적하면 6차 번스타인 다항식이 나오므로, 번스타인 베지어 형식으로 해를 구함.

// param: outT1 : tc 보다 앞에 있는 t (없을 경우 -1)
// param: outT2 : tc 보다 뒤에 있는 t (없을 경우 -1)

template <class TVector>
inline void TBezierCurve<TVector>::sphereIntersect(float tc, float r, float& outT1, float& outT2)
{
	TVector P = pos(tc);
	float r2 = r * r;

	float w[7];
	float t[6];

	float dt1 = Math::MAX_FLOAT;
	float dt2 = Math::MAX_FLOAT;

	float tmin = Math::MAX_FLOAT;
	float tmax = Math::MIN_FLOAT;

	outT1 = -1.0f;
	outT2 = -1.0f;

	for (uint section = 0; section < _numSections; ++section)
	{
		TVector* V = getSection(section);

		// Convert polynomial to 6th-degree bezier form

		// Precalculated table : z[i, j] = binomial(n, i) * binomial(m, j) / binomial(m+n, i+j)
		const float z[4 * 4] =
		{
			1.0f,  0.5f,  0.2f,  0.05f,
			0.5f,  0.6f,  0.45f, 0.2f,
			0.2f,  0.45f, 0.6f,  0.5f,
			0.05f, 0.2f,  0.5f,  1.0f
		};

		// Calculate (Vi - P) dot (Vj - P)
		float b[4 * 4];

		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<4; ++j)
			{
				//b[i + j*4] = (V[i] - P) * (V[j] - P) * z[i + j*4];
				b[i + j*4] = (V[i] - P).dotProduct(V[j] - P) * z[i + j*4];
			}
		}

		// merge table into bezier coefficients
		BezierSolver<6>::mergeTable(4, 4, b, w);

		// Apply ' = r*r' term 
		for (int i=0; i<7; ++i)
		{
			w[i] -= r2;
		}

		// Find all possible roots of 6th-degree equation
		int numSol = BezierSolver<6>::solveBernsteinBasis(w, t);

		for (int i=0; i<numSol; ++i)
		{
			if (t[i] < 0.0f || t[i] > 1.0f)
				continue;

			float ta = fromNorm(section, t[i]);
			if (ta < tc && tc - ta < dt1)
			{
				outT1 = ta;
				dt1 = tc - ta;
			}

			if (ta > tc && ta - tc < dt2)
			{
				outT2 = ta;
				dt2 = ta - tc;
			}

			if (ta < tmin)
				tmin = ta;

			if (ta > tmax)
				tmax = ta;
		}

		if (outT1 >= 0.0f && outT2 >= 0.0f)
			// we've got enough result. no more search needed
			return;
	}

	// For looping
	if (isLooping())
	{
		if (outT1 < 0.0f && tmax > Math::MIN_FLOAT)
			outT1 = tmax;

		if (outT2 < 0.0f && tmin < Math::MAX_FLOAT)
			outT2 = tmin;
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;