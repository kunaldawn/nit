#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2008      Apple Inc. All Rights Reserved.

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



/*
* Support for RGBA_4_4_4_4 and RGBA_5_5_5_1 was copied from:
* https://devforums.apple.com/message/37855#37855 by a1studmuffin
*/

#include "CCTexture2D.h"

#include "ccConfig.h"
#include "ccMacros.h"
#include "CCConfiguration.h"
#include "CCPlatformMacros.h"

NS_CC_BEGIN;

//CLASS IMPLEMENTATIONS:

// If the image has alpha, you can create RGBA8 (32-bit) or RGBA4 (16-bit) or RGB5A1 (16-bit)
// Default is: RGBA8888 (32-bit textures)
static CCTexture2DPixelFormat g_defaultAlphaPixelFormat = kCCTexture2DPixelFormat_Default;

/////////////////////////////////////////////////////////////////////////////////

class CC_DLL CCTextImage : public Image
{
public:
	enum ETextAlign
	{
		kAlignCenter        = 0x33, ///< Horizontal center and vertical center.
		kAlignTop           = 0x13, ///< Horizontal center and vertical top.
		kAlignTopRight      = 0x12, ///< Horizontal right and vertical top.
		kAlignRight         = 0x32, ///< Horizontal right and vertical center.
		kAlignBottomRight   = 0x22, ///< Horizontal right and vertical bottom.
		kAlignBottom        = 0x23, ///< Horizontal center and vertical bottom.
		kAlignBottomLeft    = 0x21, ///< Horizontal left and vertical bottom.
		kAlignLeft          = 0x31, ///< Horizontal left and vertical center.
		kAlignTopLeft       = 0x11, ///< Horizontal left and vertical top.
	};

	/**
	@brief	Create image with specified string.
	@param  font		the font object which use to draw the text. If nil, use the default font of the manager.
	@param  size        the font size, if 0, use the system default size.
	@param  pText       the text which the image show, nil cause init fail
	@param  nWidth      the image width, if 0, the width match the text's width
	@param  nHeight     the image height, if 0, the height match the text's height
	@param  eAlignMask  the test Alignment
	*/
	CCTextImage(
		const String&	text,
		CCFont*			font,
		float			size,
		int				nWidth = 0,
		int				nHeight = 0,
		ETextAlign		eAlignMask = kAlignCenter);

protected:
	virtual void						loadHeader();
	virtual void						onLoad(bool async);

	String								m_Text;
	Ref<CCFont>							m_Font;
	float								m_Size;
	int									m_Width;
	int									m_Height;
	ETextAlign							m_AlignMask;
};

/////////////////////////////////////////////////////////////////////////////////

CCTextImage::CCTextImage(const String& text, CCFont* font, float size, int nWidth, int nHeight, ETextAlign eAlignMask)
{
	m_Text		= text;
	m_Font		= font;
	m_Size		= size;
	m_Width		= nWidth;
	m_Height	= nHeight;
	m_AlignMask = eAlignMask;

	if (m_Width & 0x01) ++m_Width;
	if (m_Height & 0x01) ++m_Height;

	_linked	= true;
}

void CCTextImage::loadHeader()
{
	if (_headerLoaded) return;

	Ref<CCFont> font = m_Font;

	if (font == NULL)
	{
		CCFontManager* fontMgr = CCFontManager::getSingleton();
		font = fontMgr->getDefault();
	}

	if (font == NULL)
	{
		CCLOGERROR("no default font assigned while rendering: '%s'", m_Text.c_str());
		_error = true;
		return;
	}

	int align = 0;

	DWORD dwHoriFlag = m_AlignMask & 0x0f;
	DWORD dwVertFlag = (m_AlignMask & 0xf0) >> 4;

	switch (dwHoriFlag)
	{
	case 1: // left
		align |= CCFont::ALIGN_LEFT;
		break;
	case 2: // right
		align |= CCFont::ALIGN_RIGHT;
		break;
	case 3: // center
		align |= CCFont::ALIGN_CENTER;
		break;
	}

	switch (dwVertFlag)
	{
	case 1: // top
		align |= CCFont::ALIGN_TOP;
		break;
	case 2: // bottom
		align |= CCFont::ALIGN_BOTTOM;
		break;
	case 3: // middle
		align |= CCFont::ALIGN_MIDDLE;
		break;
	}

	float size = m_Size;
	if (size == 0.0f)
		size = font->getDefaultSize();

	size *= CC_CONTENT_SCALE_FACTOR();

	int textWidth = m_Width;
	int textHeight = m_Height;
	
	if (m_Width == 0 || m_Height == 0)
	{
		Ref<CCFont::RenderSource> rs = font->prepare(m_Text.c_str(), size, m_Width, m_Height, align);

		textWidth = rs->_textWidth;
		textHeight = rs->_textHeight;
//		LOG(0, "++ prep text: '%s' %dx%d\n", m_Text.c_str(), textWidth, textHeight);
	}
	else
	{
//		LOG(0, "++ skip text: '%s' %dx%d\n", m_Text.c_str(), textWidth, textHeight);
	}

	size_t bpp = 2; // TODO: Expose parameters for user tweaks

	size_t memSize = textWidth * textHeight * bpp;

	_header.width			= textWidth;
	_header.height			= textHeight;
	_header.pixelFormat	= bpp == 2 ? PixelFormat::RGBA_4444_PA : PixelFormat::RGBA_8888_PA;
	_header.memorySize		= memSize;

	_header.sourceWidth	= textWidth;
	_header.sourceHeight	= textHeight;

	_header.contentLeft	= 0;
	_header.contentTop		= 0;
	_header.contentRight	= textWidth;
	_header.contentBottom	= textHeight;

	_header.mipmapCount	= 1;

	if (_pixelBuffer)
		Deallocate(_pixelBuffer, 0);

	_pitch					= textWidth * bpp;
	_bitsPerPixel			= bpp * 8;

	_headerLoaded			= true;
}

void CCTextImage::onLoad(bool async)
{
	Ref<CCFont> font = m_Font;

	if (font == NULL)
	{
		CCFontManager* fontMgr = CCFontManager::getSingleton();
		font = fontMgr->getDefault();
	}

	if (font == NULL)
	{
		CCLOGERROR("no default font assigned while rendering: '%s'", m_Text.c_str());
		_error = true;
		return;
	}

	int align = 0;

	DWORD dwHoriFlag = m_AlignMask & 0x0f;
	DWORD dwVertFlag = (m_AlignMask & 0xf0) >> 4;

	switch (dwHoriFlag)
	{
	case 1: // left
		align |= CCFont::ALIGN_LEFT;
		break;
	case 2: // right
		align |= CCFont::ALIGN_RIGHT;
		break;
	case 3: // center
		align |= CCFont::ALIGN_CENTER;
		break;
	}

	switch (dwVertFlag)
	{
	case 1: // top
		align |= CCFont::ALIGN_TOP;
		break;
	case 2: // bottom
		align |= CCFont::ALIGN_BOTTOM;
		break;
	case 3: // middle
		align |= CCFont::ALIGN_MIDDLE;
		break;
	}

	float size = m_Size;
	if (size == 0.0f)
		size = font->getDefaultSize();

	size *= CC_CONTENT_SCALE_FACTOR();

	Ref<CCFont::RenderSource> rs = font->prepare(m_Text.c_str(), size, m_Width, m_Height, align);

	int textWidth = rs->_textWidth;
	int textHeight = rs->_textHeight;

	size_t bpp = 2; // TODO: Expose parameters for user tweaks

	size_t memSize = textWidth * textHeight * bpp;
	uint8* imageData = Allocate(memSize);
	memset(imageData, 0x00, memSize);

	_header.width			= textWidth;
	_header.height			= textHeight;
	_header.pixelFormat	= bpp == 2 ? PixelFormat::RGBA_4444_PA : PixelFormat::RGBA_8888_PA;
	_header.memorySize		= memSize;
	
	_header.sourceWidth	= textWidth;
	_header.sourceHeight	= textHeight;

	_header.contentLeft	= 0;
	_header.contentTop		= 0;
	_header.contentRight	= textWidth;
	_header.contentBottom	= textHeight;

	_header.mipmapCount	= 1;

	if (_pixelBuffer)
		Deallocate(_pixelBuffer, 0);

	_pixelBuffer	= (uint8*)imageData;
	_pitch			= textWidth * bpp;
	_bitsPerPixel	= bpp * 8;

	_headerLoaded	= true;

//	LOG(0, "++ blit text: '%s' %dx%d\n", m_Text.c_str(), textWidth, textHeight);
	rs->blitBitmap(imageData, bpp, textWidth, textHeight, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////

CCTexture2D::CCTexture2D()
{
	m_ePixelFormat = kCCTexture2DPixelFormat_UNKNOWN;
}

CCTexture2D::~CCTexture2D()
{
	if (m_NitTex && m_NitTex->getManager() == NULL)
		m_NitTex->dispose();
}

bool CCTexture2D::initWithTexture(GLESTexture* tex)
{
	try 
	{ 
		m_NitTex = tex;
		m_NitTex->loadHeader(); 

		m_ePixelFormat = FromNitPixelFormat(m_NitTex->getPixelFormat());

		if (m_ePixelFormat == kCCTexture2DPixelFormat_UNKNOWN)
			NIT_THROW_FMT(EX_NOT_SUPPORTED, "pixel format '%s' not recognized by cocos", m_NitTex->getPixelFormat().getName().c_str());

		Vector2 min(0, 0);
		Vector2 max;
		const GLESTexture::Header& hdr = tex->getHeader();

		m_SourceSize.width = hdr.sourceWidth;
		m_SourceSize.height = hdr.sourceHeight;

		min.x = hdr.contentLeft / float(hdr.width);
		min.y = hdr.contentTop / float(hdr.height);
		max.x = hdr.contentRight / float(hdr.width);
		max.y = hdr.contentBottom / float(hdr.height);
		m_NitTex->setMinTexCoord(min);
		m_NitTex->setMaxTexCoord(max);
	}
	catch (Exception& ex)
	{
		CCLOGERROR("can't load texture '%s': %s", tex->getSourceUrl().c_str(), ex.getFullDescription().c_str());
		return false;
	}

	return true;
}

bool CCTexture2D::initWithString(const char* text, CCFont* font, float size)
{
	return initWithString(text, font, size, CCSizeMake(0,0), CCTextAlignmentCenter);
}

bool CCTexture2D::initWithString(const char* text, CCFont* font, float size, const CCSize& dimensions, CCTextAlignment alignment)
{
	CCTextImage::ETextAlign eAlign = (CCTextAlignmentCenter == alignment) ? CCTextImage::kAlignCenter
		: (CCTextAlignmentLeft == alignment) ? CCTextImage::kAlignLeft : CCTextImage::kAlignRight;

	Ref<Image> image = new CCTextImage(text, font, size, (int)dimensions.width, (int)dimensions.height, eAlign);

	bool convertTo4444 = false; 
	if (convertTo4444)
	{
		// TODO: 4444 font blitter already implemented - examine again and remove this
		image->load();
		if (image->isLoaded()) 
			image->makeRgba_4444();
	}

	Ref<GLESTexture> tex = new GLESTexture(image);
	return initWithTexture(tex);
}

// void CCTexture2D::setTexParameters(ccTexParams* texParams)
// {
// 	// TODO: refactor to nit::Texture
// 	CCAssert( (getPixelsWide() == ccNextPOT(getPixelsWide()) && getPixelsHigh() == ccNextPOT(getPixelsHigh())) ||
// 		(texParams->wrapS == GL_CLAMP_TO_EDGE && texParams->wrapT == GL_CLAMP_TO_EDGE),
// 		"GL_CLAMP_TO_EDGE should be used in NPOT textures");
// 	glBindTexture( GL_TEXTURE_2D, m_NitTex->GetHandle() );
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams->minFilter );
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams->magFilter );
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams->wrapS );
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams->wrapT );
// }

void CCTexture2D::setAliasTexParameters()
{
	m_NitTex->setMinFilter(Texture::MIN_NEAREST);
	m_NitTex->setMagFilter(Texture::MAG_NEAREST);
}

void CCTexture2D::setAntiAliasTexParameters()
{
	m_NitTex->setMinFilter(Texture::MIN_LINEAR);
	m_NitTex->setMagFilter(Texture::MAG_LINEAR);
}

void CCTexture2D::generateMipmap()
{
	CCAssert( (getPixelsWide() == PixelFormat::calcNextPot(getPixelsWide()) && getPixelsHigh() == PixelFormat::calcNextPot(getPixelsHigh())), 
		"Mimpap texture only works in POT textures");

	m_NitTex->setAutoGenMipmap(true);
}

unsigned int CCTexture2D::bitsPerPixelForFormat()
{
	return PixelFormat::calcBitsPerPixel(m_NitTex->getPixelFormat());
}

void CCTexture2D::setDefaultAlphaPixelFormat(CCTexture2DPixelFormat format)
{
	g_defaultAlphaPixelFormat = format;
}

cocos2d::CCTexture2DPixelFormat CCTexture2D::defaultAlphaPixelFormat()
{
	return g_defaultAlphaPixelFormat;
}

cocos2d::CCTexture2DPixelFormat CCTexture2D::FromNitPixelFormat(PixelFormat fmt)
{
	switch (fmt.getValue())
	{
	case PixelFormat::RGBA_8888:
	case PixelFormat::RGBA_8888_PA:
		return kCCTexture2DPixelFormat_RGBA8888; break;

	case PixelFormat::BGRA_8888:
	case PixelFormat::BGRA_8888_PA:
		return kCCTexture2DPixelFormat_BGRA8888; break;

	case PixelFormat::RGB_888:
		return kCCTexture2DPixelFormat_RGB888; break;

	case PixelFormat::RGB_565:
		return kCCTexture2DPixelFormat_RGB565; break;

	case PixelFormat::A8:
		return kCCTexture2DPixelFormat_A8; break;

	case PixelFormat::L8:
		return kCCTexture2DPixelFormat_I8; break;

	case PixelFormat::AL_88:
	case PixelFormat::AL_88_PA:
		return kCCTexture2DPixelFormat_AI88; break;

	case PixelFormat::RGBA_4444:
	case PixelFormat::RGBA_4444_PA:
		return kCCTexture2DPixelFormat_RGBA4444; break;

	case PixelFormat::RGBA_5551:
	case PixelFormat::RGBA_5551_PA:
		return kCCTexture2DPixelFormat_RGB5A1; break;

	case PixelFormat::PVR4:
	case PixelFormat::PVR4A:
	case PixelFormat::PVR4A_PA:
		return kCCTexture2DPixelFormat_PVRTC4; break;

	case PixelFormat::PVR2:
	case PixelFormat::PVR2A:
	case PixelFormat::PVR2A_PA:
		return kCCTexture2DPixelFormat_PVRTC2; break;

	default:
		return kCCTexture2DPixelFormat_UNKNOWN;
	}
}

PixelFormat CCTexture2D::ToNitPixelFormat(CCTexture2DPixelFormat fmt, bool alphaPremultiplied)
{
	PixelFormat::ValueType base;

	switch (fmt)
	{
	case kCCTexture2DPixelFormat_RGBA8888:
		base = PixelFormat::RGBA_8888; break;

	case kCCTexture2DPixelFormat_RGB888:
		base = PixelFormat::RGB_888; break;

	case kCCTexture2DPixelFormat_RGB565:
		base = PixelFormat::RGB_565; break;

	case kCCTexture2DPixelFormat_A8:
		base = PixelFormat::A8; break;

	case kCCTexture2DPixelFormat_I8:
		base = PixelFormat::L8; break;

	case kCCTexture2DPixelFormat_AI88:
		base = PixelFormat::AL_88; break;

	case kCCTexture2DPixelFormat_RGBA4444:
		base = PixelFormat::RGBA_4444; break;

	case kCCTexture2DPixelFormat_RGB5A1:
		base = PixelFormat::RGBA_5551; break;

	case kCCTexture2DPixelFormat_PVRTC4:
		base = PixelFormat::PVR4; break;

	case kCCTexture2DPixelFormat_PVRTC2:
		base = PixelFormat::PVR2; break;

	default:
		return PixelFormat::UNKNOWN;
	}

	if (alphaPremultiplied && (base & PixelFormat::FLAG_HAS_ALPHA))
		base |= PixelFormat::FLAG_PMUL_ALPHA;

	return base;
}

char* CCTexture2D::description()
{
	char *ret = new char[100];
	sprintf(ret, "<CCTexture2D | activated = %u | Dimensions = %u x %u | Coordinates = (%.2f, %.2f)>", 
		m_NitTex->isActivated(),
		m_NitTex->getHeader().width,
		m_NitTex->getHeader().height,
		m_NitTex->getMaxTexCoord().x,
		m_NitTex->getMaxTexCoord().y);

	return ret;
}

void CCTexture2D::drawAtPoint(RenderContext* ctx, const CCPoint& point)
{
	const Vector2& min = m_NitTex->getMinTexCoord();
	const Vector2& max = m_NitTex->getMaxTexCoord();

	GLfloat	coordinates[] = 
	{	
		min.x,	max.y,
		max.x,	max.y,
		min.x,	min.y,
		max.x,	min.y
	};

	GLfloat	width = getPixelsWide() * max.x;
	GLfloat height = getPixelsHigh() * max.y;

	GLfloat		vertices[] = 
	{	
		point.x,			point.y,	0.0f,
		width + point.x,	point.y,	0.0f,
		point.x,			height  + point.y,	0.0f,
		width + point.x,	height  + point.y,	0.0f 
	};

	glBindTexture(GL_TEXTURE_2D, activate(ctx));
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void CCTexture2D::drawInRect(RenderContext* ctx, const CCRect& rect)
{
	const Vector2& min = m_NitTex->getMinTexCoord();
	const Vector2& max = m_NitTex->getMaxTexCoord();

	GLfloat	coordinates[] = 
	{	
		min.x,	max.y,
		max.x,	max.y,
		min.x,	min.y,
		max.x,	min.y
	};

	GLfloat	vertices[] = {	rect.origin.x,		rect.origin.y,							/*0.0f,*/
		rect.origin.x + rect.size.width,		rect.origin.y,							/*0.0f,*/
		rect.origin.x,							rect.origin.y + rect.size.height,		/*0.0f,*/
		rect.origin.x + rect.size.width,		rect.origin.y + rect.size.height,		/*0.0f*/ };

	glBindTexture(GL_TEXTURE_2D, activate(ctx));
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

GLuint CCTexture2D::activate(RenderContext* ctx)
{
	if (m_NitTex == NULL) return 0;
	if (m_NitTex->getHandle()->isValid(ctx)) 
		return m_NitTex->getHandle()->useHandle(ctx);

	m_NitTex->deactivate();
	m_NitTex->load();
	if (m_NitTex->activate(ctx) != m_NitTex->AR_OK) return 0;

	m_NitTex->applyTexParams(ctx);
	return m_NitTex->getHandle()->useHandle(ctx);
}

bool CCTexture2D::isSameWith(CCTexture2D* other)
{
	if (this == other) return true;
	if (this == NULL || other == NULL) return false;

	if (this->m_NitTex == other->m_NitTex) return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////////

NS_CC_END;
