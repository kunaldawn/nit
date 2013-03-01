#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2009      Jason Booth

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

#include "CCConfiguration.h"
#include "CCRenderTexture.h"
#include "CCDirector.h"
#include "CCTextureCache.h"

#include "nitrender/RenderService.h"

NS_CC_BEGIN;

// implementation CCRenderTexture
CCRenderTexture::CCRenderTexture()
: m_pSprite(NULL)
, m_nOldFBO(0)
, m_pTexture(0)
, m_ePixelFormat(kCCTexture2DPixelFormat_RGBA8888)
{
	m_FrameBuffer = new GLESFrameBufferHandle();
}

CCRenderTexture::~CCRenderTexture()
{
    removeAllChildrenWithCleanup(true);
}

CCSprite * CCRenderTexture::getSprite()
{
	return m_pSprite;
}

CCRenderTexture * CCRenderTexture::renderTextureWithWidthAndHeight(int w, int h, CCTexture2DPixelFormat eFormat)
{
    CCRenderTexture *pRet = new CCRenderTexture();

    if(pRet && pRet->initWithWidthAndHeight(w, h, eFormat))
    {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

CCRenderTexture * CCRenderTexture::renderTextureWithWidthAndHeight(int w, int h)
{
	CCRenderTexture *pRet = new CCRenderTexture();

    if(pRet && pRet->initWithWidthAndHeight(w, h, kCCTexture2DPixelFormat_RGBA8888))
	{
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet)
	return NULL;
}

bool CCRenderTexture::initWithWidthAndHeight(int w, int h, CCTexture2DPixelFormat eFormat)
{
	bool needPOT = !g_Render->isRenderTargetNPOTSupported();

	float scale = CC_CONTENT_SCALE_FACTOR();

	w = int(w * scale);
	h = int(h * scale);

	int texWidth = w;
	int texHeight = h;

	if (needPOT)
	{
		texWidth = PixelFormat::calcNextPot(w);
		texHeight = PixelFormat::calcNextPot(h);
	}

	m_ePixelFormat = eFormat;

	m_pTexture = new CCTexture2D();
	m_pTexture->initWithTexture(
		new GLESTexture(texWidth, texHeight, CCTexture2D::ToNitPixelFormat(eFormat, false), NULL, w, h));

	m_pTexture->setAliasTexParameters();

	m_pSprite = CCSprite::spriteWithTexture(m_pTexture);
	CCRect rect = CCRectZero;
	rect.size = m_pTexture->getContentSize();
	m_pSprite->setTextureRect(rect, false);

	m_pTexture->release();
	m_pSprite->setScaleY(-1);
	this->addChild(m_pSprite);

	ccBlendFunc blendFunc = { GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
	m_pSprite->setBlendFunc(blendFunc);

	return true;
}

bool CCRenderTexture::activateFBO(RenderContext* ctx)
{
	if (m_FrameBuffer->isValid(ctx))
		return true;

	if (m_pTexture->getNitTexture()->activate(ctx) != GLESTexture::AR_OK)
		return false;

	// generate FBO
	if (!m_FrameBuffer->generate(ctx))
		return false; // Device not ready

	// associate texture with FBO
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &m_nOldFBO);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_FrameBuffer->useHandle(ctx));
	glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, m_pTexture->activate(ctx), 0);

	// check if it worked
	CCAssert(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES, 
		"Render Texture : Could not attach texture to framebuffer");

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_nOldFBO);
	m_nOldFBO = 0;

	return true;
}

void CCRenderTexture::begin(RenderContext* ctx)
{
	if (!activateFBO(ctx))
		return;

	// Save the current matrix
	glPushMatrix();

	const CCSize& texSize = m_pTexture->getContentSizeInPixels();

	// Calculate the adjustment ratios based on the old and new projections
	CCSize size = CCDirector::sharedDirector()->getDisplaySizeInPixels();
	float widthRatio = size.width / texSize.width;
	float heightRatio = size.height / texSize.height;

	// Apply the projection of the director projection first
	CCDirector::sharedDirector()->applyProjection(ctx);

	// Adjust the orthographic propjection and viewport
	glOrthof((float)-1.0 / widthRatio,  (float)1.0 / widthRatio, (float)-1.0 / heightRatio, (float)1.0 / heightRatio, -1,1);
	ctx->viewport(0, 0, (GLsizei)texSize.width, (GLsizei)texSize.height);

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &m_nOldFBO);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_FrameBuffer->useHandle(ctx));//Will direct drawing to the frame buffer created above

	ctx->setAlphaBlending(true);
	ctx->setDepthTest(true);

	CCDirector::sharedDirector()->enableDefaultGLStates(ctx);
}

void CCRenderTexture::beginWithClear(RenderContext* ctx, float r, float g, float b, float a)
{
    this->begin(ctx);

	// save clear color
	GLfloat	clearColor[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE,clearColor); 

	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// restore clear color
	glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);     
}

void CCRenderTexture::end(RenderContext* ctx)
{
	// NOTE: avoid sharedDirector()->disableDefaultGLStates(), because it would disrupt further rendering process.

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_nOldFBO);
	// Restore the original matrix and viewport
	glPopMatrix();
	ctx->viewport(CCDirector::sharedDirector()->GetRenderView());
}

void CCRenderTexture::clear(RenderContext* ctx, float r, float g, float b, float a)
{
	this->beginWithClear(ctx, r, g, b, a);
	this->end(ctx);
}

Image* CCRenderTexture::newImageCopy(RenderContext* ctx, int x, int y, int nWidth, int nHeight)
{
	if (m_pTexture == NULL)
		return NULL;

	if (!m_FrameBuffer->isValid(ctx))
		return NULL;

	const CCSize& s = m_pTexture->getContentSizeInPixels();
	int tx = (int)s.width;
	int ty = (int)s.height;

	if (x < 0 || x >= tx || y < 0 || y >= ty) return NULL;
	if (nWidth < 0 || nHeight < 0) return NULL;
	if (nWidth == 0 && nHeight != 0) return NULL;
	if (nHeight == 0 && nWidth != 0) return NULL;

	// If the area we want to acquire is larger than the actual texture size - do a clipping
	int saveWidth = nWidth;
	int saveHeight = nHeight;

	if (saveWidth == 0) saveWidth = tx;
	if (saveHeight == 0) saveHeight = ty;

	saveWidth = (x + saveWidth > tx) ? (tx - x) : saveWidth;
	saveHeight = (y + saveHeight > ty) ? (ty - y) : saveHeight;

	CCAssert(m_ePixelFormat == kCCTexture2DPixelFormat_RGBA8888, "only RGBA8888 can be saved as image");

	int imageWidth = saveWidth;
	int imageHeight = saveHeight;

	int maxTextureSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

	bool needPOT = !g_Render->isRenderTargetNPOTSupported();

	if (needPOT)
	{
		imageWidth = PixelFormat::calcNextPot(imageWidth);
		imageHeight = PixelFormat::calcNextPot(imageHeight);
	}

	if (imageWidth == 0 || imageHeight == 0)
		return NULL;

	CCAssert(imageWidth <= maxTextureSize && imageHeight <= maxTextureSize, "pot texture size exceeds maximum tex size");

	size_t texBufSize = imageWidth * imageHeight * 4;
	GLubyte* texBuf = (GLubyte*) NIT_ALLOC(texBufSize);
	ASSERT_THROW(texBuf, EX_MEMORY);

	// Perform read operation on GL - read as POT-ed size
	GLint oldFBO = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldFBO);
	GLint oldViewport[4];
	glGetIntegerv(GL_VIEWPORT, oldViewport);

	glViewport(0, 0, tx, ty);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_FrameBuffer->useHandle(ctx));
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(x, y, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, texBuf);

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFBO);
	glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
	
	// Prepare the header - Assign source & content size as specified size
	Image::Header hdr = { 0 };

	hdr.width			= imageWidth;
	hdr.height			= imageHeight;
	hdr.sourceWidth		= saveWidth;
	hdr.sourceHeight	= saveHeight;
	hdr.mipmapCount		= 1;
	hdr.memorySize		= texBufSize;
	hdr.contentLeft		= 0;
	hdr.contentRight	= 0;
	hdr.contentRight	= saveWidth;
	hdr.contentBottom	= saveHeight;
	hdr.pixelFormat		= PixelFormat::RGBA_8888_PA;

	bool needFlip = true; // Upside-down on OPEN-GL when reading render target

	// Create image instance
	Image* img = NULL;

	if (!needFlip && saveWidth == imageWidth && saveHeight == imageHeight)
	{
		// Buffer copy not necessary
		img = new Image(hdr, texBuf); // image takes the texBuf
	}
	else
	{
		// Buffer copy needed
		img = new Image(hdr, texBuf, imageWidth, imageHeight, imageWidth * 4, false);
		NIT_DEALLOC(texBuf, texBufSize);
	}

	return img;
}

NS_CC_END;
