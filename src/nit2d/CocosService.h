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

#pragma once

#include "nit2d/nit2d.h"

////////////////////////////////////////////////////////////////////////////////

namespace cocos2d
{
	class CCTouchDispatcher;
	class CCKeypadDispatcher;
	class CCIMEDispatcher;
	class CCDirector;
	class CCTextureCache;
	class CCSpriteFrameCache;
	class CCAnimationCache;
	class CCConfiguration;
	class CCFontManager;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define g_Cocos							static_cast<nit::CocosService*>(g_Service->getService(::nit::Service::SVC_COCOS))

////////////////////////////////////////////////////////////////////////////////

class AppDelegate;
class TextureManager;

////////////////////////////////////////////////////////////////////////////////

class NIT2D_API CocosService : public Service
{
public:
	CocosService(Package* package);

public:
	cocos2d::CCDirector*				getDirector()							{ return _director; }
	cocos2d::CCConfiguration*			getConfiguration()						{ return _configuration; }
	RenderView*							getRenderView()							{ return _renderView; }

	cocos2d::CCTouchDispatcher*			getTouchDispatcher()					{ return _touchDispatcher; }
	cocos2d::CCKeypadDispatcher*		getKeypadDispatcher()					{ return _keypadDispatcher; }
	cocos2d::CCIMEDispatcher*			getImeDispatcher()						{ return _imeDispatcher; }

	cocos2d::CCTextureCache*			getTextureCache()						{ return _textureCache; }
	cocos2d::CCSpriteFrameCache*		getSpriteFrameCache()					{ return _spriteFrameCache; }
	cocos2d::CCAnimationCache*			getAnimationCache()						{ return _animationCache; }
	cocos2d::CCActionManager*			getActionManager()						{ return _actionManager; }
	cocos2d::CCFontManager*				getFontManager()						{ return _fontManager; }

protected:								// Module implementation
	virtual void						onRegister();
	virtual void						onUnregister();
	virtual void						onInit();
	virtual void						onFinish();
	virtual void						onContextEnter();

private:
	void								onAppLoop(const Event* evt);
	void								onAppSuspend(const Event* evt);
	void								onAppInactive(const Event* evt);
	void								onAppActive(const Event* evt);
	void								onAppResume(const Event* evt);
	void								onAppLowMemory(const Event* evt);

	void								createSingletons();
	void								switchSingletons();
	void								destroySingletons();

	cocos2d::CCDirector*				_director;
	cocos2d::CCConfiguration*			_configuration;
	cocos2d::CCTouchDispatcher*			_touchDispatcher;
	cocos2d::CCKeypadDispatcher*		_keypadDispatcher;
	cocos2d::CCIMEDispatcher*			_imeDispatcher;
	cocos2d::CCTextureCache*			_textureCache;
	cocos2d::CCSpriteFrameCache*		_spriteFrameCache;
	cocos2d::CCAnimationCache*			_animationCache;
	cocos2d::CCActionManager*			_actionManager;
	cocos2d::CCFontManager*				_fontManager;

	Ref<RenderView>						_renderView;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
