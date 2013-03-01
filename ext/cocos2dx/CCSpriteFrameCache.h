/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2009      Jason Booth
Copyright (c) 2009      Robert J Payne
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

#ifndef __SPRITE_CCSPRITE_FRAME_CACHE_H__
#define __SPRITE_CCSPRITE_FRAME_CACHE_H__

/*
 * To create sprite frames and texture atlas, use this tool:
 * http://zwoptex.zwopple.com/
 */

#include <string>
#include "CCSpriteFrame.h"
#include "CCTexture2D.h"
#include "CCObject.h"
#include "CCMutableDictionary.h"

NS_CC_BEGIN;

class CCSprite;

/** @brief Singleton that handles the loading of the sprite frames.
 It saves in a cache the sprite frames.
 @since v0.9
 */
class CC_DLL CCSpriteFrameCache : public WeakSupported
{
public:
	bool init(void);
	~CCSpriteFrameCache(void);

	/*Adds multiple Sprite Frames with a DataRecord. The texture will be associated with the created sprite frames.
	 */
	void addSpriteFramesWithRecord(DataRecord* rec, CCTexture2D* texture);

	/** Adds multiple Sprite Frames from a plist file.
	 * A texture will be loaded automatically. The texture name will composed by replacing the .plist suffix with .png
	 * If you want to use another texture, you should use the addSpriteFramesWithFile:texture method.
	 */
	void addSpriteFramesWithFile(StreamSource* plistSource);

	/** Adds multiple Sprite Frames from a plist file. The texture will be associated with the created sprite frames.
	@since v0.99.5
	*/
	void addSpriteFramesWithFile(StreamSource* plistSource, const char* textureFileName);

	/** Adds multiple Sprite Frames from a plist file. The texture will be associated with the created sprite frames. */
	void addSpriteFramesWithFile(StreamSource* plistSource, CCTexture2D *pobTexture);

	/** Adds an sprite frame with a given name.
	 If the name already exists, then the contents of the old name will be replaced with the new one.
	 */
	void addSpriteFrame(CCSpriteFrame *pobFrame, const char *pszFrameName);

	/** Purges the dictionary of loaded sprite frames.
	 * Call this method if you receive the "Memory Warning".
	 * In the short term: it will free some resources preventing your app from being killed.
	 * In the medium term: it will allocate more resources.
	 * In the long term: it will be the same.
	 */
	void removeSpriteFrames(void);

	/** Removes unused sprite frames.
	 * Sprite Frames that have a retain count of 1 will be deleted.
	 * It is convenient to call this method after when starting a new Scene.
	 */
	void removeUnusedSpriteFrames(void);

	/** Deletes an sprite frame from the sprite frame cache. */
	void removeSpriteFrameByName(const char *pszName);

	/** Removes multiple Sprite Frames from a plist file.
	* Sprite Frames stored in this file will be removed.
	* It is convinient to call this method when a specific texture needs to be removed.
	* @since v0.99.5
	*/
	void removeSpriteFramesFromFile(StreamSource* plistSource);

	/** Removes multiple Sprite Frames from DataRecord.
	* @since v0.99.5
	*/
	void removeSpriteFramesFromRecord(DataRecord* rec);

	/** Removes all Sprite Frames associated with the specified textures.
	* It is convinient to call this method when a specific texture needs to be removed.
	* @since v0.995.
	*/
	void removeSpriteFramesFromTexture(CCTexture2D* texture);

	/** Returns an Sprite Frame that was previously added.
	 If the name is not found it will return nil.
	 You should retain the returned copy if you are going to use it.
	 */
	CCSpriteFrame* spriteFrameByName(const char *pszName);

public:
	/** Returns the shared instance of the Sprite Frame cache */
	static CCSpriteFrameCache* sharedSpriteFrameCache(void);

	static void _setInstance(CCSpriteFrameCache* instance);

public:
	// HACK: Do not use in user code - only in CocosService!
	// TODO: make friend access
	CCSpriteFrameCache(void) : m_pSpriteFrames(NULL), m_pSpriteFramesAliases(NULL){}

private:
	const char * valueForKey(const char *key, CCDictionary<String, CCObject*> *dict);

protected:
	CCDictionary<String, CCSpriteFrame*> *m_pSpriteFrames;
	CCDictionary<String, CCString*> *m_pSpriteFramesAliases;
};

NS_CC_END;

#endif // __SPRITE_CCSPRITE_FRAME_CACHE_H__
