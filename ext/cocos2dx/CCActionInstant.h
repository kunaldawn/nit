/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2011 Zynga Inc.

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

#ifndef __CCINSTANT_ACTION_H__
#define __CCINSTANT_ACTION_H__

#include <string>
#include "CCAction.h"

NS_CC_BEGIN;

/** 
@brief Instant actions are immediate actions. They don't have a duration like
the CCIntervalAction actions.
*/ 
class CC_DLL CCActionInstant : public CCFiniteTimeAction //<NSCopying>
{
public:
	CCActionInstant();
	virtual ~CCActionInstant(){}
	// CCAction methods
	virtual CCObject* copyWithZone(CCZone *pZone);
	virtual bool isDone(void);
	virtual void step(ccTime dt);
	virtual void update(ccTime time);
	//CCFiniteTimeAction method
	virtual CCFiniteTimeAction * reverse(void);
};

/** @brief Show the node
*/
class CC_DLL CCShow : public CCActionInstant
{
public:
	CCShow(){}
	virtual ~CCShow(){}
	//super methods
	virtual void startWithTarget(CCNode *pTarget);
	virtual CCFiniteTimeAction * reverse(void);
	virtual CCObject* copyWithZone(CCZone *pZone);
public:
	//override static method
	/** Allocates and initializes the action */
	static CCShow * action();
};



/** 
@brief Hide the node
*/
class CC_DLL CCHide : public CCActionInstant
{
public:
	CCHide(){}
	virtual ~CCHide(){}
	//super methods
	virtual void startWithTarget(CCNode *pTarget);
	virtual CCFiniteTimeAction * reverse(void);
	virtual CCObject* copyWithZone(CCZone *pZone);
public:
	//override static method
	/** Allocates and initializes the action */
	static CCHide * action();
};

/** @brief Toggles the visibility of a node
*/
class CC_DLL CCToggleVisibility : public CCActionInstant
{
public:
	CCToggleVisibility(){}
	virtual ~CCToggleVisibility(){}
	//super method
	virtual void startWithTarget(CCNode *pTarget);
public:
	//override static method
	/** Allocates and initializes the action */
	static CCToggleVisibility * action();
};

/** 
@brief Flips the sprite horizontally
@since v0.99.0
*/
class CC_DLL CCFlipX : public CCActionInstant
{
public:
	CCFlipX()
		:m_bFlipX(false)
	{}
	virtual ~CCFlipX(){}

	/** create the action */
	static CCFlipX * actionWithFlipX(bool x);
	/** init the action */
	bool initWithFlipX(bool x);
	//super methods
	virtual void startWithTarget(CCNode *pTarget);
	virtual CCFiniteTimeAction * reverse(void);
	virtual CCObject* copyWithZone(CCZone *pZone);

protected:
	bool	m_bFlipX;
};

/** 
@brief Flips the sprite vertically
@since v0.99.0
*/
class CC_DLL CCFlipY : public CCActionInstant
{
public:
	CCFlipY()
		:m_bFlipY(false)
	{}
	virtual ~CCFlipY(){}
	
	/** create the action */
	static CCFlipY * actionWithFlipY(bool y);
	/** init the action */
	bool initWithFlipY(bool y);
	//super methods
	virtual void startWithTarget(CCNode *pTarget);
	virtual CCFiniteTimeAction * reverse(void);
	virtual CCObject* copyWithZone(CCZone *pZone);

protected:
	bool	m_bFlipY;
};

/** @brief Places the node in a certain position
*/
class CC_DLL CCPlace : public CCActionInstant //<NSCopying>
{
public:
	CCPlace(){}
	virtual ~CCPlace(){}
	/** creates a Place action with a position */
	static CCPlace * actionWithPosition(const CCPoint& pos);
	/** Initializes a Place action with a position */
	bool initWithPosition(const CCPoint& pos);
	//super methods
	virtual void startWithTarget(CCNode *pTarget);
	virtual CCObject* copyWithZone(CCZone *pZone);
protected:
	CCPoint m_tPosition;
};

/** @brief Calls an event with given handler when action starts
*/
class CC_DLL CCEventCall : public CCActionInstant // <NSCopying>
{
public:
	CCEventCall() { }
	virtual ~CCEventCall() { }
	Ref<Event> m_Event;
	Ref<EventHandler> m_Handler;

	static CCEventCall* actionWithTarget(Event* evt, EventHandler* handler);
	virtual bool initWithTarget(Event* evt, EventHandler* handler);
	virtual void execute();
	virtual void startWithTarget(CCNode* pTarget);
	CCObject* copyWithZone(CCZone *pZone);
	Event* getEvent() { return m_Event; }
	EventHandler* getHandler() { return m_Handler; }
	void setTarget(Event* evt, EventHandler* handler);
};

NS_CC_END;

#endif //__CCINSTANT_ACTION_H__
