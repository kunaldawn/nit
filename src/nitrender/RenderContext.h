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

#include "nitrender/nitrender.h"

#include "nitrender/RenderSpec.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// Temporal RenderContext with which nit2d use.
// TODO: Implement one on nit3d and replace to it.

class RenderView;
class RenderDevice;
class RenderService;

class NITRENDER_API RenderContext : public WeakSupported
{
public:
	RenderContext(RenderView* view);
	virtual ~RenderContext();

public:
	RenderSpec*							getSpec()								{ return _spec; }
	RenderView*							getView()								{ return _view; }
	RenderDevice*						getDevice()								{ return _device; }
	RenderService*						getService()							{ return _service; }

public:
	void								viewport(int left, int top, int width, int height);
	void								viewport(RenderView* view);

	void								perspective(float fovy, float aspect, float zNear, float zFar);
	void								lookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ);

	void								setAlphaBlending(bool flag);
	void								setDepthTest(bool flag);

public:
	void								drawPoint2d(const Vector2& point);
	void								drawPoints2d(const Vector2* points, uint numPoints);
	void								drawLine2d(const Vector2& from, const Vector2& to);
	void								drawPoly2d(const Vector2* points, uint numPoints, bool filled=true, bool closed=true);
	void								drawCircle2d(const Vector2& center, float r, float a, int numSegments=14, bool drawLineToCenter=false);
	void								drawQuadBezier2d(const Vector2& from, const Vector2& control, const Vector2& to, int numSegments=15);
	void								drawCubicBezier2d(const Vector2& from, const Vector2& control1, const Vector2& control2, const Vector2& to, int numSegments=15);

protected:
	friend class RenderService;

	RenderSpec*							_spec;
	RenderView*							_view;
	RenderDevice*						_device;
	RenderService*						_service;
	int									_beginCount;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
