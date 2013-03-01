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
#include "CCLabelTTF.h"

NS_CC_BEGIN;

//
//CCLabelTTF
//
CCLabelTTF::CCLabelTTF()
    : m_eAlignment(CCTextAlignmentCenter)
    , m_fFontSize(0.0)
    , m_pString(NULL)
{
}

CCLabelTTF::~CCLabelTTF()
{
	CC_SAFE_DELETE(m_pString);        
}

CCLabelTTF * CCLabelTTF::labelWithString(const char *label, const CCSize& dimensions, CCTextAlignment alignment, const char *fontName, float fontSize)
{
	CCLabelTTF *pRet = new CCLabelTTF();
	if(pRet && pRet->initWithString(label, dimensions, alignment, fontName, fontSize))
	{
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet);
	return NULL;
}
CCLabelTTF * CCLabelTTF::labelWithString(const char *label, const char *fontName, float fontSize)
{
	return labelWithString(label, CCSizeZero, CCTextAlignmentCenter, fontName, fontSize);
}

CCLabelTTF* CCLabelTTF::labelWithString(const char* label, CCFont* font, float size)
{
	return labelWithString(label, CCSizeZero, CCTextAlignmentCenter, font, size);
}

CCLabelTTF* CCLabelTTF::labelWithString(const char* label, const CCSize& dimensions, CCTextAlignment alignment, CCFont* font, float fontSize)
{
	CCLabelTTF *pRet = new CCLabelTTF();
	if(pRet && pRet->initWithString(label, dimensions, alignment, font, fontSize))
	{
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet);
	return NULL;
}

bool CCLabelTTF::initWithString(const char *label, const CCSize& dimensions, CCTextAlignment alignment, const char *fontName, float fontSize)
{
	assert(label != NULL);
	if (CCSprite::init())
	{
		float scale = CC_CONTENT_SCALE_FACTOR();

		m_tDimensions = CCSizeMake( dimensions.width * scale, dimensions.height * scale );
		m_eAlignment = alignment;

		CCFontManager* fontMgr = CCFontManager::getSingleton();

		m_fFontSize = fontSize;
		m_Font = fontMgr->getFont(fontName);

		if (m_Font == NULL)
		{
			LOG(0, "?? CCLabelTTF '%s' : font '%s' not found, using default\n", label, fontName);
		}

		this->setString(label);
		return true;
	}
	return false;
}

bool CCLabelTTF::initWithString(const char* label, const CCSize& dimensions, CCTextAlignment alignment, CCFont* font, float fontSize)
{
	assert(label != NULL);
	if (CCSprite::init())
	{
		float scale = CC_CONTENT_SCALE_FACTOR();

		m_tDimensions = CCSizeMake( dimensions.width * scale, dimensions.height * scale );
		m_eAlignment = alignment;
		m_Font = font;
		m_fFontSize = fontSize;

		this->setString(label);
		return true;
	}
	return false;
}

void CCLabelTTF::setFont(CCFont* font)
{
	if (m_Font == font)
		return;

	m_Font = font;

	updateLabelTexture();
}

void CCLabelTTF::setString(const char *label)
{
    if (m_pString)
    {
		if (m_pobTexture && strcmp(label, m_pString->c_str()) == 0)
			return;

        delete m_pString;
        m_pString = NULL;
    }
    m_pString = new String(label);

	updateLabelTexture();
}

const char* CCLabelTTF::getString(void)
{
	return m_pString->c_str();
}

char * CCLabelTTF::description()
{
	char *ret = new char[100] ;
	sprintf(ret, "<CCLabelTTF | FontFace = %s, FontSize = %.1f>", m_Font->getFace()->getName().c_str(), m_fFontSize);
	return ret;
}

void CCLabelTTF::updateLabelTexture()
{
	const char* label = m_pString ? m_pString->c_str() : NULL;

	if (label == NULL)
	{
		return;
	}

	CCTexture2D *texture = NULL;

	CCFontManager* fontMgr = CCFontManager::getSingleton();

	if (m_Font == NULL)
		m_Font = fontMgr->getDefault();

	if( CCSize::CCSizeEqualToSize( m_tDimensions, CCSizeZero ) )
	{
		texture = new CCTexture2D();
		texture->initWithString(label, m_Font, m_fFontSize);
	}
	else
	{
		texture = new CCTexture2D();
		texture->initWithString(label, m_Font, m_fFontSize, m_tDimensions, m_eAlignment);
	}

	this->setTexture(texture);
	if (texture) texture->release();

	if (texture && texture->isHeaderLoaded())
	{
		CCRect rect = CCRectZero;
		rect.size = texture->getContentSize();
		this->setTextureRect(rect, false);
	}
}

NS_CC_END;