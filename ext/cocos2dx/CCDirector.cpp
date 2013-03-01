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

#include "CCNS.h"
#include "CCDirector.h"
#include "CCScene.h"
#include "CCMutableArray.h"
#include "ccMacros.h"
#include "CCTouchDispatcher.h"
#include "CCPointExtension.h"
#include "CCTransition.h"
#include "CCTextureCache.h"
#include "CCTransition.h"
#include "CCSpriteFrameCache.h"
#include "CCAutoreleasePool.h"
#include "CCLabelBMFont.h"
#include "CCActionManager.h"
#include "CCLabelTTF.h"
#include "CCConfiguration.h"
#include "CCKeypadDispatcher.h"
#include "CCAnimationCache.h"
#include "CCTouch.h"

#include <string>

#include "nitrender/RenderService.h"

NS_CC_BEGIN;

// singleton stuff
static CCDirector* s_Instance = NULL;

#define kDefaultFPS		60  // 60 frames per second
extern const char* cocos2dVersion(void);

CCDirector* CCDirector::sharedDirector(void)
{
	return s_Instance;
}

void CCDirector::_setInstance(CCDirector* instance)
{
	s_Instance = instance;
}

bool CCDirector::init(void)
{
	CCLOG("cocos2d: %s", cocos2dVersion());

	// scenes
	m_pRunningScene = NULL;
	m_pNextScene = NULL;

	m_pNotificationNode = NULL;

	m_pobScenesStack = new CCMutableArray<CCScene*>();

	// Set default projection (3D)
	m_eProjection = kCCDirectorProjectionDefault;

	// projection delegate if "Custom" projection is used
	m_pProjectionDelegate = NULL;

	// FPS
	m_bDisplayFPS = false;
	m_uTotalFrames = m_uFrames = 0;
	m_pszFPS = new char[10];
	m_fLastUpdateTime = 0;
	m_bNextDeltaTimeZero = true;

	// paused ?
	m_bPaused = false;

	// animating?
	m_bAnimating = false;

	// drawing?
	m_bDrawing = true;
	
	m_obWinSizeInPixels = m_obWinSizeInPoints = CCSizeZero;

	// portrait mode default
	m_eDeviceOrientation = CCDeviceOrientationPortrait;		

	m_fContentScaleFactor = 1.0f;

	m_fAccumDt = 0.0f;
	m_fDeltaTime = 0.0f;
	m_pFPSLabel = NULL;

	m_Timer = new TickTimer();
	m_Scheduler = new TimeScheduler();

	m_DebugBound		= false;
	m_DebugSprite		= false;
	m_DebugSpriteBatch	= false;
	m_DebugLabel		= false;
	m_DebugClip			= false;

	// create autorelease pool
	CCPoolManager::getInstance()->push();

	return true;
}
	
CCDirector::~CCDirector(void)
{
	CCLOGINFO("cocos2d: deallocing %p", this);

	CC_SAFE_RELEASE(m_pFPSLabel);
	CC_SAFE_RELEASE(m_pRunningScene);
	CC_SAFE_RELEASE(m_pNotificationNode);
	CC_SAFE_RELEASE(m_pobScenesStack);

	// pop the autorelease pool
	CCPoolManager::getInstance()->pop();

	// delete fps string
	delete []m_pszFPS;
}

void CCDirector::applyGLDefaults(RenderContext* ctx)
{
	// set other opengl default values
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ctx->setAlphaBlending(true);
	ctx->setDepthTest(true);

	applyProjection(ctx);
	applyOrientation(ctx);
}

void CCDirector::enableDefaultGLStates(RenderContext* ctx)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
}

void CCDirector::disableDefaultGLStates(RenderContext* ctx)
{
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void CCDirector::updateScene(void)
{
	if (!m_bAnimating)
		return;

	// calculate "global" dt
	calculateDeltaTime();

	//tick before glClear: issue #533
	if (! m_bPaused)
	{
		m_Timer->advance(m_fDeltaTime);
		m_Scheduler->advance(m_fDeltaTime);
	}
}

// Draw the SCene
void CCDirector::drawScene(RenderContext* ctx)
{
	if (!m_bDrawing)
		return;

	if (m_pNextScene)
	{
		setNextScene();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 	glPushMatrix();

	applyGLDefaults(ctx);

	// By default enable VertexArray, ColorArray, TextureCoordArray and Texture2D
	enableDefaultGLStates(ctx);

	// draw the scene
    if (m_pRunningScene)
    {
        m_pRunningScene->visit(ctx);
    }

	// draw the notifications node
	if (m_pNotificationNode)
	{
		m_pNotificationNode->visit(ctx);
	}

	if (m_bDisplayFPS)
	{
		showFPS(ctx);
	}

	disableDefaultGLStates(ctx);

	glPopMatrix();

	m_uTotalFrames++;
}

void CCDirector::calculateDeltaTime(void)
{
	ccTime now = g_App->getClock()->getTime();

	if (m_bNextDeltaTimeZero)
	{
		// new delta time
		m_fLastUpdateTime = now;
		m_fDeltaTime = 0;
		m_bNextDeltaTimeZero = false;
	}
	else
	{
		m_fDeltaTime = now - m_fLastUpdateTime;
		m_fDeltaTime = std::max(0.0f, m_fDeltaTime);
		m_fLastUpdateTime = now;
	}

#ifdef DEBUG
	// If we are debugging our code, prevent big delta time
	if(m_fDeltaTime > 0.2f)
	{
		m_fDeltaTime = 1 / 60.0f;
	}
#endif
}

void CCDirector::SetRenderView(RenderView* view)
{
	// Assume that this method is called prior to any other setting methods directly after initialized

	m_RenderView = view;

	if (view == NULL)
		return;

	view->setScale(m_fContentScaleFactor);

	UpdateWinSize();

	// synchronize with default orientation of view
	switch (view->getOrientation())
	{
	case RenderView::ORIENT_PORTRAIT:
		m_eDeviceOrientation = CCDeviceOrientationPortrait;
		break;

	case RenderView::ORIENT_PORTRAIT_FLIP:
		m_eDeviceOrientation = CCDeviceOrientationPortraitUpsideDown;
		break;

	case RenderView::ORIENT_LANDSCAPE:
		m_eDeviceOrientation = CCDeviceOrientationLandscapeLeft;
		break;

	case RenderView::ORIENT_LANDSCAPE_FLIP:
		m_eDeviceOrientation = CCDeviceOrientationLandscapeRight;
		break;
	}

	InputUser* user = CCTouchDispatcher::sharedDispatcher()->GetInputUser();
	if (user)
	{
		user->acquire(view->getInputDevice());
	}
}

void CCDirector::setNextDeltaTimeZero(bool bNextDeltaTimeZero)
{
	m_bNextDeltaTimeZero = bNextDeltaTimeZero;
}

void CCDirector::setProjection(ccDirectorProjection kProjection)
{
	m_eProjection = kProjection;
}

void CCDirector::applyProjection(RenderContext* ctx)
{
	float scale = CC_CONTENT_SCALE_FACTOR(); // TODO: use member variable
	CCSize size = m_obWinSizeInPixels;
	float zeye = this->getZEye();
	switch (m_eProjection)
	{
	case kCCDirectorProjection2D:
		if (m_RenderView)
			ctx->viewport(m_RenderView);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrthof(0, size.width, 0, size.height, -1024 * scale, 1024 * scale);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;

	case kCCDirectorProjection3D:		
		if (m_RenderView)
			ctx->viewport(m_RenderView);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		ctx->perspective(60, (GLfloat)size.width/size.height, zeye * 0.4f, zeye * 3.2f);
			
		glMatrixMode(GL_MODELVIEW);	
		glLoadIdentity();
		ctx->lookAt( size.width/2, size.height/2, zeye,
				 size.width/2, size.height/2, 0,
				 0.0f, 1.0f, 0.0f);				
		break;
			
	case kCCDirectorProjectionCustom:
		if (m_pProjectionDelegate)
		{
			m_pProjectionDelegate->updateProjection();
		}
		break;
			
	default:
		CCLOG("cocos2d: Director: unrecognized projecgtion");
		break;
	}
}

void CCDirector::purgeCachedData(void)
{
    CCLabelBMFont::purgeCachedData();
	CCTextureCache::sharedTextureCache()->removeUnusedTextures();
}

float CCDirector::getZEye(void)
{
    return (m_obWinSizeInPixels.height / 1.1566f);	
}

CCPoint CCDirector::convertToGL(const CCPoint& obPoint)
{
	CCSize s = m_obWinSizeInPoints;
	float newY = s.height - obPoint.y;
	float newX = s.width - obPoint.x;

#if defined(NIT_IOS)
	CCPoint ret = CCPointZero;
	switch (m_eDeviceOrientation)
	{
	case CCDeviceOrientationPortrait:
		ret = ccp(obPoint.x, newY);
		break;
	case CCDeviceOrientationPortraitUpsideDown:
		ret = ccp(newX, obPoint.y);
		break;
	case CCDeviceOrientationLandscapeLeft:
		ret.x = obPoint.y;
		ret.y = obPoint.x;
		break;
	case CCDeviceOrientationLandscapeRight:
		ret.x = newY;
		ret.y = newX;
		break;
	}
#else
	CCPoint ret = ccp(obPoint.x, newY);
#endif
	
//	LOG(0, "-- ToGL: %f, %f -> %f, %f\n", obPoint.x, obPoint.y, ret.x, ret.y);

	return ret;
}

CCPoint CCDirector::convertToUI(const CCPoint& obPoint)
{
	CCSize winSize = m_obWinSizeInPoints;
	float oppositeX = winSize.width - obPoint.x;
	float oppositeY = winSize.height - obPoint.y;

#if defined(NIT_IOS)
	CCPoint uiPoint = CCPointZero;
	switch (m_eDeviceOrientation)
	{
	case CCDeviceOrientationPortrait:
		uiPoint = ccp(obPoint.x, oppositeY);
		break;
	case CCDeviceOrientationPortraitUpsideDown:
		uiPoint = ccp(oppositeX, obPoint.y);
		break;
	case CCDeviceOrientationLandscapeLeft:
		uiPoint = ccp(obPoint.y, obPoint.x);
		break;
	case CCDeviceOrientationLandscapeRight:
		// Can't use oppositeX/Y because x/y are flipped
		uiPoint = ccp(winSize.width - obPoint.y, winSize.height - obPoint.x);
		break;
	}
#else
	CCPoint uiPoint = ccp(obPoint.x, oppositeY);
#endif
	
//	LOG(0, "-- ToUI: %f, %f -> %f, %f\n", obPoint.x, obPoint.y, uiPoint.x, uiPoint.y);

	return uiPoint;
}

CCSize CCDirector::getWinSize(void)
{
	CCSize s = m_obWinSizeInPoints;

#if defined(NIT_IOS)
	if (m_eDeviceOrientation == CCDeviceOrientationLandscapeLeft
		|| m_eDeviceOrientation == CCDeviceOrientationLandscapeRight)
	{
		// swap x,y in landspace mode
		CCSize tmp = s;
		s.width = tmp.height;
		s.height = tmp.width;
	}
#endif

	return s;
}

CCSize CCDirector::getWinSizeInPixels()
{
	CCSize s = m_obWinSizeInPixels;

#if defined(NIT_IOS)
	if (m_eDeviceOrientation == CCDeviceOrientationLandscapeLeft
		|| m_eDeviceOrientation == CCDeviceOrientationLandscapeRight)
	{
		// swap x,y in landspace mode
		CCSize tmp = s;
		s.width = tmp.height;
		s.height = tmp.width;
	}
#endif

	return s;
}

// return the current frame size
CCSize CCDirector::getDisplaySizeInPixels(void)
{
	return m_obWinSizeInPixels;
}

// scene management

void CCDirector::runWithScene(CCScene *pScene)
{
	assert(pScene != NULL);
	assert(m_pRunningScene == NULL);

	pushScene(pScene);
}

void CCDirector::replaceScene(CCScene *pScene)
{
	assert(pScene != NULL);

	unsigned int index = m_pobScenesStack->count();

	if (index == 0)
	{
		runWithScene(pScene);
	}
	else
	{
		m_bSendCleanupToScene = true;
		m_pobScenesStack->replaceObjectAtIndex(index - 1, pScene);

		m_pNextScene = pScene;
	}
}

void CCDirector::pushScene(CCScene *pScene)
{
	assert(pScene);

	m_bSendCleanupToScene = false;

	m_pobScenesStack->addObject(pScene);
	m_pNextScene = pScene;

	if (m_pobScenesStack->count() == 1)
	{
		startAnimation();
	}
}

void CCDirector::popScene(void)
{
	assert(m_pRunningScene != NULL);

	m_pobScenesStack->removeLastObject();
	unsigned int c = m_pobScenesStack->count();

	if (c == 0)
	{
		m_pRunningScene->onExit();
		m_pRunningScene->cleanup();
		m_pRunningScene->release();
		m_pRunningScene = NULL;
		stopAnimation();
	}
	else
	{
		m_bSendCleanupToScene = true;
		m_pNextScene = m_pobScenesStack->getObjectAtIndex(c - 1);
	}
}

void CCDirector::_purgeDirector()
{
	// don't release the event handlers
	// They are needed in case the director is run again
	CCTouchDispatcher::sharedDispatcher()->removeAllDelegates();

    if (m_pRunningScene)
    {
    	m_pRunningScene->onExit();
    	m_pRunningScene->cleanup();
    	m_pRunningScene->release();
    }
    
	m_pRunningScene = NULL;
	m_pNextScene = NULL;

	// remove all objects, but don't release it.
	// runWithScene might be executed after 'end'.
	m_pobScenesStack->removeAllObjects();

	stopAnimation();

	CC_SAFE_RELEASE_NULL(m_pFPSLabel);
	CC_SAFE_RELEASE_NULL(m_pProjectionDelegate);

	// purge bitmap cache
	CCLabelBMFont::purgeCachedData();
}

void CCDirector::setNextScene(void)
{
	ccSceneFlag runningSceneType = ccNormalScene;
	ccSceneFlag newSceneType = m_pNextScene->getSceneType();

	if (m_pRunningScene)
	{
		runningSceneType = m_pRunningScene->getSceneType();
	}

	// If it is not a transition, call onExit/cleanup
 	/*if (! newIsTransition)*/
	if (! (newSceneType & ccTransitionScene))
 	{
         if (m_pRunningScene)
         {
             m_pRunningScene->onExit();
         }
 
 		// issue #709. the root node (scene) should receive the cleanup message too
 		// otherwise it might be leaked.
 		if (m_bSendCleanupToScene && m_pRunningScene)
 		{
 			m_pRunningScene->cleanup();
 		}
 	}

    if (m_pRunningScene)
    {
        m_pRunningScene->release();
    }
    m_pRunningScene = m_pNextScene;
	m_pNextScene->retain();
	m_pNextScene = NULL;

	if (! (runningSceneType & ccTransitionScene) && m_pRunningScene)
	{
		m_pRunningScene->onEnter();
		m_pRunningScene->onEnterTransitionDidFinish();
	}
}

void CCDirector::pause(void)
{
	if (m_bPaused)
	{
		return;
	}

	m_bPaused = true;
}

void CCDirector::resume(void)
{
	if (! m_bPaused)
	{
		return;
	}

	m_bPaused = false;
	m_fDeltaTime = 0;
	m_bNextDeltaTimeZero = true;
}

// display the FPS using a LabelAtlas
// updates the FPS every frame
void CCDirector::showFPS(RenderContext* ctx)
{
	m_uFrames++;
	m_fAccumDt += m_fDeltaTime;

	if (m_fAccumDt > CC_DIRECTOR_FPS_INTERVAL)
	{
		m_fFrameRate = m_uFrames / m_fAccumDt;
		m_uFrames = 0;
		m_fAccumDt = 0;

		sprintf(m_pszFPS, "%.1f", m_fFrameRate);

		if (m_pFPSLabel == NULL || m_pFPSLabel->getFont()->getFace() == NULL)
		{
			if (m_pFPSLabel)
				m_pFPSLabel->release();

			m_pFPSLabel = CCLabelTTF::labelWithString(m_pszFPS);
			m_pFPSLabel->retain();
		}
		else
		{
			m_pFPSLabel->setString(m_pszFPS);
		}
	}

	if (m_pFPSLabel)
		m_pFPSLabel->draw(ctx);
}

/***************************************************
* mobile platforms specific functions
**************************************************/

CGFloat CCDirector::getContentScaleFactor(void)
{
	return m_fContentScaleFactor;
}

void CCDirector::setContentScaleFactor(CGFloat scaleFactor)
{
	if (scaleFactor == m_fContentScaleFactor)
		return;

	m_fContentScaleFactor = scaleFactor;

	if (m_RenderView)
	{
		m_RenderView->setScale(scaleFactor);
		UpdateWinSize();
	}
}

void CCDirector::applyOrientation(RenderContext* ctx)
{
	CCSize s = m_obWinSizeInPixels;
	float w = s.width / 2;
	float h = s.height / 2;

#if defined(NIT_IOS)
	switch (m_eDeviceOrientation)
	{
	case CCDeviceOrientationPortrait:
		// nothing
		break;
	case CCDeviceOrientationPortraitUpsideDown:
		// upside down
		glTranslatef(w,h,0);
		glRotatef(180,0,0,1);
		glTranslatef(-w,-h,0);
		break;
	case CCDeviceOrientationLandscapeRight:
		glTranslatef(w,h,0);
		glRotatef(90,0,0,1);
		glTranslatef(-h,-w,0);
		break;
	case CCDeviceOrientationLandscapeLeft:
		glTranslatef(w,h,0);
		glRotatef(-90,0,0,1);
		glTranslatef(-h,-w,0);
		break;
	}
#endif
}

ccDeviceOrientation CCDirector::getDeviceOrientation(void)
{
	return m_eDeviceOrientation;
}

void CCDirector::setDeviceOrientation(ccDeviceOrientation kDeviceOrientation)
{
	if (m_RenderView)
	{
		RenderView::Orientation o;

		switch (kDeviceOrientation)
		{
		case kCCDeviceOrientationPortrait:
			o = RenderView::ORIENT_PORTRAIT; break;
		case kCCDeviceOrientationPortraitUpsideDown:
			o = RenderView::ORIENT_PORTRAIT_FLIP; break;
		case kCCDeviceOrientationLandscapeLeft:
			o = RenderView::ORIENT_LANDSCAPE; break;
		case kCCDeviceOrientationLandscapeRight:
			o = RenderView::ORIENT_LANDSCAPE_FLIP; break;
		}

		m_RenderView->setOrientation(o);

		UpdateWinSize();
	}
	
	m_eDeviceOrientation = kDeviceOrientation;
}

void CCDirector::UpdateWinSize()
{
	if (m_RenderView)
	{
		float scale = 1.0f / m_fContentScaleFactor;

		m_obWinSizeInPixels = CCSize(m_RenderView->getScaledWidth(), m_RenderView->getScaledHeight());
		m_obWinSizeInPoints = CCSize(m_RenderView->getScaledWidth() * scale, m_RenderView->getScaledHeight() * scale);
	}
}

/***************************************************
* implementation of DisplayLinkDirector
**************************************************/

// should we afford 4 types of director ??
// I think DisplayLinkDirector is enough
// so we now only support DisplayLinkDirector
void CCDirector::startAnimation(void)
{
	m_bNextDeltaTimeZero = true;

	m_bAnimating = true;
}

void CCDirector::stopAnimation(void)
{
	m_bAnimating = false;
}

NS_CC_END;
