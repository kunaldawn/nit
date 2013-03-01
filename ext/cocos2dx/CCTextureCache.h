/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
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

#ifndef __CCTEXTURE_CACHE_H__
#define __CCTEXTURE_CACHE_H__

#include <string>
#include "CCObject.h"
#include "CCMutableDictionary.h"
#include "CCTexture2D.h"

NS_CC_BEGIN;

class CCAsyncObject;
class CCLock;

typedef void (*fpAsyncCallback)(CCTexture2D*, void*);

/** @brief Singleton that handles the loading of textures
* Once the texture is loaded, the next time it will return
* a reference of the previously loaded texture reducing GPU & CPU memory
*/
class CC_DLL CCTextureCache : public WeakSupported
{
protected:
	CCMutableDictionary<String, CCTexture2D*> * m_pTextures;

private:
	// @todo void addImageWithAsyncObject(CCAsyncObject* async);

public:

	CCTextureCache();
	virtual ~CCTextureCache();

	char * description(void);

	/** Retruns ths shared instance of the cache */
	static CCTextureCache * sharedTextureCache();
	static void _setInstance(CCTextureCache* instance);

	/** Returns a Texture2D object given an file image
	* If the file image was not previously loaded, it will create a new CCTexture2D
	*  object and it will return it. It will use the filename as a key.
	* Otherwise it will return a reference of a previosly loaded image.
	* Supported image extensions: .png, .bmp, .tiff, .jpeg, .pvr, .gif
	*/
	CCTexture2D* addImage(StreamSource* imageSource);

	/** Returns an already created texture. Returns nil if the texture doesn't exist.
	@since v0.99.5
	*/
// 	CCTexture2D* textureForKey(const char* key);
	/** Purges the dictionary of loaded textures.
	* Call this method if you receive the "Memory Warning"
	* In the short term: it will free some resources preventing your app from being killed
	* In the medium term: it will allocate more resources
	* In the long term: it will be the same
	*/
	void removeAllTextures();

	/** Removes unused textures
	* Textures that have a retain count of 1 will be deleted
	* It is convinient to call this method after when starting a new Scene
	* @since v0.8
	*/
	void removeUnusedTextures();

	/** Deletes a texture from the cache given a texture
	*/
	void removeTexture(CCTexture2D* texture);

	/** Deletes a texture from the cache given a its key name
	@since v0.99.4
	*/
//	void removeTextureForKey(const char *textureKeyName);

	/** Output to CCLOG the current contents of this CCTextureCache
	* This will attempt to calculate the size of each texture, and the total texture memory in use
	*
	* @since v1.0
	*/
	void dumpCachedTextureInfo();

    /** Reload all textures */
	// TODO: remove
    static void reloadAllTextures();
};

NS_CC_END;

#endif //__CCTEXTURE_CACHE_H__

