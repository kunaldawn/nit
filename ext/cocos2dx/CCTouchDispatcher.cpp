#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2009      Valentin Milea

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

#include "CCTouchDispatcher.h"
#include "CCTouchHandler.h"
#include "CCMutableArray.h"
#include "CCSet.h"
#include "CCTouch.h"
#include "CCTexture2D.h"
#include "support/ccCArray.h"

#include <assert.h>
#include <algorithm>

/**
 * Used for sort
 */
static bool less(const cocos2d::CCTouchHandler *p1, const cocos2d::CCTouchHandler *p2)
{
	return ((cocos2d::CCTouchHandler*)p1)->getPriority() < ((cocos2d::CCTouchHandler*)p2)->getPriority();
}

NS_CC_BEGIN;

NIT_EVENT_DEFINE(OnCCTouchModal,		CCTouchModalEvent);

bool CCTouchDispatcher::isDispatchEvents(void)
{
	return m_bDispatchEvents;
}

void CCTouchDispatcher::setDispatchEvents(bool bDispatchEvents)
{
	m_bDispatchEvents = bDispatchEvents;
}

static CCTouchDispatcher* s_Instance = NULL;

CCTouchDispatcher* CCTouchDispatcher::sharedDispatcher(void)
{
	return s_Instance;
}

void CCTouchDispatcher::_setInstance(CCTouchDispatcher* instance)
{
	s_Instance = instance;
}

/*
+(id) allocWithZone:(CCZone *)zone
{
	@synchronized(self) {
		CCAssert(sharedDispatcher == nil, @"Attempted to allocate a second instance of a singleton.");
		return [super allocWithZone:zone];
	}
	return nil; // on subsequent allocation attempts return nil
}
*/

bool CCTouchDispatcher::init(void)
{
	m_bDispatchEvents = true;
 	m_pTargetedHandlers = new CCMutableArray<CCTouchHandler*>(8);
 	m_pStandardHandlers = new CCMutableArray<CCTouchHandler*>(4);

 	m_pHandlersToAdd = new CCMutableArray<CCTouchHandler*>(8);
    m_pHandlersToRemove = ccCArrayNew(8);

	m_bToRemove = false;
	m_bToAdd = false;
	m_bToQuit = false;
	m_bLocked = false;

	m_HoldCount = 0;

	m_sHandlerHelperData[CCTOUCHBEGAN].m_type = CCTOUCHBEGAN;
	m_sHandlerHelperData[CCTOUCHMOVED].m_type = CCTOUCHMOVED;
	m_sHandlerHelperData[CCTOUCHENDED].m_type = CCTOUCHENDED;
	m_sHandlerHelperData[CCTOUCHCANCELLED].m_type = CCTOUCHCANCELLED;

	return true;
}

CCTouchDispatcher::~CCTouchDispatcher(void)
{
	CC_SAFE_RELEASE(m_pTargetedHandlers);
	CC_SAFE_RELEASE(m_pStandardHandlers);
	CC_SAFE_RELEASE(m_pHandlersToAdd);

	ccCArrayFree(m_pHandlersToRemove);
	m_pHandlersToRemove = NULL;	
}

//
// handlers management
//
void CCTouchDispatcher::forceAddHandler(CCTouchHandler *pHandler, CCMutableArray<CCTouchHandler*> *pArray)
{
	unsigned int u = 0;

 	CCMutableArray<CCTouchHandler*>::CCMutableArrayIterator iter;
 	for (iter = pArray->begin(); iter != pArray->end(); ++iter)
 	{
 		CCTouchHandler *h = *iter;
         if (h)
         {
 		    if (h->getPriority() < pHandler->getPriority())
 		    {
 			    ++u;
 		    }
 
 		    if (h->getDelegate() == pHandler->getDelegate())
 		    {
 			    assert(0);
 			    return;
 		    }
         }
 	}

	pArray->insertObjectAtIndex(pHandler, u);
}

void CCTouchDispatcher::addStandardDelegate(CCTouchDelegate *pDelegate, int nPriority)
{
	pDelegate->m_eTouchDelegateType = ccTouchDelegateStandardBit;
	
	CCTouchHandler *pHandler = CCStandardTouchHandler::handlerWithDelegate(pDelegate, nPriority);
	if (! m_bLocked)
	{
		forceAddHandler(pHandler, m_pStandardHandlers);
	}
	else
	{
		m_pHandlersToAdd->addObject(pHandler);
		m_bToAdd = true;
	}
}

void CCTouchDispatcher::addTargetedDelegate(CCTouchDelegate *pDelegate, int nPriority, bool bSwallowsTouches)
{
	pDelegate->m_eTouchDelegateType = ccTouchDelegateTargetedBit;
	
	CCTouchHandler *pHandler = CCTargetedTouchHandler::handlerWithDelegate(pDelegate, nPriority, bSwallowsTouches);
	if (! m_bLocked)
	{
		forceAddHandler(pHandler, m_pTargetedHandlers);
	}
	else
	{
		m_pHandlersToAdd->addObject(pHandler);
		m_bToAdd = true;
	}
}

void CCTouchDispatcher::forceRemoveDelegate(CCTouchDelegate *pDelegate)
{
	CCTouchHandler *pHandler;
	CCMutableArray<CCTouchHandler*>::CCMutableArrayIterator  iter;

	// XXX: remove it from both handlers ???
	
	// remove handler from m_pStandardHandlers
	for (iter = m_pStandardHandlers->begin(); iter != m_pStandardHandlers->end(); ++iter)
	{
		pHandler = *iter;
		if (pHandler && pHandler->getDelegate() == pDelegate)
		{
			m_pStandardHandlers->removeObject(pHandler);
			break;
		}
	}


    // remove handler from m_pTargetedHandlers
	for (iter = m_pTargetedHandlers->begin(); iter != m_pTargetedHandlers->end(); ++iter)
	{
		pHandler = *iter;
		if (pHandler && pHandler->getDelegate() == pDelegate)
		{
			m_pTargetedHandlers->removeObject(pHandler);
			break;
		}
	}

	// remove handler from m_Modals;
	m_Modals.erase(pDelegate);
}

void CCTouchDispatcher::removeDelegate(CCTouchDelegate *pDelegate)
{
	if (pDelegate == NULL)
	{
		return;
	}

	if (! m_bLocked)
	{
		forceRemoveDelegate(pDelegate);
	}
	else
	{
		ccCArrayAppendValue(m_pHandlersToRemove, pDelegate);
		m_bToRemove = true;
	}
}

void CCTouchDispatcher::forceRemoveAllDelegates(void)
{
 	m_pStandardHandlers->removeAllObjects();
 	m_pTargetedHandlers->removeAllObjects();
}

void CCTouchDispatcher::removeAllDelegates(void)
{
	if (! m_bLocked)
	{
		forceRemoveAllDelegates();
	}
	else
	{
		m_bToQuit = true;
	}
}

CCTouchHandler* CCTouchDispatcher::findHandler(CCTouchDelegate *pDelegate)
{
	CCMutableArray<CCTouchHandler*>::CCMutableArrayIterator iter;

	for (iter = m_pTargetedHandlers->begin(); iter != m_pTargetedHandlers->end(); ++iter)
	{
		if ((*iter)->getDelegate() == pDelegate)
		{
			return *iter;
		}
	}

	for (iter = m_pStandardHandlers->begin(); iter != m_pStandardHandlers->end(); ++iter)
	{
		if ((*iter)->getDelegate() == pDelegate)
		{
			return *iter;
		}
	} 

	return NULL;
}

void CCTouchDispatcher::rearrangeHandlers(CCMutableArray<CCTouchHandler*> *pArray)
{
	std::sort(pArray->begin(), pArray->end(), ::less);
}

void CCTouchDispatcher::setPriority(int nPriority, CCTouchDelegate *pDelegate)
{
    assert(pDelegate != NULL);

	CCTouchHandler *handler = NULL;

	handler = this->findHandler(pDelegate);

	assert(handler != NULL);

	handler->setPriority(nPriority);

	this->rearrangeHandlers(m_pTargetedHandlers);
	this->rearrangeHandlers(m_pStandardHandlers);
}

//
// dispatch events
//
void CCTouchDispatcher::touches(CCSet *pTouches, unsigned int uIndex)
{
	assert(uIndex >= 0 && uIndex < 4);

	CCSet *pMutableTouches;
	m_bLocked = true;

	// optimization to prevent a mutable copy when it is not necessary
 	unsigned int uTargetedHandlersCount = m_pTargetedHandlers->count();
 	unsigned int uStandardHandlersCount = m_pStandardHandlers->count();
	bool bNeedsMutableSet = (uTargetedHandlersCount && uStandardHandlersCount);

	pMutableTouches = (bNeedsMutableSet ? pTouches->mutableCopy() : pTouches);

	bool hasModals = !m_Modals.empty();

	struct ccTouchHandlerHelperData sHelper = m_sHandlerHelperData[uIndex];
	//
	// process the target handlers 1st
	//
	if (uTargetedHandlersCount > 0)
	{
        CCTouch *pTouch;
		CCSetIterator setIter;
		for (setIter = pTouches->begin(); setIter != pTouches->end(); ++setIter)
		{
			pTouch = (CCTouch *)(*setIter);
			CCTargetedTouchHandler *pHandler;
			CCMutableArray<CCTouchHandler*>::CCMutableArrayIterator arrayIter;
			for (arrayIter = m_pTargetedHandlers->begin(); arrayIter != m_pTargetedHandlers->end(); ++arrayIter)
			/*for (unsigned int i = 0; i < m_pTargetedHandlers->num; ++i)*/
			{
                pHandler = (CCTargetedTouchHandler *)(*arrayIter);

				if (hasModals && m_Modals.find(pHandler->getDelegate()) == m_Modals.end()) 
					continue;

                if (! pHandler)
                {
				   break;
                }

				bool bClaimed = false;
				if (uIndex == CCTOUCHBEGAN)
				{
					bClaimed = pHandler->getDelegate()->ccTouchBegan(pTouch);

					if (bClaimed)
					{
						pHandler->getClaimedTouches()->addObject(pTouch);
					}
				} else
				if (pHandler->getClaimedTouches()->containsObject(pTouch))
				{
					// moved ended cancelled
					bClaimed = true;

					switch (sHelper.m_type)
					{
					case CCTOUCHMOVED:
						pHandler->getDelegate()->ccTouchMoved(pTouch);
						break;
					case CCTOUCHENDED:
						pHandler->getDelegate()->ccTouchEnded(pTouch);
						pHandler->getClaimedTouches()->removeObject(pTouch);
						break;
					case CCTOUCHCANCELLED:
						pHandler->getDelegate()->ccTouchCancelled(pTouch);
						pHandler->getClaimedTouches()->removeObject(pTouch);
						break;
					}
				}

				if (bClaimed && pHandler->isSwallowsTouches())
				{
					if (bNeedsMutableSet)
					{
						pMutableTouches->removeObject(pTouch);
					}

					break;
				}
			}
		}
	}

	//
	// process standard handlers 2nd
	//
	if (uStandardHandlersCount > 0 && pMutableTouches->count() > 0)
	{
		CCMutableArray<CCTouchHandler*>::CCMutableArrayIterator iter;
		CCStandardTouchHandler *pHandler;
		for (iter = m_pStandardHandlers->begin(); iter != m_pStandardHandlers->end(); ++iter)
		{
			pHandler = (CCStandardTouchHandler*)(*iter);

			if (hasModals && m_Modals.find(pHandler->getDelegate()) == m_Modals.end()) 
				continue;

            if (! pHandler)
            {
			    break;
            }

			switch (sHelper.m_type)
			{
			case CCTOUCHBEGAN:
				pHandler->getDelegate()->ccTouchesBegan(pMutableTouches);
				break;
			case CCTOUCHMOVED:
				pHandler->getDelegate()->ccTouchesMoved(pMutableTouches);
				break;
			case CCTOUCHENDED:
				pHandler->getDelegate()->ccTouchesEnded(pMutableTouches);
				break;
			case CCTOUCHCANCELLED:
				pHandler->getDelegate()->ccTouchesCancelled(pMutableTouches);
				break;
			}
		}
	}

	if (bNeedsMutableSet)
	{
		pMutableTouches->release();
	}

	//
	// Optimization. To prevent a [handlers copy] which is expensive
	// the add/removes/quit is done after the iterations
	//
	m_bLocked = false;
	if (m_bToRemove)
	{
		m_bToRemove = false;
		for (unsigned int i = 0; i < m_pHandlersToRemove->num; ++i)
		{
			forceRemoveDelegate((CCTouchDelegate*)m_pHandlersToRemove->arr[i]);
		}
		ccCArrayRemoveAllValues(m_pHandlersToRemove);
	}

	if (m_bToAdd)
	{
		m_bToAdd = false;
 		CCMutableArray<CCTouchHandler*>::CCMutableArrayIterator iter;
         CCTouchHandler *pHandler;
 		for (iter = m_pHandlersToAdd->begin(); iter != m_pHandlersToAdd->end(); ++iter)
 		{
 			pHandler = *iter;
            if (! pHandler)
            {
                break;
            }

			if (pHandler->getDelegate()->getTouchDelegateType() & ccTouchDelegateTargetedBit)
			{				
				forceAddHandler(pHandler, m_pTargetedHandlers);
			}
			else
			{
				forceAddHandler(pHandler, m_pStandardHandlers);
			}
 		}
 
 		m_pHandlersToAdd->removeAllObjects();	
	}

	if (m_bToQuit)
	{
		m_bToQuit = false;
		forceRemoveAllDelegates();
	}
}

void CCTouchDispatcher::touchesBegan(CCSet *touches)
{
	if (m_HoldCount)
		return;

	if (m_bDispatchEvents)
	{
		this->touches(touches, CCTOUCHBEGAN);
	}
}

void CCTouchDispatcher::touchesMoved(CCSet *touches)
{
	if (m_HoldCount) 
		return;

	if (m_bDispatchEvents)
	{
		this->touches(touches, CCTOUCHMOVED);
	}
}

void CCTouchDispatcher::touchesEnded(CCSet *touches)
{
	if (m_HoldCount) 
		return;

	if (m_bDispatchEvents)
	{
		this->touches(touches, CCTOUCHENDED);
	}
}

void CCTouchDispatcher::touchesCancelled(CCSet *touches)
{
	if (m_HoldCount) 
		return;

	if (m_bDispatchEvents)
	{
		this->touches(touches, CCTOUCHCANCELLED);
	}
}

void CCTouchDispatcher::holdUserTouch()
{
	++m_HoldCount;

	if (m_HoldCount == 1)
	{
		// TODO: Release old targeted delegates

	}
}

void CCTouchDispatcher::unholdUserTouch()
{
	--m_HoldCount;
}

void CCTouchDispatcher::SetInputUser(InputUser* user)
{
	if (user == m_InputUser)
		return;

	if (m_InputUser)
	{
		m_InputUser->channel()->unbind(0, this);
	}

	m_InputUser = user;

	if (user)
	{
		user->channel()->bind(nit::Events::OnPointerOn,		this, &CCTouchDispatcher::OnPointerOn);
		user->channel()->bind(nit::Events::OnPointerDrag,		this, &CCTouchDispatcher::OnPointerDrag);
		user->channel()->bind(nit::Events::OnPointerOff,		this, &CCTouchDispatcher::OnPointerOff);
		user->channel()->bind(nit::Events::OnPointerCancel,	this, &CCTouchDispatcher::OnPointerCancel);
	}

	RenderView* view = CCDirector::sharedDirector()->GetRenderView();
	if (view && user)
	{
		user->acquire(view->getInputDevice());
	}
}

void CCTouchDispatcher::OnPointerOn(const InputPointerEvent* evt)
{
	CCSet set;
	CCTouch* pTouch = new CCTouch();

	pTouch->SetTouchInfo(0, evt->getPos().x, evt->getPos().y);

	set.addObject(pTouch);

	m_Touches.insert(std::make_pair(evt->getSource(), pTouch));
	pTouch->retain();

//  	LOG(0, ".. on %.3f %.3f\n", evt->GetPos().x, evt->GetPos().y);

	touchesBegan(&set);
}

void CCTouchDispatcher::OnPointerDrag(const InputPointerEvent* evt)
{
	map<Ref<InputPointer>, CCTouch*>::type::iterator itr = m_Touches.find(evt->getSource());
	if (itr == m_Touches.end())
		return;

	CCSet set;
	CCTouch* pTouch = itr->second;

	pTouch->SetTouchInfo(0, evt->getPos().x, evt->getPos().y);

	set.addObject(pTouch);

// 	LOG(0, ".. drag %.3f %.3f\n", evt->GetPos().x, evt->GetPos().y);

	touchesMoved(&set);
}

void CCTouchDispatcher::OnPointerOff(const InputPointerEvent* evt)
{
	map<Ref<InputPointer>, CCTouch*>::type::iterator itr = m_Touches.find(evt->getSource());
	if (itr == m_Touches.end())
		return;

	CCSet set;
	CCTouch* pTouch = itr->second;
	m_Touches.erase(itr);

	pTouch->SetTouchInfo(0, evt->getPos().x, evt->getPos().y);

	set.addObject(pTouch);

//  	LOG(0, ".. off %.3f %.3f\n", evt->GetPos().x, evt->GetPos().y);

	touchesEnded(&set);

	pTouch->release();
}

void CCTouchDispatcher::OnPointerCancel(const InputPointerEvent* evt)
{
	map<Ref<InputPointer>, CCTouch*>::type::iterator itr = m_Touches.find(evt->getSource());
	if (itr == m_Touches.end())
		return;

	CCSet set;
	CCTouch* pTouch = itr->second;
	m_Touches.erase(itr);

	pTouch->SetTouchInfo(0, evt->getPos().x, evt->getPos().y);

	set.addObject(pTouch);

//  	LOG(0, ".. cancel %.3f %.3f\n", evt->GetPos().x, evt->GetPos().y);

	touchesCancelled(&set);
}

////////////////////////////////////////////////////////////////////////////////

CCTouchInjector::CCTouchInjector(int nViewId)
{
	m_Touch = new CCTouch(nViewId, 0, 0);
	m_Touches = new CCSet();

	m_Touches->addObject(m_Touch);

	m_Enabled = false;
}

CCTouchInjector::~CCTouchInjector()
{
	m_Touch->release();
	m_Touches->release();
}

void CCTouchInjector::cleanup()
{
	if (m_Enabled)
	{
		CCTouchDispatcher* td = CCTouchDispatcher::sharedDispatcher();
		td->injectTouches(m_Touches, CCTOUCHCANCELLED);
		td->unholdUserTouch();
		m_Enabled = false;
	}

	setEnabled(false);

	CCNode::cleanup();
}

void CCTouchInjector::setEnabled(bool flag)
{
	if (m_Enabled == flag) return;

	CCTouchDispatcher* td = CCTouchDispatcher::sharedDispatcher();

	if (flag)
	{
		td->holdUserTouch();
		td->injectTouches(m_Touches, CCTOUCHBEGAN);
	}
	else
	{
		td->injectTouches(m_Touches, CCTOUCHENDED);
		td->unholdUserTouch();
	}
}

void CCTouchInjector::setPosition(const CCPoint& var)
{
	if (m_Enabled)
		CCTouchDispatcher::sharedDispatcher()->injectTouches(m_Touches, CCTOUCHMOVED);
}

CCTouchInjector* CCTouchInjector::injectorWithViewId(int nViewId)
{
	CCTouchInjector* ret = new CCTouchInjector(nViewId);
	ret->autorelease();
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

NS_CC_END;
