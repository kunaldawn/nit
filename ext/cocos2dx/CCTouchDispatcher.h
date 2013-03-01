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

#ifndef __TOUCH_DISPATCHER_CCTOUCH_DISPATCHER_H__
#define __TOUCH_DISPATCHER_CCTOUCH_DISPATCHER_H__

#include "CCTouchDelegateProtocol.h"
#include "CCObject.h"
#include "CCMutableArray.h"

#include "nit/input/InputUser.h"

NS_CC_BEGIN;

typedef enum
{
	ccTouchSelectorBeganBit = 1 << 0,
	ccTouchSelectorMovedBit = 1 << 1,
	ccTouchSelectorEndedBit = 1 << 2,
	ccTouchSelectorCancelledBit = 1 << 3,
	ccTouchSelectorAllBits = ( ccTouchSelectorBeganBit | ccTouchSelectorMovedBit | ccTouchSelectorEndedBit | ccTouchSelectorCancelledBit),
} ccTouchSelectorFlag;


enum {
	CCTOUCHBEGAN,
	CCTOUCHMOVED,
	CCTOUCHENDED,
	CCTOUCHCANCELLED,
	
	ccTouchMax,
};

class CCSet;

struct ccTouchHandlerHelperData {
	// we only use the type
//	void (StandardTouchDelegate::*touchesSel)(CCSet*, CCEvent*);
//	void (TargetedTouchDelegate::*touchSel)(NSTouch*, CCEvent*);
	int  m_type;
};


class CC_DLL EGLTouchDelegate
{
public:
	virtual void touchesBegan(CCSet* touches) = 0;
	virtual void touchesMoved(CCSet* touches) = 0;
	virtual void touchesEnded(CCSet* touches) = 0;
	virtual void touchesCancelled(CCSet* touches) = 0;

    virtual ~EGLTouchDelegate() {}
};

////////////////////////////////////////////////////////////////////////////////

class CC_DLL CCTouchInjector : public CCNode
{
public:
	static CCTouchInjector*				injectorWithViewId(int nViewId);

public:
	bool								isEnabled()								{ return m_Enabled; }
	void								setEnabled(bool flag);

	virtual void						setPosition(const CCPoint& var);
	virtual void						cleanup();

protected:
	CCTouchInjector(int nViewId);
	virtual ~CCTouchInjector();

private:
	CCTouch*							m_Touch;
	CCSet*								m_Touches;
	bool								m_Enabled;
};

////////////////////////////////////////////////////////////////////////////////

class CCLayer;
class CC_DLL CCTouchModalEvent : public Event
{
public:
	CCTouchModalEvent(): Layer(NULL), Modal(false) { }
	CCTouchModalEvent(CCLayer* layer, bool modal): Layer(layer), Modal(modal) { }

public:
	CCLayer*	Layer;
	bool		Modal;
};

NIT_EVENT_DECLARE(CC_DLL, OnCCTouchModal, 		CCTouchModalEvent);

////////////////////////////////////////////////////////////////////////////////

class CCTouchHandler;
struct _ccCArray;
/** @brief CCTouchDispatcher.
 Singleton that handles all the touch events.
 The dispatcher dispatches events to the registered TouchHandlers.
 There are 2 different type of touch handlers:
   - Standard Touch Handlers
   - Targeted Touch Handlers
 
 The Standard Touch Handlers work like the CocoaTouch touch handler: a set of touches is passed to the delegate.
 On the other hand, the Targeted Touch Handlers only receive 1 touch at the time, and they can "swallow" touches (avoid the propagation of the event).
 
 Firstly, the dispatcher sends the received touches to the targeted touches.
 These touches can be swallowed by the Targeted Touch Handlers. If there are still remaining touches, then the remaining touches will be sent
 to the Standard Touch Handlers.

 @since v0.8.0
 */
class CC_DLL CCTouchDispatcher : public WeakSupported, public EGLTouchDelegate, public IEventSink
{
public:
	~CCTouchDispatcher();
	bool init(void);
	CCTouchDispatcher() 
        : m_pTargetedHandlers(NULL)
        , m_pStandardHandlers(NULL)
		, m_pHandlersToAdd(NULL)
		, m_pHandlersToRemove(NULL)
		
	{}

public:
	InputUser* GetInputUser() { return m_InputUser; }
	void SetInputUser(InputUser* user);

public: // IEventSink Impl
	virtual bool isEventActive() { return isDispatchEvents(); }
	virtual bool isDisposed() { return false; }

public:
	/** Whether or not the events are going to be dispatched. Default: true */
    bool isDispatchEvents(void);
	void setDispatchEvents(bool bDispatchEvents);

	/** Adds a standard touch delegate to the dispatcher's list.
	 See StandardTouchDelegate description.
	 IMPORTANT: The delegate will be retained.
	 */
	void addStandardDelegate(CCTouchDelegate *pDelegate, int nPriority);

	/** Adds a targeted touch delegate to the dispatcher's list.
	 See TargetedTouchDelegate description.
	 IMPORTANT: The delegate will be retained.
	 */
	void addTargetedDelegate(CCTouchDelegate *pDelegate, int nPriority, bool bSwallowsTouches);

	/** Removes a touch delegate.
	 The delegate will be released
	 */
	void removeDelegate(CCTouchDelegate *pDelegate);

	/** Removes all touch delegates, releasing all the delegates */
	void removeAllDelegates(void);

	/** Changes the priority of a previously added delegate. The lower the number,
    the higher the priority */
	void setPriority(int nPriority, CCTouchDelegate *pDelegate);

	void holdUserTouch();
	void unholdUserTouch();
	int getHoldCount()					{ return m_HoldCount; }

	virtual void touchesBegan(CCSet* touches);
	virtual void touchesMoved(CCSet* touches);
	virtual void touchesEnded(CCSet* touches);
	virtual void touchesCancelled(CCSet* touches);

	void injectTouches(CCSet* pTouches, unsigned int uIndex) { touches(pTouches, uIndex); }

public:
	EventChannel* Channel() { return m_Channel ? m_Channel : m_Channel = new EventChannel(); }

public:
	/** singleton of the CCTouchDispatcher */
	static CCTouchDispatcher* sharedDispatcher();
	static void _setInstance(CCTouchDispatcher* instance);

	CCTouchHandler* findHandler(CCTouchDelegate *pDelegate);

public:
	void AddModal(CCTouchDelegate* delegate)		{ m_Modals.insert(delegate); }
	void RemoveModal(CCTouchDelegate* delegate)		{ m_Modals.erase(delegate); }
	size_t GetModalCount()							{ return m_Modals.size(); }
	void ClearModals()								{ m_Modals.clear(); }

protected:
	void touches(CCSet *pTouches, unsigned int uIndex);

	void forceRemoveDelegate(CCTouchDelegate *pDelegate);
	void forceAddHandler(CCTouchHandler *pHandler, CCMutableArray<CCTouchHandler*> *pArray);
	void forceRemoveAllDelegates(void);
	void rearrangeHandlers(CCMutableArray<CCTouchHandler*> *pArray);

protected:
 	CCMutableArray<CCTouchHandler*> *m_pTargetedHandlers;
 	CCMutableArray<CCTouchHandler*> *m_pStandardHandlers;

	int m_HoldCount;
	bool m_bLocked;
	bool m_bToAdd;
	bool m_bToRemove;
 	CCMutableArray<CCTouchHandler*> *m_pHandlersToAdd;
	set<CCTouchDelegate*>::type m_Modals;
	struct _ccCArray *m_pHandlersToRemove;
	bool m_bToQuit;
	bool m_bDispatchEvents;

	// 4, 1 for each type of event
	struct ccTouchHandlerHelperData m_sHandlerHelperData[ccTouchMax];

protected:
	Ref<InputUser> m_InputUser;
	Ref<EventChannel> m_Channel;

	map<Ref<InputPointer>, CCTouch*>::type m_Touches;

private:
	void OnPointerOn(const InputPointerEvent* evt);
	void OnPointerDrag(const InputPointerEvent* evt);
	void OnPointerOff(const InputPointerEvent* evt);
	void OnPointerCancel(const InputPointerEvent* evt);
};

NS_CC_END;

#endif // __TOUCH_DISPATCHER_CCTOUCH_DISPATCHER_H__
