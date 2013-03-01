#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada

http://www.cocos2d-x.org

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
****************************************************************************/

#include "CCCamera.h"

NS_CC_BEGIN;

CCCamera::CCCamera(void)
{
	init();
}

CCCamera::~CCCamera(void)
{
}

char * CCCamera::description(void)
{
	char *ret = new char[100];
	sprintf(ret, "<CCCamera | center = (%.2f,%.2f,%.2f)>", m_fCenterX, m_fCenterY, m_fCenterZ);
	return ret;
}

void CCCamera::init(void)
{
	restore();
}

void CCCamera::restore(void)
{
	m_fEyeX = m_fEyeY = 0.0f;
	m_fEyeZ = getZEye();

	m_fCenterX = m_fCenterY = m_fCenterZ = 0.0f;

	m_fUpX = 0.0f;
	m_fUpY = 1.0f;
	m_fUpZ = 0.0f;

	m_bDirty = false;
}

void CCCamera::locate(RenderContext* ctx)
{
	ctx->lookAt(m_fEyeX, m_fEyeY, m_fEyeZ,
		m_fCenterX, m_fCenterY, m_fCenterZ,
		m_fUpX, m_fUpY, m_fUpZ);
}

float CCCamera::getZEye(void)
{
	return FLT_EPSILON;
}

void CCCamera::setEyeXYZ(float fEyeX, float fEyeY, float fEyeZ)
{
	float scale = CC_CONTENT_SCALE_FACTOR();

	m_fEyeX = fEyeX * scale;
	m_fEyeY = fEyeY * scale;
	m_fEyeZ = fEyeZ * scale;

	m_bDirty = true;
}

void CCCamera::setCenterXYZ(float fCenterX, float fCenterY, float fCenterZ)
{
	float scale = CC_CONTENT_SCALE_FACTOR();

	m_fCenterX = fCenterX * scale;
	m_fCenterY = fCenterY * scale;
	m_fCenterZ = fCenterZ * scale;

	m_bDirty = true;
}

void CCCamera::setUpXYZ(float fUpX, float fUpY, float fUpZ)
{
	m_fUpX = fUpX;
	m_fUpY = fUpY;
	m_fUpZ = fUpZ;

	m_bDirty = true;
}

void CCCamera::getEyeXYZ(float *pEyeX, float *pEyeY, float *pEyeZ)
{
	float scale = 1.0f / CC_CONTENT_SCALE_FACTOR();

	*pEyeX = m_fEyeX * scale;
	*pEyeY = m_fEyeY * scale;
	*pEyeZ = m_fEyeZ * scale;
}

void CCCamera::getCenterXYZ(float *pCenterX, float *pCenterY, float *pCenterZ)
{
	float scale = 1.0f / CC_CONTENT_SCALE_FACTOR();

	*pCenterX = m_fCenterX * scale;
	*pCenterY = m_fCenterY * scale;
	*pCenterZ = m_fCenterZ * scale;
}

void CCCamera::getUpXYZ(float *pUpX, float *pUpY, float *pUpZ)
{
	*pUpX = m_fUpX;
	*pUpY = m_fUpY;
	*pUpZ = m_fUpZ;
}

NS_CC_END;
