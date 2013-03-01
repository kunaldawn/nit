#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2009      On-Core
 
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
#include "CCGrabber.h"
#include "ccMacros.h"
#include "CCTexture2D.h"

#include "nitrender/RenderService.h"

NS_CC_BEGIN;

CCGrabber::CCGrabber(void)
: m_oldFBO(0)
{
	m_FrameBuffer = new GLESFrameBufferHandle();
}

bool CCGrabber::activateFBO(RenderContext* ctx, CCTexture2D *pTexture)
{
	bool valid = m_FrameBuffer->isValid(ctx);

	if (valid && m_BoundTexture == pTexture)
		return true;

	if (!valid)
	{
		// generate FBO
		if (!m_FrameBuffer->generate(ctx))
			return false; // device not ready
	}

	m_BoundTexture = pTexture;

	if (pTexture == NULL)
		return false;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &m_oldFBO);

	// bind
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_FrameBuffer->useHandle(ctx));

	// associate texture with FBO
	glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D,
		pTexture->activate(ctx), 0);

	// check if it worked
	CCAssert(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES, 
		"Frame Grabber: Could not attach texture to framebuffer");

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_oldFBO);

	return true;
}

void CCGrabber::beforeRender(RenderContext* ctx, CCTexture2D *pTexture)
{
	if (!activateFBO(ctx, pTexture)) 
		return;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &m_oldFBO);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_FrameBuffer->useHandle(ctx));

	// BUG XXX: doesn't work with RGB565.

	/*glClearColor(0, 0, 0, 0);*/

	// BUG #631: To fix #631, uncomment the lines with #631
	// Warning: But it CCGrabber won't work with 2 effects at the same time
	glClearColor(0.0f,0.0f,0.0f,0.0f);	// #631

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColorMask(true, true, true, false);	// #631
}

void CCGrabber::afterRender(RenderContext* ctx)
{
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_oldFBO);
	glColorMask(true, true, true, true);	// #631
}

CCGrabber::~CCGrabber()
{
	CCLOGINFO("cocos2d: deallocing %p", this);
}

NS_CC_END;