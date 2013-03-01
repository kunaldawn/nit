#include "nit2d_pch.h"

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

#include <stack>
#include <string>
#include <cctype>
#include "CCTextureCache.h"
#include "CCTexture2D.h"
#include "ccMacros.h"
#include "CCData.h"
#include "CCDirector.h"

#include "nit/content/ContentsService.h"

NS_CC_BEGIN;

class CCAsyncObject : CCObject
{
public:
	fpAsyncCallback m_pfnCallback;
	CCObject* m_pTarget;
	String *  m_pData;
public:
	CCAsyncObject();
	~CCAsyncObject()
	{
		CCLOGINFO("cocos2d: deallocing CCAsyncObject.");
		CC_SAFE_DELETE(m_pTarget);
		CC_SAFE_DELETE(m_pData);
	}
};

// implementation CCTextureCache

// TextureCache - Alloc, Init & Dealloc
static CCTextureCache *g_sharedTextureCache;

CCTextureCache * CCTextureCache::sharedTextureCache()
{
	return g_sharedTextureCache;
}

void CCTextureCache::_setInstance(CCTextureCache* instance)
{
	g_sharedTextureCache = instance;
}

CCTextureCache::CCTextureCache()
{
	CCAssert(g_sharedTextureCache == NULL, "Attempted to allocate a second instance of a singleton.");
	
	m_pTextures = new CCMutableDictionary<String, CCTexture2D*>();
}

CCTextureCache::~CCTextureCache()
{
	CCLOGINFO("cocos2d: deallocing CCTextureCache.");

	CC_SAFE_RELEASE(m_pTextures);
}

char * CCTextureCache::description()
{
	char *ret = new char[100];
	sprintf(ret, "<CCTextureCache | Number of textures = %u>", m_pTextures->count());
	return ret;
}


// TextureCache - Add Images
/* @todo EAGLContext
void CCTextureCache::addImageWithAsyncObject(CCAsyncObject* async)
{
	
	CCAutoreleasePool *autoreleasepool = [[CCAutoreleasePool alloc] init];

	// textures will be created on the main OpenGL context
	// it seems that in SDK 2.2.x there can't be 2 threads creating textures at the same time
	// the lock is used for this purpose: issue #472
	[contextLock lock];
	if( auxEAGLcontext == nil ) {
		auxEAGLcontext = [[EAGLContext alloc]
initWithAPI:kEAGLRenderingAPIOpenGLES1
sharegroup:[[[[CCDirector sharedDirector] openGLView] context] sharegroup]];

		if( ! auxEAGLcontext )
			CCLOG(@"cocos2d: TextureCache: Could not create EAGL context");
	}

	if( [EAGLContext setCurrentContext:auxEAGLcontext] ) {

		// load / create the texture
		CCTexture2D *tex = [self addImage:async.data];

		// The callback will be executed on the main thread
		[async.target performSelectorOnMainThread:async.selector withObject:tex waitUntilDone:NO];

		[EAGLContext setCurrentContext:nil];
	} else {
		CCLOG(@"cocos2d: TetureCache: EAGLContext error");
	}
	[contextLock unlock];

	[autoreleasepool release];
}*/

/* @todo selector, NSThread
void CCTextureCache::addImageAsync(const char* filename, CCObject *target, fpAsyncCallback func)
{
	CCAssert(filename != NULL , "TextureCache: fileimage MUST not be nill");

	// optimization

	CCTexture2D * tex;
 
 	if ( (tex = m_pTextures->objectForKey(filename)) )

 	{

 		target->

 	}

 	
 	if( (tex=[textures objectForKey: filename] ) ) {
 		[target performSelector:selector withObject:tex];
 		return;
 	}
 
 	// schedule the load
 
 	CCAsyncObject *asyncObject = [[CCAsyncObject alloc] init];
 	asyncObject.selector = selector;
 	asyncObject.target = target;
 	asyncObject.data = filename;
 
 	[NSThread detachNewThreadSelector:@selector(addImageWithAsyncObject:) toTarget:self withObject:asyncObject];
 	[asyncObject release];
}*/

CCTexture2D* CCTextureCache::addImage(StreamSource* source)
{
	CCAssert(source != NULL, "TextureCache: fileimage MUST not be NULL");

	CCTexture2D * texture = NULL;
	// Split up directory and filename
	// MUTEX:
	// Needed since addImageAsync calls this method from a different thread

	String pathKey = source->getUrl();
	texture = m_pTextures->objectForKey(pathKey);

	if (texture)
	{
		return texture;
	}

	Ref<GLESTexture> nitTex;

	// TODO: TextureManager not works as planned - so create Texture here.
	// Only so RemoveTexture() works properly.
	bool useLinking = false;
	bool createWithManager = false;

	if (useLinking)
	{
		nitTex = source->link<GLESTexture>(false);
		if (nitTex)
			texture = new CCTexture2D();
	}
	else if (createWithManager)
	{
		nitTex = g_Contents->createFrom<GLESTexture>(source);
		if (nitTex)
	 		texture = new CCTexture2D();
	}
	else
	{
		nitTex = new GLESTexture(new Image(source));
		texture = new CCTexture2D();
	}

	if (texture)
	{
		texture->autorelease();
		if (texture->initWithTexture(nitTex))
			m_pTextures->setObject(texture, pathKey);
		else
			texture = NULL;
	}

	return texture;
}

// TextureCache - Remove

void CCTextureCache::removeAllTextures()
{
	m_pTextures->removeAllObjects();
}

void CCTextureCache::removeUnusedTextures()
{
	StringVector keys = m_pTextures->allKeys();
	StringVector::iterator it;
	for (it = keys.begin(); it != keys.end(); ++it)
	{
		CCTexture2D *value = m_pTextures->objectForKey(*it);
		if (value->retainCount() == 1)
		{
			CCLOG("cocos2d: CCTextureCache: removing unused texture: %s", (*it).c_str());
			m_pTextures->removeObjectForKey(*it);
		}
	}
}

void CCTextureCache::removeTexture(CCTexture2D* texture)
{
	if( ! texture )
		return;

	std::vector<String> keys = m_pTextures->allKeysForObject(texture);

	for (unsigned int i = 0; i < keys.size(); i++)
	{
		m_pTextures->removeObjectForKey(keys[i]);
	}
}

// void CCTextureCache::removeTextureForKey(const char *textureKeyName)
// {
// 	if (textureKeyName == NULL)
// 	{
// 		return;
// 	}
// 
//     string fullPath = CCFileUtils::fullPathFromRelativePath(textureKeyName);
// 	m_pTextures->removeObjectForKey(fullPath);
// }
// 
// CCTexture2D* CCTextureCache::textureForKey(const char* key)
// {
//     String strKey = CCFileUtils::fullPathFromRelativePath(key);
// 	return m_pTextures->objectForKey(strKey);
// }

void CCTextureCache::reloadAllTextures()
{
	// TODO: implement or remove
}

void CCTextureCache::dumpCachedTextureInfo()
{
	unsigned int count = 0;
	unsigned int totalBytes = 0;

	StringVector keys = m_pTextures->allKeys();
	StringVector::iterator iter;
	for (iter = keys.begin(); iter != keys.end(); iter++)
	{
		CCTexture2D *tex = m_pTextures->objectForKey(*iter);
		unsigned int bpp = tex->bitsPerPixelForFormat();
        // Each texture takes up width * height * bytesPerPixel bytes.
		unsigned int bytes = tex->getPixelsWide() * tex->getPixelsHigh() * bpp / 8;
		totalBytes += bytes;
		count++;
		CCLOG("cocos2d: \"%s\" rc=%lu id=%lu %lu x %lu @ %ld bpp => %lu KB",
			   (*iter).c_str(),
			   (long)tex->retainCount(),
			   (long)tex->getNitTexture()->getHandle(),
			   (long)tex->getPixelsWide(),
			   (long)tex->getPixelsHigh(),
			   (long)bpp,
			   (long)bytes / 1024);
	}

	CCLOG("cocos2d: CCTextureCache dumpDebugInfo: %ld textures, for %lu KB (%.2f MB)", (long)count, (long)totalBytes / 1024, totalBytes / (1024.0f*1024.0f));
}

NS_CC_END;

