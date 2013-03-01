/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2008-2011 Ricardo Quesada
Copyright (c) 2011      Zynga Inc.

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

#ifndef __SPRITE_CCSPRITE_FRAME_H__
#define __SPRITE_CCSPRITE_FRAME_H__

#include "CCNode.h"
#include "CCProtocols.h"
#include "CCObject.h"

NS_CC_BEGIN;

class CCRect;
//class CCPoint;
class CCSize;
class CCTexture2D;
class CCZone;

/** @brief A CCSpriteFrame has:
	- texture: A CCTexture2D that will be used by the CCSprite
	- rectangle: A rectangle of the texture


 You can modify the frame of a CCSprite by doing:
 
	CCSpriteFrame *frame = CCSpriteFrame::frameWithTexture(texture, rect, offset);
	sprite->setDisplayFrame(frame);
 */
class CC_DLL CCSpriteFrame : public CCObject
{
public:
	// attributes

	inline bool isRotated(void) { return m_bRotated; }
	inline void setRotated(bool bRotated) { m_bRotated = bRotated; }

	/** get rect of the frame */
	inline const CCRect& getRect(void) { return m_obRect; }
	/** set rect of the frame */
	void setRect(const CCRect& rect);

	/** get offset of the frame */
	inline const CCPoint& getOffset(void) { return m_obOffset; }
	/** set offset of the frame */
	inline void setOffset(const CCPoint& offset) { m_obOffset = offset; }

	/** get original size of the trimmed image */
	inline const CCSize& getOriginalSize(void) { return m_obOriginalSize; }
	/** set original size of the trimmed image */
	inline void setOriginalSize(const CCSize& size) { m_obOriginalSize = size; }

	/** get texture of the frame */
	inline CCTexture2D* getTexture(void) { return m_pobTexture; }
	/** set texture of the frame, the texture is retained */
	inline void setTexture(CCTexture2D* pobTexture)
	{
		CC_SAFE_RETAIN(pobTexture);
		CC_SAFE_RELEASE(m_pobTexture);
		m_pobTexture = pobTexture;
	}

public:
	~CCSpriteFrame(void);
	virtual CCObject* copyWithZone(CCZone *pZone);

	/** Create a CCSpriteFrame with a texture, rect in points.
	 It is assumed that the frame was not trimmed.
	 */
	static CCSpriteFrame* frameWithTexture(CCTexture2D* pobTexture, const CCRect& rect);

	/** Create a CCSpriteFrame with a texture, rect, rotated, offset and originalSize in pixels.
	 The originalSize is the size in points of the frame before being trimmed.
	 */
    static CCSpriteFrame* frameWithTexture(CCTexture2D* pobTexture, const CCRect& rect, bool rotated, const CCPoint& offset, const CCSize& originalSize);

public:
	/** Initializes a CCSpriteFrame with a texture, rect in points.
	 It is assumed that the frame was not trimmed.
	 */
	bool initWithTexture(CCTexture2D* pobTexture, const CCRect& rect);

	/** Initializes a CCSpriteFrame with a texture, rect, rotated, offset and originalSize in pixels.
	The originalSize is the size in points of the frame before being trimmed.
	*/
	bool initWithTexture(CCTexture2D* pobTexture, const CCRect& rect, bool rotated, const CCPoint& offset, const CCSize& originalSize);
protected:
	bool   m_bRotated;
	CCRect m_obRect;
	CCPoint m_obOffset;
	CCSize m_obOriginalSize;
	CCTexture2D *m_pobTexture;
};

NS_CC_END;

#endif //__SPRITE_CCSPRITE_FRAME_H__
