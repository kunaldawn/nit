#include "nit2d_pch.h"

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

#include "CCNS.h"
#include "ccMacros.h"
#include "CCTextureCache.h"
#include "CCSpriteFrameCache.h"
#include "CCSpriteFrame.h"
#include "CCSprite.h"
#include "support/TransformUtils.h"
#include "CCString.h"

#include "nit/data/DataLoader.h"

NS_CC_BEGIN;

static CCSpriteFrameCache *pSharedSpriteFrameCache = NULL;

CCSpriteFrameCache* CCSpriteFrameCache::sharedSpriteFrameCache(void)
{
	return pSharedSpriteFrameCache;
}

void CCSpriteFrameCache::_setInstance(CCSpriteFrameCache* instance)
{
	pSharedSpriteFrameCache = instance;
}

bool CCSpriteFrameCache::init(void)
{
	m_pSpriteFrames= new CCDictionary<String, CCSpriteFrame*>();
	m_pSpriteFramesAliases = new CCDictionary<String, CCString*>();
	return true;
}

CCSpriteFrameCache::~CCSpriteFrameCache(void)
{
	CC_SAFE_RELEASE(m_pSpriteFrames);
	CC_SAFE_RELEASE(m_pSpriteFramesAliases);
}

void CCSpriteFrameCache::addSpriteFramesWithRecord(DataRecord* rec, CCTexture2D* texture)
{
	Ref<DataRecord> metadata = rec->get("metadata");
	Ref<DataRecord> frames = rec->get("frames");

	/* Supported Zwoptex Formats:
	 * ZWTCoordinatesFormatOptionXMLLegacy = 0, // Flash Version
	 * ZWTCoordinatesFormatOptionXML1_0 = 1, // Desktop Version 0.0 - 0.4b
	 * ZWTCoordinatesFormatOptionXML1_1 = 2, // Desktop Version 1.0.0 - 1.0.1
	 * ZWTCoordinatesFormatOptionXML1_2 = 3, // Desktop Version 1.0.2+
	 */

	int format = metadata->get("format").Default(0);

	// check the format
	assert(format >= 0 && format <= 3);

	if (frames == NULL) 
	{
		LOG(0, "?? no frames\n");
		return;
	}

	// We use loop here, so cache DataKeys of frequent use
	Ref<DataNamespace> ns				= rec->getNamespace();

	DataKey* k_x 						= ns->get("x");
	DataKey* k_y 						= ns->get("y");
	DataKey* k_w 						= ns->get("width");
	DataKey* k_h 						= ns->get("height");
	DataKey* k_ox 						= ns->get("offsetX");
	DataKey* k_oy 						= ns->get("offsetY");
	DataKey* k_ow 						= ns->get("originalWidth");
	DataKey* k_oh 						= ns->get("originalHeight");
	DataKey* k_frame					= ns->get("frame");
	DataKey* k_rotated					= ns->get("rotated");
	DataKey* k_offset					= ns->get("offset");
	DataKey* k_sourceSize				= ns->get("sourceSize");
	DataKey* k_spriteSize				= ns->get("spriteSize");
	DataKey* k_spriteOffset				= ns->get("spriteOffset");
	DataKey* k_spriteSourceSize			= ns->get("spriteSourceSize");
	DataKey* k_textureRect				= ns->get("textureRect");
	DataKey* k_textureRotated			= ns->get("textureRotated");
	DataKey* k_aliases					= ns->get("aliases");

	for (DataRecord::Iterator itr = frames->begin(); itr != frames->end(); ++itr)
	{
		const String& key = itr->first->getName();
		Ref<DataRecord> frameRec = itr->second;

		if (frameRec == NULL)
			continue;

		CCSpriteFrame* spriteFrame = m_pSpriteFrames->objectForKey(key);

		if (spriteFrame)
			continue;

		if (format == 0) 
		{
			float x = frameRec->get(k_x);
			float y = frameRec->get(k_y);
			float w = frameRec->get(k_w);
			float h = frameRec->get(k_h);
			float ox = frameRec->get(k_ox);
			float oy = frameRec->get(k_oy);
			int ow = frameRec->get(k_ow);
			int oh = frameRec->get(k_oh);

			// check ow/oh
			if(!ow || !oh)
			{
				CCLOG("cocos2d: WARNING: originalWidth/Height not found on the CCSpriteFrame. AnchorPoint won't work as expected. Regenrate the .plist");
			}
			// abs ow/oh
			ow = abs(ow);
			oh = abs(oh);
			// create frame
			spriteFrame = new CCSpriteFrame();

			spriteFrame->initWithTexture(
				texture, 
				CCRectMake(x, y, w, h), 
				false,
				CCPointMake(ox, oy),
				CCSizeMake((float)ow, (float)oh)
			);
		} 
		else if (format == 1 || format == 2) 
		{
			CCRect frame = CCRectFromString(frameRec->get(k_frame).c_str());
			bool rotated = false;

			// rotation
			if (format == 2)
			{
				rotated = frameRec->get(k_rotated);
			}

			CCPoint offset = CCPointFromString(frameRec->get(k_offset).c_str());
			CCSize sourceSize = CCSizeFromString(frameRec->get(k_sourceSize).c_str());

			// create frame
			spriteFrame = new CCSpriteFrame();
			spriteFrame->initWithTexture(texture, 
				frame,
				rotated,
				offset,
				sourceSize
			);
		} 
		else if (format == 3)
		{
			// get values
			CCSize spriteSize = CCSizeFromString(frameRec->get(k_spriteSize).c_str());
			CCPoint spriteOffset = CCPointFromString(frameRec->get(k_spriteOffset).c_str());
			CCSize spriteSourceSize = CCSizeFromString(frameRec->get(k_spriteSourceSize).c_str());
			CCRect textureRect = CCRectFromString(frameRec->get(k_textureRect).c_str());
            bool textureRotated = frameRec->get(k_textureRotated);

			// get aliases
			Ref<DataArray> aliases = frameRec->get(k_aliases).toArray();

			CCString * frameKey = new CCString(key.c_str());
			for (DataArray::Iterator itr = aliases->begin(); itr != aliases->end(); ++itr)
			{
				String oneAlias = itr->toString();
				if (m_pSpriteFramesAliases->objectForKey(oneAlias))
				{
					CCLOG("cocos2d: WARNING: an alias with name %s already exists", oneAlias.c_str());
				}

				m_pSpriteFramesAliases->setObject(frameKey, oneAlias);
			}
			frameKey->release();

			// create frame
			spriteFrame = new CCSpriteFrame();
			spriteFrame->initWithTexture(texture,
				CCRectMake(textureRect.origin.x, textureRect.origin.y, spriteSize.width, spriteSize.height),
				textureRotated,
				spriteOffset,
				spriteSourceSize
			);
		}

		// add sprite frame
		m_pSpriteFrames->setObject(spriteFrame, key);
		spriteFrame->release();
	}
}

void CCSpriteFrameCache::addSpriteFramesWithFile(StreamSource* plistSource, CCTexture2D *pobTexture)
{
	DataValue root = DataValue::load(plistSource);
	Ref<DataRecord> rec = root;

	addSpriteFramesWithRecord(rec, pobTexture);
}

void CCSpriteFrameCache::addSpriteFramesWithFile(StreamSource* plistSource, const char* textureFileName)
{
	StreamSource* textureSource = plistSource->getLocator()->locate(textureFileName);
	CCTexture2D *texture = CCTextureCache::sharedTextureCache()->addImage(textureSource);

	if (texture)
	{
		addSpriteFramesWithFile(plistSource, texture);
	}
	else
	{
		CCLOG("cocos2d: CCSpriteFrameCache: couldn't load texture file. File not found %s", textureFileName);
	}
}

void CCSpriteFrameCache::addSpriteFramesWithFile(StreamSource* plistSource)
{
	DataValue root = DataValue::load(plistSource);

	String texturePath;

	Ref<DataRecord> rec = root;
	Ref<DataRecord> metadata = rec->get("metadata");

	if (metadata)
	{
		// try to read  texture file name from meta data
		texturePath = metadata->get("textureFileName").toString();
	}

	if (texturePath.empty())
	{
		// build texture path by replacing file extension
		texturePath = plistSource->getName();

		// remove .xxx
		size_t startPos = texturePath.find_last_of("."); 
		texturePath = texturePath.erase(startPos);

		// append .png
		texturePath = texturePath.append(".png");

		CCLOG("cocos2d: CCSpriteFrameCache: Trying to use file %s as texture", texturePath.c_str());
	}

	CCTexture2D *texture = CCTextureCache::sharedTextureCache()->addImage(plistSource->getLocator()->locate(texturePath.c_str()));

	if (texture)
	{
		addSpriteFramesWithRecord(rec, texture);
	}
	else
	{
		CCLOG("cocos2d: CCSpriteFrameCache: Couldn't load texture");
	}
}

void CCSpriteFrameCache::addSpriteFrame(CCSpriteFrame *pobFrame, const char *pszFrameName)
{
	m_pSpriteFrames->setObject(pobFrame, String(pszFrameName));
}

void CCSpriteFrameCache::removeSpriteFrames(void)
{
	m_pSpriteFrames->removeAllObjects();
	m_pSpriteFramesAliases->removeAllObjects();
}

void CCSpriteFrameCache::removeUnusedSpriteFrames(void)
{
	m_pSpriteFrames->begin();
	String key = "";
	CCSpriteFrame *spriteFrame = NULL;
	while( (spriteFrame = m_pSpriteFrames->next(&key)) )
	{
		if( spriteFrame->retainCount() == 1 ) 
		{
			CCLOG("cocos2d: CCSpriteFrameCache: removing unused frame: %s", key.c_str());
			m_pSpriteFrames->removeObjectForKey(key);
		}
	}
	m_pSpriteFrames->end();
}


void CCSpriteFrameCache::removeSpriteFrameByName(const char *pszName)
{
	// explicit nil handling
	if( ! pszName )
	{
		return;
	}

	// Is this an alias ?
	CCString *key = (CCString*)m_pSpriteFramesAliases->objectForKey(pszName);

	if (key)
	{
        m_pSpriteFrames->removeObjectForKey(key->m_sString);
		m_pSpriteFramesAliases->removeObjectForKey(key->m_sString);
	}
	else
	{
        m_pSpriteFrames->removeObjectForKey(String(pszName));
	}
}

void CCSpriteFrameCache::removeSpriteFramesFromRecord(DataRecord* rec)
{
	Ref<DataRecord> metadata = rec->get("metadata");
	Ref<DataRecord> frames = rec->get("frames");

	// We use loop here, so cache DataKeys of frequent use
	Ref<DataNamespace> ns				= rec->getNamespace();
	DataKey* k_aliases					= ns->get("aliases");

	StringVector keysToRemove;
	StringVector aliasesToRemove;

	int format = metadata->get("format").Default(0);

	for (DataRecord::Iterator itr = frames->begin(); itr != frames->end(); ++itr)
	{
		const String& key = itr->first->getName();
		if (m_pSpriteFrames->objectForKey(key))
			keysToRemove.push_back(key);

		Ref<DataRecord> frameRec = itr->second;

		if (format == 3)
		{
			Ref<DataArray> aliases = frameRec->get(k_aliases).toArray();
			for (DataArray::Iterator itr = aliases->begin(); itr != aliases->end(); ++itr)
			{
				aliasesToRemove.push_back(itr->toString());
			}
		}
	}

	for (StringVector::iterator itr = keysToRemove.begin(), end = keysToRemove.end(); itr != end; ++itr)
	{
		m_pSpriteFrames->removeObjectForKey(*itr);
	}

	for (StringVector::iterator itr = aliasesToRemove.begin(), end = aliasesToRemove.end(); itr != end; ++itr)
	{
		m_pSpriteFramesAliases->removeObjectForKey(*itr);
	}
}

void CCSpriteFrameCache::removeSpriteFramesFromFile(StreamSource* plistSource)
{
	DataValue root = DataValue::load(plistSource);
	Ref<DataRecord> rec = root;

	removeSpriteFramesFromRecord(rec);
}

void CCSpriteFrameCache::removeSpriteFramesFromTexture(CCTexture2D* texture)
{
	StringVector keysToRemove;

	m_pSpriteFrames->begin();
	String key = "";
	CCDictionary<String, CCObject*> *frameDict = NULL;
	while( (frameDict = (CCDictionary<String, CCObject*>*)m_pSpriteFrames->next(&key)) )
	{
		CCSpriteFrame *frame = m_pSpriteFrames->objectForKey(key);
		if (frame && (frame->getTexture() == texture))
		{
			keysToRemove.push_back(key);
		}
	}
	m_pSpriteFrames->end();

	StringVector::iterator iter;
	for (iter = keysToRemove.begin(); iter != keysToRemove.end(); ++iter)
	{
		m_pSpriteFrames->removeObjectForKey(*iter);
	}
}

CCSpriteFrame* CCSpriteFrameCache::spriteFrameByName(const char *pszName)
{
	CCSpriteFrame *frame = m_pSpriteFrames->objectForKey(String(pszName));
	if (! frame)
	{
		// try alias dictionary
		CCString *key = (CCString*)m_pSpriteFramesAliases->objectForKey(pszName);  
		if (key)
		{
			frame = m_pSpriteFrames->objectForKey(key->m_sString);
			if (! frame)
			{
				CCLOG("cocos2d: CCSpriteFrameCahce: Frame '%s' not found", pszName);
			}
		}
	}
	return frame;
}

const char * CCSpriteFrameCache::valueForKey(const char *key, CCDictionary<String, CCObject*> *dict)
{
	if (dict)
	{
		CCString *pString = (CCString*)dict->objectForKey(String(key));
		return pString ? pString->m_sString.c_str() : "";
	}
	return "";
}

NS_CC_END;
