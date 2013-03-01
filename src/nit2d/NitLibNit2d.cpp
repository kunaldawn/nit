/// nit - Noriter Framework
/// A Cross-platform Open Source Integration for Game-oriented Apps
///
/// http://www.github.com/ellongrey/nit
///
/// Copyright (c) 2013 by Jun-hyeok Jang
/// 
/// (see each file to see the different copyright owners)
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// Author: ellongrey

#include "nit2d_pch.h"

#include "nit2d/CocosService.h"
#include "nit2d/NitLibCocos.h"

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

#include "CCKeypadDispatcher.h"
#include "CCIMEDispatcher.h"
#include "CCConfiguration.h"

#include "CCTouchDispatcher.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT2D_API, nit::CocosService, Service, incRefCount, decRefCount);

class NB_CocosService : TNitClass<CocosService>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(director),
			PROP_ENTRY_R(touchDispatcher),
			PROP_ENTRY_R(keypadDispatcher),
			PROP_ENTRY_R(imeDispatcher),
			PROP_ENTRY_R(textureCache),
			PROP_ENTRY_R(spriteFrameCache),
			PROP_ENTRY_R(animationCache),
			PROP_ENTRY_R(configuration),
			PROP_ENTRY_R(actionManager),
			PROP_ENTRY_R(fontManager),
			PROP_ENTRY_R(renderView),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(director)				{ return push(v, self(v)->getDirector()); }
	NB_PROP_GET(touchDispatcher)		{ return push(v, self(v)->getTouchDispatcher()); }
	NB_PROP_GET(keypadDispatcher)		{ return push(v, self(v)->getKeypadDispatcher()); }
	NB_PROP_GET(imeDispatcher)			{ return push(v, self(v)->getImeDispatcher()); }
	NB_PROP_GET(textureCache)			{ return push(v, self(v)->getTextureCache()); }
	NB_PROP_GET(spriteFrameCache)		{ return push(v, self(v)->getSpriteFrameCache()); }
	NB_PROP_GET(animationCache)			{ return push(v, self(v)->getAnimationCache()); }
	NB_PROP_GET(configuration)			{ return push(v, self(v)->getConfiguration()); }
	NB_PROP_GET(actionManager)			{ return push(v, self(v)->getActionManager()); }
	NB_PROP_GET(fontManager)			{ return push(v, self(v)->getFontManager()); }
	NB_PROP_GET(renderView)				{ return push(v, self(v)->getRenderView()); }
};

////////////////////////////////////////////////////////////////////////////////

class NIT2D_API CCScriptNode : public TScriptEventSink<CCScriptNode, cocos2d::CCNode>, public cocos2d::CCTouchDelegate, public IScriptRef
{
private:
	typedef cocos2d::CCNode				inherited;

protected:
	CCScriptNode();
	virtual ~CCScriptNode();

public:
	static CCScriptNode*				nodeWithPeer(ScriptPeer* peer);
	bool								initWithPeer(ScriptPeer* peer);

public:									// Touch & Event
	bool								isTouchEnabled()						{ return _touchEnabled; }
	bool								isTouchTargeted()						{ return _touchTargeted; }

	void								setTouchEnabled(bool flag);
	void								setTouchTargeted(bool flag);

	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

	void								setModal(bool flag, bool recursive = true);

protected:								// CCTouchDelegate implementation
	virtual void						destroy(void)							{ release(); }
	virtual void						keep(void)								{ retain(); }

	// targeted
	virtual bool						ccTouchBegan(cocos2d::CCTouch *pTouch);
	virtual void						ccTouchMoved(cocos2d::CCTouch *pTouch);
	virtual void						ccTouchEnded(cocos2d::CCTouch *pTouch);
	virtual void						ccTouchCancelled(cocos2d::CCTouch *pTouch);

	// standard
	virtual void						ccTouchesBegan(cocos2d::CCSet *pTouches);
	virtual void						ccTouchesMoved(cocos2d::CCSet *pTouches);
	virtual void						ccTouchesEnded(cocos2d::CCSet *pTouches);
	virtual void						ccTouchesCancelled(cocos2d::CCSet *pTouches);

public:									// IScriptRef impl
	virtual RefCounted*					_ref()									{ return inherited::_ref(); }
	virtual WeakRef*					_weak()									{ return inherited::_weak(); }
	virtual ScriptPeer*					getScriptPeer()							{ return _peer; }
	virtual String						getDebugString()						{ return description(); }

public:									// IEventSink override
	virtual bool						isEventActive()							{ return _peer && m_bIsRunning; }
	virtual bool						isDisposed()							{ return _peer == NULL; }

public:									// CCNode's script overridables
	virtual void						onEnter();

	virtual void						onExit();

	virtual void						onEnterTransitionDidFinish();
	virtual void						cleanup();

private:
	Ref<ScriptPeer>						_peer;
	bool								_touchEnabled : 1;
	bool								_touchTargeted : 1;

	Ref<EventChannel>					_channel;

	void								registerWithTouchDispatcher();
	void								unregisterWithTouchDispatcher();
};

////////////////////////////////////////////////////////////////////////////////

CCScriptNode::CCScriptNode()
{
	_touchEnabled = true;
	_touchTargeted = true;
}

CCScriptNode::~CCScriptNode()
{

}

CCScriptNode* CCScriptNode::nodeWithPeer(ScriptPeer* peer)
{
	CCScriptNode* ret = new CCScriptNode();
	if (ret && ret->initWithPeer(peer))
	{
		ret->autorelease();
		return ret;
	}
	else
	{
		delete ret;
		return NULL;
	}
}

bool CCScriptNode::initWithPeer(ScriptPeer* peer)
{
	if (peer == NULL) return false;

	_peer = peer;

	return true;
}

void CCScriptNode::onEnter()
{
	inherited::onEnter();

	if (_touchEnabled)
		registerWithTouchDispatcher();

	if (_peer)
		_peer->callMethod("onEnter", 0);
}

void CCScriptNode::onExit()
{
	if (_peer)
		_peer->callMethod("onExit", 0);

	if (_touchEnabled)
		unregisterWithTouchDispatcher();

	inherited::onExit();
}

void CCScriptNode::onEnterTransitionDidFinish()
{
	if (_peer)
		_peer->callMethod("onEnterTransitionDidFinish", 0);

	inherited::onEnterTransitionDidFinish();
}

void CCScriptNode::cleanup()
{
	if (_peer)
		_peer->callMethod("onCleanup", 0);

	inherited::cleanup();
}

bool CCScriptNode::ccTouchBegan(cocos2d::CCTouch *pTouch)
{
	if (!IsRealVisible())
		return false;

	if (_channel)
	{
		Ref<cocos2d::CCTargetedTouchEvent> evt = new cocos2d::CCTargetedTouchEvent(this, pTouch);
		_channel->send(cocos2d::Events::OnCCTargetedTouchBegin, evt);
		return evt->isConsumed();
	}

	return false;
}

void CCScriptNode::ccTouchMoved(cocos2d::CCTouch *pTouch)
{
	if (_channel)
		_channel->send(cocos2d::Events::OnCCTargetedTouchMoved, new cocos2d::CCTargetedTouchEvent(this, pTouch));
}

void CCScriptNode::ccTouchEnded(cocos2d::CCTouch *pTouch)
{
	if (_channel)
		_channel->send(cocos2d::Events::OnCCTargetedTouchEnded, new cocos2d::CCTargetedTouchEvent(this, pTouch));
}

void CCScriptNode::ccTouchCancelled(cocos2d::CCTouch *pTouch)
{
	if (_channel)
		_channel->send(cocos2d::Events::OnCCTargetedTouchCancelled, new cocos2d::CCTargetedTouchEvent(this, pTouch));
}

void CCScriptNode::ccTouchesBegan(cocos2d::CCSet *pTouches)
{
	if (!IsRealVisible())
		return;

	if (_channel)
		_channel->send(cocos2d::Events::OnCCTouchBegin, new cocos2d::CCTouchEvent(this, pTouches));
}

void CCScriptNode::ccTouchesMoved(cocos2d::CCSet *pTouches)
{
	if (_channel)
		_channel->send(cocos2d::Events::OnCCTouchMoved, new cocos2d::CCTouchEvent(this, pTouches));
}

void CCScriptNode::ccTouchesEnded(cocos2d::CCSet *pTouches)
{
	if (_channel)
		_channel->send(cocos2d::Events::OnCCTouchEnded, new cocos2d::CCTouchEvent(this, pTouches));
}

void CCScriptNode::ccTouchesCancelled(cocos2d::CCSet *pTouches)
{
	if (_channel)
		_channel->send(cocos2d::Events::OnCCTouchCancelled, new cocos2d::CCTouchEvent(this, pTouches));
}

void CCScriptNode::setTouchEnabled(bool flag)
{
	if (_touchEnabled == flag)
		return;

	_touchEnabled = flag;

	if (!m_bIsRunning) return;

	if (flag)
		registerWithTouchDispatcher();
	else
		unregisterWithTouchDispatcher();
}

void CCScriptNode::setTouchTargeted(bool flag)
{
	if (_touchTargeted == flag)
		return;

	_touchTargeted = flag;

	if (m_bIsRunning && _touchEnabled)
	{
		unregisterWithTouchDispatcher();
		registerWithTouchDispatcher();
	}
}

void CCScriptNode::registerWithTouchDispatcher()
{
	if (_touchTargeted)
		cocos2d::CCTouchDispatcher::sharedDispatcher()->addTargetedDelegate(this, 0, true);
	else
		cocos2d::CCTouchDispatcher::sharedDispatcher()->addStandardDelegate(this, 0);
}

void CCScriptNode::unregisterWithTouchDispatcher()
{
	cocos2d::CCTouchDispatcher::sharedDispatcher()->removeDelegate(this);
}

void CCScriptNode::setModal(bool flag, bool recursive /*= true*/)
{
	cocos2d::CCTouchDispatcher* td = cocos2d::CCTouchDispatcher::sharedDispatcher();

	if (flag)
		td->AddModal(this);
	else
		td->RemoveModal(this);

	if (!recursive) return;

	if (m_pChildren)
	{
		CCObject* child;
		CCARRAY_FOREACH(m_pChildren, child)
		{
			if (dynamic_cast<cocos2d::CCLayer*>(child))
			{
				static_cast<cocos2d::CCLayer*>(child)->setModal(flag, recursive);
			}
			else if (dynamic_cast<CCScriptNode*>(child))
			{
				static_cast<CCScriptNode*>(child)->setModal(flag, recursive);
			}
			else
			{
				cocos2d::CCTouchDelegate* delegate = dynamic_cast<cocos2d::CCTouchDelegate*>(child);
				if (delegate)
				{
					if (flag)
						td->AddModal(delegate);
					else
						td->RemoveModal(delegate);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCScriptNode, ScriptNode);
NB_TYPE_REF(NIT2D_API, cc::ScriptNode, cocos2d::CCNode, IScriptRef::scriptIncRef, IScriptRef::scriptDecRef);

class NB_CCScriptNode : TNitClass<CCScriptNode>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(touchEnabled),
			PROP_ENTRY	(touchTargeted),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			FUNC_ENTRY	(_inherited),
			FUNC_ENTRY_H(setModal,		"(flag: bool, recursive=true)"),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IScriptEventSink>(v);
		addInterface<cocos2d::CCTouchDelegate>(v);
		addInterface<IScriptRef>(v);
	}

	NB_PROP_GET(touchEnabled)			{ return push(v, self(v)->isTouchEnabled()); }
	NB_PROP_GET(touchTargeted)			{ return push(v, self(v)->isTouchTargeted()); }

	NB_PROP_SET(touchEnabled)			{ self(v)->setTouchEnabled(getBool(v, 2)); return 0; }
	NB_PROP_SET(touchTargeted)			{ self(v)->setTouchTargeted(getBool(v, 2)); return 0; }

	NB_CONS()
	{
		CCScriptNode* node = CCScriptNode::nodeWithPeer(new ScriptPeer(v, 1));
		ScriptRuntime::getRuntime(v)->retain(node);
		setSelf(v, node);
		return 0;
	}

	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }

	NB_FUNC(_inherited)					{ return SQ_OK; } // allow inheritance
	
	NB_FUNC(setModal)					{ self(v)->setModal(getBool(v, 2), optBool(v, 3, true)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

class NIT2D_API CCScriptLayer : public TScriptEventSink<CCScriptLayer, cocos2d::CCLayer>, public IScriptRef
{
private:
	typedef cocos2d::CCLayer			inherited;

protected:
	CCScriptLayer() { }
	virtual ~CCScriptLayer();

public:
	static CCScriptLayer*				layerWithPeer(ScriptPeer* peer);
	bool								initWithPeer(ScriptPeer* peer);

public:									// IScriptRef impl
	virtual RefCounted*					_ref()									{ return inherited::_ref(); }
	virtual WeakRef*					_weak()									{ return inherited::_weak(); }
	virtual ScriptPeer*					getScriptPeer()							{ return _peer; }
	virtual String						getDebugString()						{ return description(); }

public:									// IEventSink override
	virtual bool						isEventActive()							{ return _peer && m_bIsRunning; }
	virtual bool						isDisposed()							{ return _peer == NULL; }

public:									// CCLayer's script overridables
	virtual void						onEnter();
	virtual void						onExit();
	virtual void						onEnterTransitionDidFinish();
	virtual void						cleanup();

private:
	Ref<ScriptPeer>						_peer;

	void								onSessionStop(const Event* evt);
};

////////////////////////////////////////////////////////////////////////////////

CCScriptLayer::~CCScriptLayer()
{

}

CCScriptLayer* CCScriptLayer::layerWithPeer(ScriptPeer* peer)
{
	CCScriptLayer* ret = new CCScriptLayer();
	if (ret && ret->initWithPeer(peer))
	{
		ret->autorelease();
		return ret;
	}
	else
	{
		delete ret;
		return NULL;
	}
}

bool CCScriptLayer::initWithPeer(ScriptPeer* peer)
{
	if (!init()) return false;

	if (peer == NULL) return false;

	_peer = peer;

	g_Session->channel()->priority(1)->bind(Events::OnSessionStop, this, &CCScriptLayer::onSessionStop);

	return true;
}

void CCScriptLayer::onSessionStop(const Event* evt)
{
	Ref<CCScriptLayer> safe = this;

	removeFromParentAndCleanup(true);
	_peer = NULL;
}

void CCScriptLayer::onEnter()
{
	inherited::onEnter();

	if (_peer)
		_peer->callMethod("onEnter", 0);
}

void CCScriptLayer::onExit()
{
	if (_peer)
		_peer->callMethod("onExit", 0);

	inherited::onExit();
}

void CCScriptLayer::onEnterTransitionDidFinish()
{
	if (_peer)
		_peer->callMethod("onEnterTransitionDidFinish", 0);

	inherited::onEnterTransitionDidFinish();
}

void CCScriptLayer::cleanup()
{
	if (_peer)
		_peer->callMethod("onCleanup", 0);

	inherited::cleanup();
}

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCScriptLayer, ScriptLayer);
NB_TYPE_REF(NIT2D_API, cc::ScriptLayer, cocos2d::CCLayer, IScriptRef::scriptIncRef, IScriptRef::scriptDecRef);

class NB_CCScriptLayer : TNitClass<CCScriptLayer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY	(_inherited),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IScriptEventSink>(v);
		addInterface<IScriptRef>(v);
	}

	NB_CONS()
	{
		CCScriptLayer* layer = CCScriptLayer::layerWithPeer(new ScriptPeer(v, 1));
		ScriptRuntime::getRuntime(v)->retain(layer);
		setSelf(v, layer);
		return 0;
	}

	NB_FUNC(_inherited)					{ return SQ_OK; } // allow inheritance
};

////////////////////////////////////////////////////////////////////////////////

class NIT2D_API CCScriptScene : public TScriptEventSink<CCScriptScene, cocos2d::CCScene>, public IScriptRef
{
private:
	typedef cocos2d::CCScene			inherited;

protected:
	CCScriptScene() { }
	virtual ~CCScriptScene();

public:
	static CCScriptScene*				sceneWithPeer(ScriptPeer* peer);
	bool								initWithPeer(ScriptPeer* peer);

public:									// IScriptRef impl
	virtual RefCounted*					_ref()									{ return inherited::_ref(); }
	virtual WeakRef*					_weak()									{ return inherited::_weak(); }
	virtual ScriptPeer*					getScriptPeer()							{ return _peer; }
	virtual String						getDebugString()						{ return description(); }

public:									// IEventSink override
	virtual bool						isEventActive()							{ return _peer && m_bIsRunning; }
	virtual bool						isDisposed()							{ return _peer == NULL; }

public:									// CCScene's script overridables
	virtual void						onEnter();
	virtual void						onExit();
	virtual void						onEnterTransitionDidFinish();
	virtual void						cleanup();

private:
	Ref<ScriptPeer>						_peer;

	void								onSessionStop(const Event* evt);
};

////////////////////////////////////////////////////////////////////////////////

CCScriptScene::~CCScriptScene()
{
}

CCScriptScene* CCScriptScene::sceneWithPeer(ScriptPeer* peer)
{
	CCScriptScene* ret = new CCScriptScene();
	if (ret && ret->initWithPeer(peer))
	{
		ret->autorelease();
		return ret;
	}
	else
	{
		delete ret;
		return NULL;
	}
}

bool CCScriptScene::initWithPeer(ScriptPeer* peer)
{
	if (!init()) return false;

	if (peer == NULL) return false;

	_peer = peer;

	g_Session->channel()->priority(1)->bind(Events::OnSessionStop, this, &CCScriptScene::onSessionStop);

	return true;
}

void CCScriptScene::onSessionStop(const Event* evt)
{
	Ref<CCScriptScene> safe = this;

	removeAllChildrenWithCleanup(true);
	_peer = NULL;
}

void CCScriptScene::onEnter()
{
	inherited::onEnter();

	if (_peer)
		_peer->callMethod("onEnter", 0);
}

void CCScriptScene::onExit()
{
	if (_peer)
		_peer->callMethod("onExit", 0);

	inherited::onExit();
}

void CCScriptScene::onEnterTransitionDidFinish()
{
	if (_peer)
		_peer->callMethod("onEnterTransitionDidFinish", 0);

	inherited::onEnterTransitionDidFinish();
}

void CCScriptScene::cleanup()
{
	if (_peer)
		_peer->callMethod("onCleanup", 0);

	inherited::cleanup();
}

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCScriptScene, ScriptScene);
NB_TYPE_REF(NIT2D_API, cc::ScriptScene, cocos2d::CCScene, IScriptRef::scriptIncRef, IScriptRef::scriptDecRef);

class NB_CCScriptScene : TNitClass<CCScriptScene>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY	(_inherited),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IScriptEventSink>(v);
		addInterface<IScriptRef>(v);
	}

	NB_CONS()
	{
		CCScriptScene* layer = CCScriptScene::sceneWithPeer(new ScriptPeer(v, 1));
		ScriptRuntime::getRuntime(v)->retain(layer);
		setSelf(v, layer);
		return 0;
	}

	NB_FUNC(_inherited)					{ return SQ_OK; } // allow inheritance
};

////////////////////////////////////////////////////////////////////////////////

NIT2D_API SQRESULT NitLibNit2d(HSQUIRRELVM v)
{
	NB_CocosService::Register(v);

	NB_CCScriptNode::Register(v);
	NB_CCScriptLayer::Register(v);
	NB_CCScriptScene::Register(v);

	////////////////////////////////////

	sq_pushroottable(v);

	if (g_Service)
	{
		NitBind::newSlot(v, -1, "cocos", g_Cocos);
	}

	sq_poptop(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
