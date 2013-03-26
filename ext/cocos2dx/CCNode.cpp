#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2009      Valentin Milea
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

#include "CCNode.h"
#include "CCPointExtension.h"
#include "support/TransformUtils.h"
#include "CCCamera.h"
#include "CCGrid.h"
#include "CCDirector.h"
#include "CCTouch.h"
#include "CCActionManager.h"

#if CC_COCOSNODE_RENDER_SUBPIXEL
#define RENDER_IN_SUBPIXEL
#else
#define RENDER_IN_SUBPIXEL (GLfloat)(int)
#endif

NS_CC_BEGIN;

CCNode::CCNode(void)
: m_nZOrder(0)
, m_fVertexZ(0.0f)
, m_fRotation(0.0f)
, m_fScaleX(1.0f)
, m_fScaleY(1.0f)
, m_tPosition(CCPointZero)
, m_tPositionInPixels(CCPointZero)
// children (lazy allocs)
, m_pChildren(NULL)
// lazy alloc
, m_pCamera(NULL)
, m_pGrid(NULL)
, m_bIsVisible(true)
, m_tAnchorPoint(CCPointZero)
, m_tAnchorPointInPixels(CCPointZero)
, m_tContentSize(CCSizeZero)
, m_tContentSizeInPixels(CCSizeZero)
, m_bIsRunning(false)
, m_pParent(NULL)
// "whole screen" objects. like Scenes and Layers, should set isRelativeAnchorPoint to false
, m_bIsRelativeAnchorPoint(true)
, m_nTag(kCCNodeTagInvalid)
// userData is always inited as nil
, m_pUserData(NULL)
, m_Name(NULL)
, m_bIsTransformDirty(true)
, m_bIsInverseDirty(true)
, m_fSkewX(0.0)
, m_fSkewY(0.0)
, m_IsClipActive(false)
#ifdef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
, m_bIsTransformGLDirty(true)
#endif
{
    // nothing
}
CCNode::~CCNode()
{
	CCLOGINFO( "cocos2d: deallocing" );

	// attributes
	CC_SAFE_RELEASE(m_pCamera);

	CC_SAFE_RELEASE(m_pGrid);

	if(m_pChildren && m_pChildren->count() > 0)
	{
        CCObject* child;
        CCARRAY_FOREACH(m_pChildren, child)
        {
            CCNode* pChild = (CCNode*) child;
            if (pChild)
            {
                pChild->m_pParent = NULL;
            }
        }
	}

	// children
	CC_SAFE_RELEASE(m_pChildren);

	if (m_Name)
		delete m_Name;
}

void CCNode::arrayMakeObjectsPerformSelector(CCArray* pArray, callbackFunc func)
{
	if(pArray && pArray->count() > 0)
	{
        CCObject* child;
        CCARRAY_FOREACH(pArray, child)
        {
            CCNode* pNode = (CCNode*) child;
            if(pNode && (0 != func))
            {
                (pNode->*func)();
            }
        }
	}
}

float CCNode::getSkewX()
{
	return m_fSkewX;
}

void CCNode::setSkewX(float newSkewX)
{
	m_fSkewX = newSkewX;
	m_bIsTransformDirty = m_bIsInverseDirty = true;
#if CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	m_bIsTransformGLDirty = true;
#endif
}

float CCNode::getSkewY()
{
	return m_fSkewY;
	m_bIsTransformDirty = m_bIsInverseDirty = true;
#if CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	m_bIsTransformGLDirty = true;
#endif
}

void CCNode::setSkewY(float newSkewY)
{
	m_fSkewY = newSkewY;

}

/// zOrder getter
int CCNode::getZOrder()
{
	return m_nZOrder;
}

/// zOrder setter : private method
/// used internally to alter the zOrder variable. DON'T call this method manually 
void CCNode::setZOrder(int z)
{
	m_nZOrder = z;
}

/// ertexZ getter
float CCNode::getVertexZ()
{
	return m_fVertexZ / CC_CONTENT_SCALE_FACTOR();
}


/// vertexZ setter
void CCNode::setVertexZ(float var)
{
	m_fVertexZ = var * CC_CONTENT_SCALE_FACTOR();
}


/// rotation getter
float CCNode::getRotation()
{
	return m_fRotation;
}

/// rotation setter
void CCNode::setRotation(float newRotation)
{
	m_fRotation = newRotation;
	m_bIsTransformDirty = m_bIsInverseDirty = true;
#ifdef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	m_bIsTransformGLDirty = true;
#endif
}

/// scale getter
float CCNode::getScale(void)
{
	CCAssert( m_fScaleX == m_fScaleY, "CCNode#scale. ScaleX != ScaleY. Don't know which one to return");
	return m_fScaleX;
}

/// scale setter
void CCNode::setScale(float scale)
{
	m_fScaleX = m_fScaleY = scale;
	m_bIsTransformDirty = m_bIsInverseDirty = true;
#ifdef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	m_bIsTransformGLDirty = true;
#endif
}

/// scaleX getter
float CCNode::getScaleX()
{
	return m_fScaleX;
}

/// scaleX setter
void CCNode::setScaleX(float newScaleX)
{
	m_fScaleX = newScaleX;
	m_bIsTransformDirty = m_bIsInverseDirty = true;
#ifdef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	m_bIsTransformGLDirty = true;
#endif
}

/// scaleY getter
float CCNode::getScaleY()
{
	return m_fScaleY;
}

/// scaleY setter
void CCNode::setScaleY(float newScaleY)
{
	m_fScaleY = newScaleY;
	m_bIsTransformDirty = m_bIsInverseDirty = true;
#ifdef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	m_bIsTransformGLDirty = true;
#endif
}

/// position getter
const CCPoint& CCNode::getPosition()
{
	return m_tPosition;
}

/// position setter
void CCNode::setPosition(const CCPoint& newPosition)
{
	float scale = CC_CONTENT_SCALE_FACTOR();
	m_tPosition = newPosition;

	if (scale == 1.0f)
	{
		m_tPositionInPixels = m_tPosition;
	}
	else
	{
		m_tPositionInPixels = newPosition * scale;
	}

	m_bIsTransformDirty = m_bIsInverseDirty = true;
#ifdef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	m_bIsTransformGLDirty = true;
#endif
}

void CCNode::setPositionInPixels(const CCPoint& newPosition)
{
	float scale = CC_CONTENT_SCALE_FACTOR();
    m_tPositionInPixels = newPosition;

	if (scale == 1.0f)
	{
		m_tPosition = m_tPositionInPixels;
	}
	else
	{
		m_tPosition = newPosition / scale;
	}

	m_bIsTransformDirty = m_bIsInverseDirty = true;

#if CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	m_bIsTransformGLDirty = true;
#endif // CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
}

const CCPoint& CCNode::getPositionInPixels()
{
	return m_tPositionInPixels;
}

/// children getter
CCArray* CCNode::getChildren()
{
	return m_pChildren;
}

/// camera getter: lazy alloc
CCCamera* CCNode::getCamera()
{
	if (!m_pCamera)
	{
		m_pCamera = new CCCamera();
	}
	
	return m_pCamera;
}


/// grid getter
CCGridBase* CCNode::getGrid()
{
	return m_pGrid;
}

/// grid setter
void CCNode::setGrid(CCGridBase* pGrid)
{
	CC_SAFE_RETAIN(pGrid);
	CC_SAFE_RELEASE(m_pGrid);
	m_pGrid = pGrid;
}


/// isVisible getter
bool CCNode::getIsVisible()
{
	return m_bIsVisible;
}

/// isVisible setter
void CCNode::setIsVisible(bool var)
{
	m_bIsVisible = var;
}


/// anchorPoint getter
const CCPoint& CCNode::getAnchorPoint()
{
	return m_tAnchorPoint;
}

void CCNode::setAnchorPoint(const CCPoint& point)
{
	if( ! CCPoint::CCPointEqualToPoint(point, m_tAnchorPoint) ) 
	{
		m_tAnchorPoint = point;
		m_tAnchorPointInPixels = ccp( m_tContentSizeInPixels.width * m_tAnchorPoint.x, m_tContentSizeInPixels.height * m_tAnchorPoint.y );
		m_bIsTransformDirty = m_bIsInverseDirty = true;
#ifdef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
		m_bIsTransformGLDirty = true;
#endif
	}
}

/// anchorPointInPixels getter
const CCPoint& CCNode::getAnchorPointInPixels()
{
	return m_tAnchorPointInPixels;
}

/// contentSize getter
const CCSize& CCNode::getContentSize()
{
	return m_tContentSize;
}

void CCNode::setContentSize(const CCSize& size)
{
	if( ! CCSize::CCSizeEqualToSize(size, m_tContentSize) ) 
	{
		float scale = CC_CONTENT_SCALE_FACTOR();

		m_tContentSize = size;

        if (scale == 1.0f)
        {
            m_tContentSizeInPixels = m_tContentSize;
        }
        else
        {
            m_tContentSizeInPixels = CCSizeMake( size.width * scale, size.height * scale );
        }

		m_tAnchorPointInPixels = ccp( m_tContentSizeInPixels.width * m_tAnchorPoint.x, m_tContentSizeInPixels.height * m_tAnchorPoint.y );
		m_bIsTransformDirty = m_bIsInverseDirty = true;
#ifdef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
		m_bIsTransformGLDirty = true;
#endif
	}
}

void CCNode::setContentSizeInPixels(const CCSize& size)
{
	if (! CCSize::CCSizeEqualToSize(size, m_tContentSizeInPixels))
	{
		float scale = CC_CONTENT_SCALE_FACTOR();

		m_tContentSizeInPixels = size;

		if (scale == 1.0f)
		{
			m_tContentSize = m_tContentSizeInPixels;
		}
		else
		{
			m_tContentSize = CCSizeMake(size.width / scale, size.height / scale);
		}

		m_tAnchorPointInPixels = ccp(m_tContentSizeInPixels.width * m_tAnchorPoint.x, m_tContentSizeInPixels.height * m_tAnchorPoint.y);
		m_bIsTransformDirty = m_bIsInverseDirty = true;

#if CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
		m_bIsTransformGLDirty = true;
#endif // CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	}
}

const CCSize& CCNode::getContentSizeInPixels()
{
	return m_tContentSizeInPixels;
}


// isRunning getter
bool CCNode::getIsRunning()
{
	return m_bIsRunning;
}


/// parent getter
CCNode * CCNode::getParent()
{
	return m_pParent;
}
/// parent setter
void CCNode::setParent(CCNode * var)
{
	m_pParent = var;
}

/// isRelativeAnchorPoint getter
bool CCNode::getIsRelativeAnchorPoint()
{
	return m_bIsRelativeAnchorPoint;
}
/// isRelativeAnchorPoint setter
void CCNode::setIsRelativeAnchorPoint(bool newValue)
{
	m_bIsRelativeAnchorPoint = newValue;
	m_bIsTransformDirty = m_bIsInverseDirty = true;
#ifdef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	m_bIsTransformGLDirty = true;
#endif
}

/// tag getter
int CCNode::getTag()
{
	return m_nTag;
}

/// tag setter
void CCNode::setTag(int var)
{
	m_nTag = var;
}

/// userData getter
void * CCNode::getUserData()
{
	return m_pUserData;
}

/// userData setter
void CCNode::setUserData(void *var)
{
	m_pUserData = var;
}


CCRect CCNode::boundingBox()
{
	CCRect ret = boundingBoxInPixels();
	return ret.ToPoints();
}

CCRect CCNode::boundingBoxInPixels()
{
	CCRect rect = CCRectMake(0, 0, m_tContentSizeInPixels.width, m_tContentSizeInPixels.height);
	return CCRectApplyAffineTransform(rect, nodeToParentTransform());
}


CCNode * CCNode::node(void)
{
	CCNode * pRet = new CCNode();
	pRet->autorelease();
	return pRet;
}


void CCNode::cleanup()
{
	// actions
	this->stopAllActions();

	// timers
	arrayMakeObjectsPerformSelector(m_pChildren, &CCNode::cleanup);
}


char * CCNode::description()
{
	char *ret = new char[100] ;
	sprintf(ret, "<CCNode | Tag = %d>", m_nTag);
	return ret;
}

// lazy allocs
void CCNode::childrenAlloc(void)
{
    m_pChildren = CCArray::arrayWithCapacity(4);
    m_pChildren->retain();
}

CCNode* CCNode::getChildByTag(int aTag)
{
	CCAssert( aTag != kCCNodeTagInvalid, "Invalid tag");

	if(m_pChildren && m_pChildren->count() > 0)
	{
        CCObject* child;
        CCARRAY_FOREACH(m_pChildren, child)
        {
            CCNode* pNode = (CCNode*) child;
			if(pNode && pNode->m_nTag == aTag)
				return pNode;
		}
	}
	return NULL;
}

/* "add" logic MUST only be on this method
* If a class want's to extend the 'addChild' behaviour it only needs
* to override this method
*/
void CCNode::addChild(CCNode *child, int zOrder, int tag)
{	
	CCAssert( child != NULL, "Argument must be non-nil");
	CCAssert( child->m_pParent == NULL, "child already added. It can't be added again");

	if( ! m_pChildren )
	{
		this->childrenAlloc();
	}

	this->insertChild(child, zOrder);

	child->m_nTag = tag;

	child->setParent(this);

	if( m_bIsRunning )
	{
		child->onEnter();
		child->onEnterTransitionDidFinish();
	}
}

void CCNode::addChild(CCNode *child, int zOrder)
{
	CCAssert( child != NULL, "Argument must be non-nil");
	this->addChild(child, zOrder, child->m_nTag);
}

void CCNode::addChild(CCNode *child)
{
	CCAssert( child != NULL, "Argument must be non-nil");
	this->addChild(child, child->m_nZOrder, child->m_nTag);
}

void CCNode::removeFromParentAndCleanup(bool cleanup)
{
	this->m_pParent->removeChild(this,cleanup);
}

/* "remove" logic MUST only be on this method
* If a class want's to extend the 'removeChild' behavior it only needs
* to override this method
*/
void CCNode::removeChild(CCNode* child, bool cleanup)
{
	// explicit nil handling
	if (m_pChildren == NULL)
	{
		return;
	}

	if ( m_pChildren->containsObject(child) )
	{
		this->detachChild(child,cleanup);
	}
}

void CCNode::removeChildByTag(int tag, bool cleanup)
{
	CCAssert( tag != kCCNodeTagInvalid, "Invalid tag");

	CCNode *child = this->getChildByTag(tag);

	if (child == NULL)
	{
		CCLOG("cocos2d: removeChildByTag: child not found!");
	}
	else
	{
		this->removeChild(child, cleanup);
	}
}

void CCNode::removeAllChildrenWithCleanup(bool cleanup)
{
	// not using detachChild improves speed here
	if ( m_pChildren && m_pChildren->count() > 0 )
	{
        CCObject* child;
        CCARRAY_FOREACH(m_pChildren, child)
		{
            CCNode* pNode = (CCNode*) child;
			if (pNode)
			{
				// IMPORTANT:
				//  -1st do onExit
				//  -2nd cleanup
				if(m_bIsRunning)
				{
					pNode->onExit();
				}

				if (cleanup)
				{
					pNode->cleanup();
				}
				// set parent nil at the end
				pNode->setParent(NULL);
			}
		}
		
		m_pChildren->removeAllObjects();
	}
	
}

void CCNode::detachChild(CCNode *child, bool doCleanup)
{
	// IMPORTANT:
	//  -1st do onExit
	//  -2nd cleanup
	if (m_bIsRunning)
	{
		child->onExit();
	}

	// If you don't do cleanup, the child's actions will not get removed and the
	// its scheduledSelectors_ dict will not get released!
	if (doCleanup)
	{
		child->cleanup();
	}

	// set parent nil at the end
	child->setParent(NULL);

	m_pChildren->removeObject(child);
}


// helper used by reorderChild & add
void CCNode::insertChild(CCNode* child, int z)
{
    unsigned int index = 0;
    CCNode* a = (CCNode*) m_pChildren->lastObject();
    if (!a || a->getZOrder() <= z)
    {
        m_pChildren->addObject(child);
    }
    else
    {
        CCObject* pObject;
        CCARRAY_FOREACH(m_pChildren, pObject)
        {
            CCNode* pNode = (CCNode*) pObject;
            if ( pNode && (pNode->m_nZOrder > z ))
            {
                m_pChildren->insertObject(child, index);
                break;
            }
            index++;
        }
    }

    child->setZOrder(z);
}

void CCNode::reorderChild(CCNode *child, int zOrder)
{
	CCAssert( child != NULL, "Child must be non-nil");

	child->retain();
	m_pChildren->removeObject(child);

	insertChild(child, zOrder);
	child->release();
}

 void CCNode::draw(RenderContext* ctx)
 {
#ifndef NIT_SHIPPING

	 if (CCDirector::sharedDirector()->GetDebugBound())
	 {
		 glColor4f(0, 1, 0, 1);
		 // draw bounding box
		 CCSize s = m_tContentSizeInPixels;
		 CCPoint vertices[4] = {
			 ccp(0,0), ccp(s.width,0),
			 ccp(s.width,s.height), ccp(0,s.height)
		 };
		 ctx->drawPoly2d(vertices, 4, false, true);
	 }
#endif

	 // override me
	 // Only use- this function to draw your staff.
	 // DON'T draw your stuff outside this method
 }

#if defined(NIT_IOS)
CCPoint convertToViewport(const CCPoint& gl)
{
	CCDirector* director = CCDirector::sharedDirector();

	CCSize winSize;
	float rx, ry;

	switch (director->getDeviceOrientation())
	{
		case CCDeviceOrientationPortrait:
			return ccp(gl.x, gl.y);
			
		case CCDeviceOrientationPortraitUpsideDown:
			winSize = director->getWinSize();
			rx = winSize.width - gl.x;
			ry = winSize.height - gl.y;
			return ccp(rx, ry);
			
		case CCDeviceOrientationLandscapeLeft:
			winSize = director->getWinSize();
			rx = winSize.width - gl.x;
			return ccp(gl.y, rx);
			
		case CCDeviceOrientationLandscapeRight:
			winSize = director->getWinSize();
			ry = winSize.height - gl.y;
			return ccp(ry, gl.x);
	}
}
#endif

// TODO: temporary code
static bool g_BoundClipping = false;
static CCRect g_ClipRect;

static bool RectOverlap(const CCRect& a, const CCRect& b)
{
	float l0 = a.origin.x;
	float t0 = a.origin.y;
	float r0 = a.origin.x + a.size.width;
	float b0 = a.origin.y + a.size.height;

	float l1 = b.origin.x;
	float t1 = b.origin.y;
	float r1 = b.origin.x + b.size.width;
	float b1 = b.origin.y + b.size.height;

	// The rectangles don't overlap if 
	// one rectangle's minimum in some dimension  
	// is greater than the other's maximum in 
	// that dimension. 

	bool noOverlap = 
		l0 > r1 ||
		l1 > r0 ||
		t0 > b1 ||
		t1 > b0;

	return !noOverlap; 
}

static CCRect CalcAABB(CCNode* node)
{
	const CCSize& s = node->getContentSize();

	// calc aabb rect for node

	CCPoint corners[] = 
	{
		node->convertToWorldSpace(CCPoint(0, 0)),
		node->convertToWorldSpace(CCPoint(s.width, 0)),
		node->convertToWorldSpace(CCPoint(0, s.height)),
		node->convertToWorldSpace(CCPoint(s.width, s.height)),
	};

	float l = Math::MAX_FLOAT;
	float t = Math::MAX_FLOAT;
	float r = Math::MIN_FLOAT;
	float b = Math::MIN_FLOAT;

	for (int i=0; i < COUNT_OF(corners); ++i)
	{
		CCPoint& p = corners[i];

		if (p.x < l) l = p.x;
		if (p.y < t) t = p.y;
		if (p.x > r) r = p.x;
		if (p.y > b) b = p.y;
	}

	return CCRect(l, t, r-l, b-t);
}


void CCNode::visit(RenderContext* ctx)
{
	// quick return if not visible
	if (!m_bIsVisible)
	{
		return;
	}
	glPushMatrix();

 	if (m_pGrid && m_pGrid->isActive())
 	{
 		m_pGrid->beforeDraw(ctx);
 		this->transformAncestors(ctx);
 	}

	this->transform(ctx);

    CCNode* pNode = NULL;

	bool debugClip = false;

#ifndef NIT_SHIPPING
	debugClip = m_IsClipActive && CCDirector::sharedDirector()->GetDebugClip();
#endif

	if (m_IsClipActive)
	{
 		g_BoundClipping = true;
 		g_ClipRect = CalcAABB(this);
	}

	if (m_IsClipActive && !debugClip)
	{
		// TODO: refactor to render context
		glEnable(GL_SCISSOR_TEST);

		float scale = CC_CONTENT_SCALE_FACTOR();

		CCPoint worldpos = this->getParent()->convertToWorldSpace(m_tPosition);

		float cw = m_tContentSize.width * m_fScaleX;
		float ch = m_tContentSize.height * m_fScaleY;
		
		CCPoint p0;
		p0.x = worldpos.x - m_tAnchorPoint.x * cw;
		p0.y = worldpos.y - m_tAnchorPoint.y * ch;

		CCPoint p1 = p0 + CCPoint(cw, ch);
		
#if defined(NIT_IOS)
		p0 = convertToViewport(p0);
		p1 = convertToViewport(p1);
#endif

		if (p0.x > p1.x) std::swap(p0.x, p1.x);
		if (p0.y > p1.y) std::swap(p0.y, p1.y);
		
		if (p0.x < 0) p0.x = 0;
		if (p0.y < 0) p0.y = 0;

		glScissor(GLint(p0.x * scale), GLint(p0.y * scale), 
				  GLint( (p1.x - p0.x) * scale), GLint( (p1.y - p0.y) * scale) );
	}

	unsigned int i = 0;

	if(m_pChildren && m_pChildren->count() > 0)
	{
		// draw children zOrder < 0
        ccArray *arrayData = m_pChildren->data;
        for( ; i < arrayData->num; i++ )
        {
            pNode = (CCNode*) arrayData->arr[i];
			if (pNode == NULL) 
				continue;

			if (pNode->m_nZOrder >= 0)
				break;

			pNode->visit(ctx);
		}
    }

	// self draw
	bool clipped = false;
	if (g_BoundClipping)
	{
		CCRect aabb = CalcAABB(this);
		clipped = !RectOverlap(g_ClipRect, aabb);
	}

	if (!clipped)
		this->draw(ctx);

	// TODO: Handle cases with drawing outside the bound clipping
	// (think about particles)

	// draw children zOrder >= 0 (continue with i)
    if (m_pChildren && m_pChildren->count() > 0)
    {
        ccArray *arrayData = m_pChildren->data;
        for( ; i < arrayData->num; i++ )
        {
            pNode = (CCNode*) arrayData->arr[i];
			if (pNode == NULL) 
				continue;

			pNode->visit(ctx);
		}		
	}

	if (m_IsClipActive && !debugClip)
		glDisable(GL_SCISSOR_TEST);

	if (m_IsClipActive)
		g_BoundClipping = false;

#ifndef NIT_SHIPPING
	debugClip = m_IsClipActive && CCDirector::sharedDirector()->GetDebugClip();

	if (debugClip)
	{
		glColor4f(1, 0, 0, 1);
		// draw bounding box
		CCSize s = m_tContentSizeInPixels;
		CCPoint vertices[4] = {
			ccp(0,0), ccp(s.width,0),
			ccp(s.width,s.height), ccp(0,s.height)
		};
		ctx->drawPoly2d(vertices, 4, false, true);
	}
#endif

	if (m_pGrid && m_pGrid->isActive())
 	{
 		m_pGrid->afterDraw(ctx, this);
	}
 
	glPopMatrix();
}

void CCNode::transformAncestors(RenderContext* ctx)
{
	if( m_pParent != NULL  )
	{
		m_pParent->transformAncestors(ctx);
		m_pParent->transform(ctx);
	}
}

void CCNode::transform(RenderContext* ctx)
{	
	// transformations

#if CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
	// BEGIN alternative -- using cached transform
	//
	if( m_bIsTransformGLDirty ) {
		CCAffineTransform t = this->nodeToParentTransform();
		CGAffineToGL(&t, m_pTransformGL);
		m_bIsTransformGLDirty = false;
	}

	glMultMatrixf(m_pTransformGL);
	if( m_fVertexZ )
	{
		glTranslatef(0, 0, m_fVertexZ);
	}

	// XXX: Expensive calls. Camera should be integrated into the cached affine matrix
	if (m_pCamera && !(m_pGrid && m_pGrid->isActive())) {
		bool translate = (m_tAnchorPointInPixels.x != 0.0f || m_tAnchorPointInPixels.y != 0.0f);

		if( translate )
		{
			ccglTranslate(RENDER_IN_SUBPIXEL(m_tAnchorPointInPixels.x), RENDER_IN_SUBPIXEL(m_tAnchorPointInPixels.y), 0);
		}

		m_pCamera->locate();

		if( translate )
		{
			ccglTranslate(RENDER_IN_SUBPIXEL(-m_tAnchorPointInPixels.x), RENDER_IN_SUBPIXEL(-m_tAnchorPointInPixels.y), 0);
		}
	}


	// END alternative

#else
	// BEGIN original implementation
	// 
	// translate
	if ( m_bIsRelativeAnchorPoint && (m_tAnchorPointInPixels.x != 0 || m_tAnchorPointInPixels.y != 0 ) )
		glTranslatef( RENDER_IN_SUBPIXEL(-m_tAnchorPointInPixels.x), RENDER_IN_SUBPIXEL(-m_tAnchorPointInPixels.y), 0);

	if (m_tAnchorPointInPixels.x != 0 || m_tAnchorPointInPixels.y != 0)
		glTranslatef( RENDER_IN_SUBPIXEL(m_tPositionInPixels.x + m_tAnchorPointInPixels.x), RENDER_IN_SUBPIXEL(m_tPositionInPixels.y + m_tAnchorPointInPixels.y), m_fVertexZ);
	else if ( m_tPositionInPixels.x !=0 || m_tPositionInPixels.y !=0 || m_fVertexZ != 0)
		glTranslatef( RENDER_IN_SUBPIXEL(m_tPositionInPixels.x), RENDER_IN_SUBPIXEL(m_tPositionInPixels.y), m_fVertexZ );

	// rotate
	if (m_fRotation != 0.0f )
		glRotatef( -m_fRotation, 0.0f, 0.0f, 1.0f );

	// skew
	if ( (m_fSkewX != 0.0f) || (m_fSkewY != 0.0f) ) 
	{
		CCAffineTransform skewMatrix = CCAffineTransformMake( 1.0f, tanf(CC_DEGREES_TO_RADIANS(m_fSkewY)), tanf(CC_DEGREES_TO_RADIANS(m_fSkewX)), 1.0f, 0.0f, 0.0f );
		GLfloat	glMatrix[16];
		CGAffineToGL(&skewMatrix, glMatrix);															 
		glMultMatrixf(glMatrix);
	}

	// scale
	if (m_fScaleX != 1.0f || m_fScaleY != 1.0f)
		glScalef( m_fScaleX, m_fScaleY, 1.0f );

	if ( m_pCamera  && !(m_pGrid && m_pGrid->isActive()) )
		m_pCamera->locate(ctx);

	// restore and re-position point
	if (m_tAnchorPointInPixels.x != 0.0f || m_tAnchorPointInPixels.y != 0.0f)
		glTranslatef(RENDER_IN_SUBPIXEL(-m_tAnchorPointInPixels.x), RENDER_IN_SUBPIXEL(-m_tAnchorPointInPixels.y), 0);

	//
	// END original implementation
#endif

}


void CCNode::onEnter()
{
	arrayMakeObjectsPerformSelector(m_pChildren, &CCNode::onEnter);

	CCActionManager::sharedManager()->resumeTarget(this);

	m_bIsRunning = true;
}

void CCNode::onEnterTransitionDidFinish()
{
	arrayMakeObjectsPerformSelector(m_pChildren, &CCNode::onEnterTransitionDidFinish);
}

void CCNode::onExit()
{
	CCActionManager::sharedManager()->pauseTarget(this);

	m_bIsRunning = false;

	arrayMakeObjectsPerformSelector(m_pChildren, &CCNode::onExit);
}
CCAction * CCNode::runAction(CCAction* action)
{
	CCAssert( action != NULL, "Argument must be non-nil");
	CCActionManager::sharedManager()->addAction(action, this, !m_bIsRunning);
	return action;
}

void CCNode::stopAllActions()
{
	CCActionManager::sharedManager()->removeAllActionsFromTarget(this);
}

void CCNode::stopAction(CCAction* action)
{
	CCActionManager::sharedManager()->removeAction(action);
}

void CCNode::stopActionByTag(int tag)
{
	CCAssert( tag != kCCActionTagInvalid, "Invalid tag");
	CCActionManager::sharedManager()->removeActionByTag(tag, this);
}

CCAction * CCNode::getActionByTag(int tag)
{
	CCAssert( tag != kCCActionTagInvalid, "Invalid tag");
	return CCActionManager::sharedManager()->getActionByTag(tag, this);
}

unsigned int CCNode::numberOfRunningActions()
{
	return CCActionManager::sharedManager()->numberOfRunningActionsInTarget(this);
}

CCAffineTransform CCNode::nodeToParentTransform(void)
{
	if (m_bIsTransformDirty) {

		m_tTransform = CCAffineTransformIdentity;

		if( ! m_bIsRelativeAnchorPoint && ! CCPoint::CCPointEqualToPoint(m_tAnchorPointInPixels, CCPointZero) )
		{
			m_tTransform = CCAffineTransformTranslate(m_tTransform, m_tAnchorPointInPixels.x, m_tAnchorPointInPixels.y);
		}

		if(! CCPoint::CCPointEqualToPoint(m_tPositionInPixels, CCPointZero))
		{
			m_tTransform = CCAffineTransformTranslate(m_tTransform, m_tPositionInPixels.x, m_tPositionInPixels.y);
		}

		if(m_fRotation != 0)
		{
			m_tTransform = CCAffineTransformRotate(m_tTransform, -CC_DEGREES_TO_RADIANS(m_fRotation));
		}

		if(m_fSkewX != 0 || m_fSkewY != 0) 
		{
			// create a skewed coordinate system
			CCAffineTransform skew = CCAffineTransformMake(1.0f, tanf(CC_DEGREES_TO_RADIANS(m_fSkewY)), tanf(CC_DEGREES_TO_RADIANS(m_fSkewX)), 1.0f, 0.0f, 0.0f);
			// apply the skew to the transform
			m_tTransform = CCAffineTransformConcat(skew, m_tTransform);
		}

		if(! (m_fScaleX == 1 && m_fScaleY == 1)) 
		{
			m_tTransform = CCAffineTransformScale(m_tTransform, m_fScaleX, m_fScaleY);
		}

		if(! CCPoint::CCPointEqualToPoint(m_tAnchorPointInPixels, CCPointZero))
		{
			m_tTransform = CCAffineTransformTranslate(m_tTransform, -m_tAnchorPointInPixels.x, -m_tAnchorPointInPixels.y);
		}

		m_bIsTransformDirty = false;
	}

	return m_tTransform;
}

CCAffineTransform CCNode::parentToNodeTransform(void)
{
	if ( m_bIsInverseDirty ) {
		m_tInverse = CCAffineTransformInvert(this->nodeToParentTransform());
		m_bIsInverseDirty = false;
	}

	return m_tInverse;
}

CCAffineTransform CCNode::nodeToWorldTransform()
{
	CCAffineTransform t = this->nodeToParentTransform();

	for (CCNode *p = m_pParent; p != NULL; p = p->getParent())
		t = CCAffineTransformConcat(t, p->nodeToParentTransform());

	return t;
}

CCAffineTransform CCNode::worldToNodeTransform(void)
{
	return CCAffineTransformInvert(this->nodeToWorldTransform());
}

CCPoint CCNode::convertToNodeSpace(const CCPoint& worldPoint)
{
	float scale = CC_CONTENT_SCALE_FACTOR();

	CCPoint ret;
	if (scale == 1.0f)
	{
		ret = CCPointApplyAffineTransform(worldPoint, worldToNodeTransform());
	}
	else 
	{
		ret = ccpMult(worldPoint, scale);
		ret = CCPointApplyAffineTransform(ret, worldToNodeTransform());
		ret = ccpMult(ret, 1/scale);
	}

	return ret;
}

CCPoint CCNode::convertToWorldSpace(const CCPoint& nodePoint)
{
	float scale = CC_CONTENT_SCALE_FACTOR();

	CCPoint ret;
	if (scale == 1.0f)
	{
		ret = CCPointApplyAffineTransform(nodePoint, nodeToWorldTransform());
	}
	else 
	{
		ret = ccpMult( nodePoint, scale );
		ret = CCPointApplyAffineTransform(ret, nodeToWorldTransform());
		ret = ccpMult( ret, 1/scale );
	}

	return ret;
}

CCPoint CCNode::convertToNodeSpaceAR(const CCPoint& worldPoint)
{
	float scale = CC_CONTENT_SCALE_FACTOR();

	CCPoint nodePoint = convertToNodeSpace(worldPoint);
	CCPoint anchorInPoints;
	if (scale == 1.0f)
	{
		anchorInPoints = m_tAnchorPointInPixels;
	}
	else
	{
		anchorInPoints = m_tAnchorPointInPixels / scale;
	}

	return ccpSub(nodePoint, anchorInPoints);
}

CCPoint CCNode::convertToWorldSpaceAR(const CCPoint& nodePoint)
{
	float scale = CC_CONTENT_SCALE_FACTOR();

	CCPoint anchorInPoints;
	if (scale == 1.0f)
	{
		anchorInPoints = m_tAnchorPointInPixels;
	}
	else
	{
		anchorInPoints = m_tAnchorPointInPixels / scale;
	}

	CCPoint pt = ccpAdd(nodePoint, anchorInPoints);
	return convertToWorldSpace(pt);
}
CCPoint CCNode::convertToWindowSpace(const CCPoint& nodePoint)
{
	CCPoint worldPoint = this->convertToWorldSpace(nodePoint);
	return CCDirector::sharedDirector()->convertToUI(worldPoint);
}

// convenience methods which take a CCTouch instead of CCPoint
CCPoint CCNode::convertTouchToNodeSpace(CCTouch *touch)
{
	CCPoint point = touch->locationInView(touch->view());
	point = CCDirector::sharedDirector()->convertToGL(point);
	return this->convertToNodeSpace(point);
}
CCPoint CCNode::convertTouchToNodeSpaceAR(CCTouch *touch)
{
	CCPoint point = touch->locationInView(touch->view());
	point = CCDirector::sharedDirector()->convertToGL(point);
	return this->convertToNodeSpaceAR(point);
}

void CCNode::setName(const String& name)
{
	if (name.empty())
	{
		if (m_Name) delete m_Name;
		m_Name = NULL;
	}
	else
	{
		if (m_Name) *m_Name = name;
		else m_Name = new String(name);
	}
}

NS_CC_END;
