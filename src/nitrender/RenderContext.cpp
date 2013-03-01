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

#include "NITRENDER_pch.h"

#include "nitrender/RenderContext.h"

#include "nitrender/RenderView.h"
#include "nitrender/RenderDevice.h"

#include <cmath>

#ifndef M_PI
#	define M_PI       3.14159265358979323846
#endif

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

RenderContext::RenderContext(RenderView* view)
{
	_view			= view;
	_device		= view->getRenderDevice();
	_spec			= _device->getSpec();
	_service		= NULL;
	_beginCount	= 0;
}

RenderContext::~RenderContext()
{
}

void RenderContext::perspective(float fovy, float aspect, float zNear, float zFar)
{
	GLfloat xmin, xmax, ymin, ymax;

	ymax = zNear * (GLfloat)tanf(fovy * (float)M_PI / 360);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustumf(xmin, xmax,
		ymin, ymax,
		zNear, zFar);	
}

void RenderContext::lookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ)
{
    GLfloat m[16];
    GLfloat x[3], y[3], z[3];
    GLfloat mag;

    /* Make rotation matrix */

    /* Z vector */
    z[0] = eyeX - centerX;
    z[1] = eyeY - centerY;
    z[2] = eyeZ - centerZ;
    mag = (float)sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    if (mag) {
        z[0] /= mag;
        z[1] /= mag;
        z[2] /= mag;
    }

    /* Y vector */
    y[0] = upX;
    y[1] = upY;
    y[2] = upZ;

    /* X vector = Y cross Z */
    x[0] = y[1] * z[2] - y[2] * z[1];
    x[1] = -y[0] * z[2] + y[2] * z[0];
    x[2] = y[0] * z[1] - y[1] * z[0];

    /* Recompute Y = Z cross X */
    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] = z[0] * x[1] - z[1] * x[0];

    /* cross product gives area of parallelogram, which is < 1.0 for
    * non-perpendicular unit-length vectors; so normalize x, y here
    */

    mag = (float)sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }

    mag = (float)sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag) {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }

#define M(row,col)  m[col*4+row]
    M(0, 0) = x[0];
    M(0, 1) = x[1];
    M(0, 2) = x[2];
    M(0, 3) = 0.0f;
    M(1, 0) = y[0];
    M(1, 1) = y[1];
    M(1, 2) = y[2];
    M(1, 3) = 0.0f;
    M(2, 0) = z[0];
    M(2, 1) = z[1];
    M(2, 2) = z[2];
    M(2, 3) = 0.0f;
    M(3, 0) = 0.0f;
    M(3, 1) = 0.0f;
    M(3, 2) = 0.0f;
    M(3, 3) = 1.0f;
#undef M
    {
        int a;
        GLfloat fixedM[16];
        for (a = 0; a < 16; ++a)
            fixedM[a] = m[a];
        glMultMatrixf(fixedM);
    }

    /* Translate Eye to Origin */
    glTranslatef(-eyeX, -eyeY, -eyeZ);
}

#define NIT_PA_BLEND_SRC	GL_ONE
#define NIT_PA_BLEND_DST	GL_ONE_MINUS_SRC_ALPHA

void RenderContext::setAlphaBlending(bool flag)
{
	if (flag)
	{
		glEnable(GL_BLEND);
		glBlendFunc(NIT_PA_BLEND_SRC, NIT_PA_BLEND_DST);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void RenderContext::setDepthTest(bool flag)
{
	if (flag)
	{
		glClearDepthf(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
//		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}

void RenderContext::drawPoint2d(const Vector2& point)
{
	float scale = _view->getScale();
	Vector2 p(point.x * scale, point.y * scale);
	// Default GL states: GL_TEXTURE_2D, GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	// Needed states: GL_VERTEX_ARRAY, 
	// Unneeded states: GL_TEXTURE_2D, GL_TEXTURE_COORD_ARRAY, GL_COLOR_ARRAY	
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, &p);	
	glDrawArrays(GL_POINTS, 0, 1);

	// restore default state
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);	
}

void RenderContext::drawPoints2d(const Vector2* points, uint numPoints)
{
	// Default GL states: GL_TEXTURE_2D, GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	// Needed states: GL_VERTEX_ARRAY, 
	// Unneeded states: GL_TEXTURE_2D, GL_TEXTURE_COORD_ARRAY, GL_COLOR_ARRAY	
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, points);
	glDrawArrays(GL_POINTS, 0, (GLsizei)numPoints);

	// restore default state
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);	
}

void RenderContext::drawLine2d(const Vector2& from, const Vector2& to)
{
	Vector2 vertices[2] = 
	{
		from, to
	};

	// Default GL states: GL_TEXTURE_2D, GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	// Needed states: GL_VERTEX_ARRAY, 
	// Unneeded states: GL_TEXTURE_2D, GL_TEXTURE_COORD_ARRAY, GL_COLOR_ARRAY	
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vertices);	
	glDrawArrays(GL_LINES, 0, 2);

	// restore default state
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);	
}

void RenderContext::drawPoly2d(const Vector2* points, uint numPoints, bool filled/*=true*/, bool closed/*=true*/)
{
	// Default GL states: GL_TEXTURE_2D, GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	// Needed states: GL_VERTEX_ARRAY, 
	// Unneeded states: GL_TEXTURE_2D, GL_TEXTURE_COORD_ARRAY, GL_COLOR_ARRAY	
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	// convert to pixels ?

	GLenum mode;
	if (closed)
		mode = filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP;
	else
		mode = filled ? GL_TRIANGLE_FAN : GL_LINE_STRIP;

	glVertexPointer(2, GL_FLOAT, 0, points);
	glDrawArrays(mode, 0, numPoints);

	// restore default state
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
}

void RenderContext::drawCircle2d(const Vector2& center, float r, float a, int numSegments/*=14*/, bool drawLineToCenter/*=true*/)
{
	int additionalSegment = 1;
	if (drawLineToCenter)
	{
		++additionalSegment;
	}

	const float coef = 2.0f * (float) (M_PI) / numSegments;

	vector<float>::type vertices(2 * (numSegments + 2));

	for(int i=0; i<=numSegments ;i++)
	{
		float rads = i*coef;
		float j = r * cosf(rads + a) + center.x;
		float k = r * sinf(rads + a) + center.y;

		vertices[i*2] = j;
		vertices[i*2+1] = k;
	}
	vertices[(numSegments+1)*2] = center.x;
	vertices[(numSegments+1)*2+1] = center.y;

	// Default GL states: GL_TEXTURE_2D, GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	// Needed states: GL_VERTEX_ARRAY, 
	// Unneeded states: GL_TEXTURE_2D, GL_TEXTURE_COORD_ARRAY, GL_COLOR_ARRAY	
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);	
	glDrawArrays(GL_LINE_STRIP, 0, (GLsizei) numSegments+additionalSegment);

	// restore default state
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);	
}

void RenderContext::drawQuadBezier2d(const Vector2& from, const Vector2& control, const Vector2& to, int numSegments/*=15*/)
{
	vector<Vector2>::type vertices(numSegments + 1);

	float t = 0.0f;
	for(int i = 0; i < numSegments; i++)
	{
		float x = powf(1 - t, 2) * from.x + 2.0f * (1 - t) * t * control.x + t * t * to.x;
		float y = powf(1 - t, 2) * from.y + 2.0f * (1 - t) * t * control.y + t * t * to.y;
		vertices[i].x = x;
		vertices[i].y = y;
		t += 1.0f / numSegments;
	}
	vertices[numSegments].x = to.x;
	vertices[numSegments].y = to.y;

	// Default GL states: GL_TEXTURE_2D, GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	// Needed states: GL_VERTEX_ARRAY, 
	// Unneeded states: GL_TEXTURE_2D, GL_TEXTURE_COORD_ARRAY, GL_COLOR_ARRAY	
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);	
	glDrawArrays(GL_LINE_STRIP, 0, (GLsizei) numSegments + 1);

	// restore default state
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);	
}

void RenderContext::drawCubicBezier2d(const Vector2& from, const Vector2& control1, const Vector2& control2, const Vector2& to, int numSegments/*=15*/)
{
	vector<Vector2>::type vertices(numSegments + 1);

	float t = 0;
	for(int i = 0; i < numSegments; ++i)
	{
		float x = powf(1 - t, 3) * from.x + 3.0f * powf(1 - t, 2) * t * control1.x + 3.0f * (1 - t) * t * t * control2.x + t * t * t * to.x;
		float y = powf(1 - t, 3) * from.y + 3.0f * powf(1 - t, 2) * t * control1.y + 3.0f * (1 - t) * t * t * control2.y + t * t * t * to.y;
		vertices[i].x = x;
		vertices[i].y = y;
		t += 1.0f / numSegments;
	}
	vertices[numSegments].x = to.x;
	vertices[numSegments].y = to.y;

	// Default GL states: GL_TEXTURE_2D, GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	// Needed states: GL_VERTEX_ARRAY, 
	// Unneeded states: GL_TEXTURE_2D, GL_TEXTURE_COORD_ARRAY, GL_COLOR_ARRAY	
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);	
	glDrawArrays(GL_LINE_STRIP, 0, (GLsizei) numSegments + 1);

	// restore default state
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);	
}

void RenderContext::viewport(int left, int top, int width, int height)
{
	glViewport(left, top, width, height);
}

void RenderContext::viewport(RenderView* view)
{
	float scale = view->getScale();
	glViewport(
		GLsizei(view->getLeft() * scale), 
		GLsizei(view->getTop() * scale), 
		GLsizei(view->getWidth() * scale), 
		GLsizei(view->getHeight() * scale) );
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
