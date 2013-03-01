#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2009      Lam Pham

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

#include "CCTransitionRadial.h"
#include "CCDirector.h"
#include "CCRenderTexture.h"
#include "CCActionInstant.h"
#include "CCActionProgressTimer.h"
#include "CCPointExtension.h"

NS_CC_BEGIN;

enum {
	kSceneRadial = 0xc001,
};

CCTransitionRadialCCW::CCTransitionRadialCCW()
{
	m_OutRT = NULL;
}

CCTransitionRadialCCW::~CCTransitionRadialCCW()
{
	if (m_OutRT)
		m_OutRT->release();
}

void CCTransitionRadialCCW::sceneOrder()
{
	m_bIsInSceneOnTop = false;
}

CCProgressTimerType CCTransitionRadialCCW::radialType()
{
	return kCCProgressTimerTypeRadialCCW;
}

void CCTransitionRadialCCW::onEnter()
{
	CCTransitionScene::onEnter();
	// create a transparent color layer
	// in which we are going to add our rendertextures
	CCSize size = CCDirector::sharedDirector()->getWinSize();

	// create the second render texture for outScene
	m_OutRT = CCRenderTexture::renderTextureWithWidthAndHeight((int)size.width, (int)size.height);

	if (NULL == m_OutRT)
		return;
	
	m_OutRT->retain();

	m_OutRT->getSprite()->setAnchorPoint(ccp(0.5f,0.5f));
	m_OutRT->setPosition(ccp(size.width/2, size.height/2));
	m_OutRT->setAnchorPoint(ccp(0.5f,0.5f));

	//	We need the texture in RenderTexture.
	CCProgressTimer *outNode = CCProgressTimer::progressWithTexture(m_OutRT->getSprite()->getTexture());
	// but it's flipped upside down so we flip the sprite
	outNode->getSprite()->setFlipY(true);

	// fix content scale factor for radial texture
	CCRect rect = CCRectZero;
	rect.size = m_OutRT->getSprite()->getTexture()->getContentSize();
	outNode->getSprite()->setTextureRect(rect, false);
	float scale = 1.0f / CC_CONTENT_SCALE_FACTOR();
	rect.size.width *= scale;
	rect.size.height *= scale;
	outNode->setContentSize(rect.size);

	//	Return the radial type that we want to use
	outNode->setType(radialType());
	outNode->setPercentage(100.f);
	outNode->setPosition(ccp(size.width/2, size.height/2));
	outNode->setAnchorPoint(ccp(0.5f,0.5f));

	// create the blend action
	CCAction * layerAction = CCSequence::actions
	(
		CCProgressFromTo::actionWithDuration(m_fDuration, 100.0f, 0.0f),
		CCEventCall::actionWithTarget(NULL, createEventHandler(this, &CCTransitionScene::_finish)),
		NULL
	);
	// run the blend action
	outNode->runAction(layerAction);

	// add the layer (which contains our two rendertextures) to the scene
	this->addChild(outNode, 2, kSceneRadial);
}


// clean up on exit
void CCTransitionRadialCCW::onExit()
{
	if (m_OutRT)
	{
		m_OutRT->release();
		m_OutRT = NULL;
	}

	// remove our layer and release all containing objects 
	this->removeChildByTag(kSceneRadial, false);
	CCTransitionScene::onExit();
}

void CCTransitionRadialCCW::draw(RenderContext* ctx)
{
	if (m_OutRT)
	{
		// render outScene to its texturebuffer
		m_OutRT->clear(ctx, 0, 0, 0, 1);
		m_OutRT->begin(ctx);
		m_pOutScene->visit(ctx);
		m_OutRT->end(ctx);
	}

	CCScene::draw(ctx);
	m_pInScene->visit(ctx);
}

CCTransitionRadialCCW* CCTransitionRadialCCW::transitionWithDuration(ccTime t, CCScene* scene)
{
    CCTransitionRadialCCW* pScene = new CCTransitionRadialCCW();
    pScene->initWithDuration(t, scene);
    pScene->autorelease();

    return pScene;
}

CCProgressTimerType CCTransitionRadialCW::radialType()
{
	return kCCProgressTimerTypeRadialCW;
}

CCTransitionRadialCW* CCTransitionRadialCW::transitionWithDuration(ccTime t, CCScene* scene)
{
    CCTransitionRadialCW* pScene = new CCTransitionRadialCW();
    pScene->initWithDuration(t, scene);
    pScene->autorelease();

    return pScene;
}

NS_CC_END;
