#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010 cocos2d-x.org

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


#include "CCObject.h"
#include "CCAutoreleasePool.h"
#include <assert.h>

NS_CC_BEGIN;

CCObject* CCObject::copyWithZone(CCZone *pZone)
{
    CC_UNUSED_PARAM(pZone);
	assert(0);
	return NULL;
}


CCObject::CCObject(void)
{
	static unsigned int uObjectCount = 0;

	m_uID = ++uObjectCount;

	// when the object is created, the refrence count of it is 1
	incRefCount();
	m_bManaged = false;
}

CCObject::~CCObject(void)
{
	// if the object is managed, we should remove it
	// from pool manager
	if (m_bManaged)
	{
		CCPoolManager::getInstance()->removeObject(this);
	}
}

CCObject* CCObject::copy()
{
 	return copyWithZone(NULL);
}

CCObject* CCObject::autorelease(void)
{
	CCPoolManager::getInstance()->addObject(this);

	m_bManaged = true;
	return this;
}

bool CCObject::isEqual(const CCObject *pObject)
{
	return this == pObject;
}

NS_CC_END;
