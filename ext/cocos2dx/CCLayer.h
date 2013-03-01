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

#ifndef __CCLAYER_H__
#define __CCLAYER_H__

#include "CCNode.h"
#include "CCProtocols.h"
#include "CCTouchDelegateProtocol.h"
#include "CCKeypadDelegate.h"

NS_CC_BEGIN;

//
// CCLayer
//
/** @brief CCLayer is a subclass of CCNode that implements the TouchEventsDelegate protocol.

All features from CCNode are valid, plus the following new features:
- It can receive iPhone Touches
- It can receive Accelerometer input
*/

////////////////////////////////////////////////////////////////////////////////

// TODO: Replace CCKeypadDelegate related codes to use this
class CC_DLL CCTouchEvent : public Event
{
public:
	CCTouchEvent() : Sender(NULL), Touches(NULL) { }
	CCTouchEvent(CCObject* sender, CCSet* touches): Sender(sender), Touches(touches) { }

public:
	CCObject* Sender;
	CCSet* Touches;
};

NIT_EVENT_DECLARE(CC_DLL, OnCCTouchBegin, 		CCTouchEvent);
NIT_EVENT_DECLARE(CC_DLL, OnCCTouchMoved, 		CCTouchEvent);
NIT_EVENT_DECLARE(CC_DLL, OnCCTouchEnded, 		CCTouchEvent);
NIT_EVENT_DECLARE(CC_DLL, OnCCTouchCancelled,	CCTouchEvent);

////////////////////////////////////////////////////////////////////////////////

class CC_DLL CCTargetedTouchEvent : public Event
{
public:
	CCTargetedTouchEvent() : Sender(NULL), Touch(NULL) { }
	CCTargetedTouchEvent(CCObject* sender, CCTouch* touch): Sender(sender), Touch(touch) { }

public:
	CCObject* Sender;
	CCTouch* Touch;
};

NIT_EVENT_DECLARE(CC_DLL, OnCCTargetedTouchBegin, 		CCTargetedTouchEvent);
NIT_EVENT_DECLARE(CC_DLL, OnCCTargetedTouchMoved, 		CCTargetedTouchEvent);
NIT_EVENT_DECLARE(CC_DLL, OnCCTargetedTouchEnded, 		CCTargetedTouchEvent);
NIT_EVENT_DECLARE(CC_DLL, OnCCTargetedTouchCancelled,	CCTargetedTouchEvent);

////////////////////////////////////////////////////////////////////////////////

// TODO: Replace CCKeypadDelegate related codes to use this
class CC_DLL CCKeypadEvent : public Event
{
public:
	CCKeypadEvent() : Sender(NULL) { }
	CCKeypadEvent(CCObject* sender) : Sender(sender) { }

public:
	CCObject* Sender;
};

NIT_EVENT_DECLARE(CC_DLL, OnCCKeypadBackClick, CCKeypadEvent);
NIT_EVENT_DECLARE(CC_DLL, OnCCKeypadMenuClick, CCKeypadEvent);

////////////////////////////////////////////////////////////////////////////////

class CC_DLL CCLayer : public CCNode, public CCTouchDelegate, public CCKeypadDelegate
{
public:
	CCLayer();
	virtual ~CCLayer();
	bool init();
	static CCLayer *node(void);

	virtual void onEnter();
	virtual void onExit();
    virtual void onEnterTransitionDidFinish();
	virtual bool ccTouchBegan(CCTouch *pTouch);

	// default implements are used to call script callback if exist
	virtual void ccTouchesBegan(CCSet *pTouches);
	virtual void ccTouchesMoved(CCSet *pTouches);
	virtual void ccTouchesEnded(CCSet *pTouches);
	virtual void ccTouchesCancelled(CCSet *pTouches);

	virtual void destroy(void);
	virtual void keep(void);
	
    virtual void KeypadDestroy();
    virtual void KeypadKeep();
	virtual void keyBackClicked();
	virtual void keyMenuClicked();

	void setModal(bool flag, bool recursive = true);

public:
	EventChannel* channel() { return _channel ? _channel : _channel = new EventChannel(); }

	/** If isTouchEnabled, this method is called onEnter. Override it to change the
	way CCLayer receives touch events.
	( Default: CCTouchDispatcher::sharedDispatcher()->addStandardDelegate(this,0); )
	Example:
	void CCLayer::registerWithTouchDispatcher()
	{
	CCTouchDispatcher::sharedDispatcher()->addTargetedDelegate(this,INT_MIN+1,true);
	}
	@since v0.8.0
	*/
	virtual void registerWithTouchDispatcher(void);

	/** whether or not it will receive Touch events.
	You can enable / disable touch events with this property.
	Only the touches of this node will be affected. This "method" is not propagated to it's children.
	@since v0.8.1
	*/
	CC_PROPERTY(bool, m_bIsTouchEnabled, IsTouchEnabled)

	/** whether or not it will receive keypad events
    You can enable / disable keypad events with this property.
    it's new in cocos2d-x
    */
    CC_PROPERTY(bool, m_bIsKeypadEnabled, IsKeypadEnabled)

protected:
	Ref<EventChannel> _channel;
};
    
// for the subclass of CCLayer, each has to implement the static "node" method 
#define LAYER_NODE_FUNC(layer) \
static layer* node() \
{ \
layer *pRet = new layer(); \
if (pRet && pRet->init()) \
{ \
pRet->autorelease(); \
return pRet; \
} \
else \
{ \
delete pRet; \
pRet = NULL; \
return NULL; \
} \
}; 

#define LAYER_NODE_FUNC_PARAM(layer,__PARAMTYPE__,__PARAM__) \
	static layer* node(__PARAMTYPE__ __PARAM__) \
{ \
	layer *pRet = new layer(); \
	if (pRet && pRet->init(__PARAM__)) \
{ \
	pRet->autorelease(); \
	return pRet; \
	} \
else \
{ \
	delete pRet; \
	pRet = NULL; \
	return NULL; \
	} \
	}; 



//
// CCLayerColor
//
/** @brief CCLayerColor is a subclass of CCLayer that implements the CCRGBAProtocol protocol.

All features from CCLayer are valid, plus the following new features:
- opacity
- RGB colors
*/
class CC_DLL CCLayerColor : public CCLayer , public CCRGBAProtocol, public CCBlendProtocol
{
protected:
	ccVertex2F m_pSquareVertices[4];
	ccColor4B  m_pSquareColors[4];

public:

	CCLayerColor();
	virtual ~CCLayerColor();

	virtual void draw(RenderContext* ctx);
	virtual void setContentSize(const CCSize& var);

	/** creates a CCLayer with color, width and height in Points */
	static CCLayerColor * layerWithColorWidthHeight(const ccColor4B& color, GLfloat width, GLfloat height);
	/** creates a CCLayer with color. Width and height are the window size. */
	static CCLayerColor * layerWithColor(const ccColor4B& color);

	/** initializes a CCLayer with color, width and height in Points */
	virtual bool initWithColorWidthHeight(const ccColor4B& color, GLfloat width, GLfloat height);
	/** initializes a CCLayer with color. Width and height are the window size. */
	virtual bool initWithColor(const ccColor4B& color);

	/** change width in Points*/
	void changeWidth(GLfloat w);
	/** change height in Points*/
	void changeHeight(GLfloat h);
	/** change width and height in Points
	@since v0.8
	*/
	void changeWidthAndHeight(GLfloat w ,GLfloat h);

	/** Opacity: conforms to CCRGBAProtocol protocol */
	CC_PROPERTY(GLubyte, m_cOpacity, Opacity)
	/** Color: conforms to CCRGBAProtocol protocol */
	CC_PROPERTY_PASS_BY_REF(ccColor3B, m_tColor, Color)
	/** BlendFunction. Conforms to CCBlendProtocol protocol */
	CC_PROPERTY(ccBlendFunc, m_tBlendFunc, BlendFunc)

	virtual CCRGBAProtocol* convertToRGBAProtocol() { return (CCRGBAProtocol*)this; }
    LAYER_NODE_FUNC(CCLayerColor);
    
protected:
	virtual void updateColor();
};

//
// CCLayerGradient
//
/** CCLayerGradient is a subclass of CCLayerColor that draws gradients across
the background.

All features from CCLayerColor are valid, plus the following new features:
- direction
- final color
- interpolation mode

Color is interpolated between the startColor and endColor along the given
vector (starting at the origin, ending at the terminus).  If no vector is
supplied, it defaults to (0, -1) -- a fade from top to bottom.

If 'compressedInterpolation' is disabled, you will not see either the start or end color for
non-cardinal vectors; a smooth gradient implying both end points will be still
be drawn, however.

If ' compressedInterpolation' is enabled (default mode) you will see both the start and end colors of the gradient.

@since v0.99.5
*/
class CC_DLL CCLayerGradient : public CCLayerColor
{
public:
    /** Creates a full-screen CCLayer with a gradient between start and end. */
    static CCLayerGradient* layerWithColor(const ccColor4B& start, const ccColor4B& end);

    /** Creates a full-screen CCLayer with a gradient between start and end in the direction of v. */
    static CCLayerGradient* layerWithColor(const ccColor4B& start, const ccColor4B& end, const CCPoint& v);

    /** Initializes the CCLayer with a gradient between start and end. */
    virtual bool initWithColor(const ccColor4B& start, const ccColor4B& end);

    /** Initializes the CCLayer with a gradient between start and end in the direction of v. */
    virtual bool initWithColor(const ccColor4B& start, const ccColor4B& end, const CCPoint& v);

	CC_PROPERTY_PASS_BY_REF(ccColor3B, m_startColor, StartColor)
	CC_PROPERTY_PASS_BY_REF(ccColor3B, m_endColor, EndColor)
    CC_PROPERTY(GLubyte, m_cStartOpacity, StartOpacity)
    CC_PROPERTY(GLubyte, m_cEndOpacity, EndOpacity)
	CC_PROPERTY_PASS_BY_REF(CCPoint, m_AlongVector, Vector)

    /** Whether or not the interpolation will be compressed in order to display all the colors of the gradient both in canonical and non canonical vectors
    Default: YES
    */
    CC_PROPERTY(bool, m_bCompressedInterpolation, IsCompressedInterpolation)

    LAYER_NODE_FUNC(CCLayerGradient);
protected:
    virtual void updateColor();
};

/** @brief CCMultipleLayer is a CCLayer with the ability to multiplex it's children.
Features:
- It supports one or more children
- Only one children will be active a time
*/
class CC_DLL CCLayerMultiplex : public CCLayer
{
protected:
	unsigned int m_nEnabledLayer;
	CCMutableArray<CCLayer *> * m_pLayers;
public:

	CCLayerMultiplex();
	virtual ~CCLayerMultiplex();

	/** creates a CCLayerMultiplex with one or more layers using a variable argument list. */
	static CCLayerMultiplex * layerWithLayers(CCLayer* layer, ... );

    /**
	 * lua script can not init with undetermined number of variables
	 * so add these functinons to be used with lua.
	 */
	static CCLayerMultiplex * layerWithLayer(CCLayer* layer);
	void addLayer(CCLayer* layer);
	bool initWithLayer(CCLayer* layer);

	/** initializes a MultiplexLayer with one or more layers using a variable argument list. */
	bool initWithLayers(CCLayer* layer, va_list params);
	/** switches to a certain layer indexed by n. 
	The current (old) layer will be removed from it's parent with 'cleanup:YES'.
	*/
	void switchTo(unsigned int n);
	/** release the current layer and switches to another layer indexed by n.
	The current (old) layer will be removed from it's parent with 'cleanup:YES'.
	*/
	void switchToAndReleaseMe(unsigned int n);
    
    LAYER_NODE_FUNC(CCLayerMultiplex);
};

NS_CC_END;

#endif // __CCLAYER_H__

