/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (C) 2008      Apple Inc. All Rights Reserved.

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

#ifndef __CCTEXTURE2D_H__
#define __CCTEXTURE2D_H__

#include <string>
#include "CCObject.h"
#include "CCGeometry.h"
#include "ccTypes.h"

#include "nitrender/GLESTexture.h"

NS_CC_BEGIN;

class CCFont;

//CONSTANTS:

/** @typedef CCTexture2DPixelFormat
Possible texture pixel formats
*/
typedef enum {
	kCCTexture2DPixelFormat_Automatic = 0,
	//! 32-bit texture: RGBA8888
	kCCTexture2DPixelFormat_RGBA8888,
	//! 24-bit texture: RGBA888
	kCCTexture2DPixelFormat_RGB888,
	//! 16-bit texture without Alpha channel
	kCCTexture2DPixelFormat_RGB565,
	//! 8-bit textures used as masks
	kCCTexture2DPixelFormat_A8,
	//! 8-bit intensity texture
	kCCTexture2DPixelFormat_I8,
	//! 16-bit textures used as masks
	kCCTexture2DPixelFormat_AI88,
	//! 16-bit textures: RGBA4444
	kCCTexture2DPixelFormat_RGBA4444,
	//! 16-bit textures: RGB5A1
	kCCTexture2DPixelFormat_RGB5A1,	
	//! 4-bit PVRTC-compressed texture: PVRTC4	- only powervr
	kCCTexture2DPixelFormat_PVRTC4,
	//! 2-bit PVRTC-compressed texture: PVRTC2	- only powervr
	kCCTexture2DPixelFormat_PVRTC2,
	//! 32-bit texture: RGBA8888				 - only powervr
	kCCTexture2DPixelFormat_BGRA8888,

	kCCTexture2DPixelFormat_UNKNOWN,

	//! Default texture format: RGBA8888
	kCCTexture2DPixelFormat_Default = kCCTexture2DPixelFormat_RGBA8888,

	// backward compatibility stuff
	kTexture2DPixelFormat_Automatic = kCCTexture2DPixelFormat_Automatic,
	kTexture2DPixelFormat_RGBA8888 = kCCTexture2DPixelFormat_RGBA8888,
	kTexture2DPixelFormat_RGB888 = kCCTexture2DPixelFormat_RGB888,
	kTexture2DPixelFormat_RGB565 = kCCTexture2DPixelFormat_RGB565,
	kTexture2DPixelFormat_A8 = kCCTexture2DPixelFormat_A8,
	kTexture2DPixelFormat_RGBA4444 = kCCTexture2DPixelFormat_RGBA4444,
	kTexture2DPixelFormat_RGB5A1 = kCCTexture2DPixelFormat_RGB5A1,
	kTexture2DPixelFormat_Default = kCCTexture2DPixelFormat_Default

} CCTexture2DPixelFormat;

/**
Extension to set the Min / Mag filter
*/
typedef struct _ccTexParams {
	GLuint	minFilter;
	GLuint	magFilter;
	GLuint	wrapS;
	GLuint	wrapT;
} ccTexParams;

/////////////////////////////////////////////////////////////////////////////////

// CCTexture2D is now just a wrapper class using nit::Texture
// TODO: remove and use nit.Texture directly

class CC_DLL CCTexture2D : public CCObject
{
public:
	static CCTexture2DPixelFormat		FromNitPixelFormat(PixelFormat fmt);
	static PixelFormat					ToNitPixelFormat(CCTexture2DPixelFormat fmt, bool alphaPremultiplied);

public:
	CCTexture2D();
	virtual ~CCTexture2D();

	char*								description();

	bool								isHeaderLoaded()						{ return this && this->m_NitTex && this->m_NitTex->isHeaderLoaded(); }
	bool								isLoaded()								{ return this && this->m_NitTex && this->m_NitTex->isLoaded(); }
	bool								isActivated()							{ return this && this->m_NitTex && this->m_NitTex->isActivated(); }

	bool								isSameWith(CCTexture2D* other);

	bool								initWithTexture(GLESTexture* tex);

	bool								initWithString(const char* text, CCFont* font, float size);
	bool								initWithString(const char* text, CCFont* font, float size, const CCSize& dimensions, CCTextAlignment alignment);

	GLuint								activate(RenderContext* ctx);
//	void								setTexParameters(ccTexParams* texParams);
	void								setAntiAliasTexParameters();
	void								setAliasTexParameters();
	void								generateMipmap();
	unsigned int						bitsPerPixelForFormat();
//	void								setPVRImagesHavePremultipliedAlpha(bool haveAlphaPremultiplied);

	static void							setDefaultAlphaPixelFormat(CCTexture2DPixelFormat format);
	static CCTexture2DPixelFormat		defaultAlphaPixelFormat();

	void								drawAtPoint(RenderContext* ctx, const CCPoint& point);
	void								drawInRect(RenderContext* ctx, const CCRect& rect);

public:
	GLESTexture*						getNitTexture()						{ return m_NitTex; }

	CCTexture2DPixelFormat				getPixelFormat()						{ return m_ePixelFormat; }
	unsigned int						getPixelsWide()							{ return m_NitTex->getHeader().width; }
	unsigned int						getPixelsHigh()							{ return m_NitTex->getHeader().height; }
	const CCSize&						getContentSizeInPixels()				{ return m_SourceSize; }
	CCSize								getContentSize()						{ return m_SourceSize; }
	bool								getHasPremultipliedAlpha()				{ return m_NitTex->getPixelFormat().isAlphaPremultiplied(); }

	const CCPoint&						getMinTexCoord()						{ return m_NitTex->getMinTexCoord(); }
	const CCPoint&						getMaxTexCoord()						{ return m_NitTex->getMaxTexCoord(); }
	void								setMinTexCoord(const CCPoint& st)		{ m_NitTex->setMinTexCoord(st); }
	void								setMaxTexCoord(const CCPoint& st)		{ m_NitTex->setMaxTexCoord(st); }

private:
	CCTexture2DPixelFormat				m_ePixelFormat;
	Ref<GLESTexture>					m_NitTex;
	CCSize								m_SourceSize;
};

/////////////////////////////////////////////////////////////////////////////////

NS_CC_END;

#endif //__CCTEXTURE2D_H__

