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

#include "nitrender/RenderService.h"

#include "nit/app/AppBase.h"
#include "nit/data/Settings.h"

#include "CCPlatformMacros.h"
#include "CCKeypadDispatcher.h"
#include "CCIMEDispatcher.h"
#include "CCConfiguration.h"
#include "CCFont.h"
#include "CCAutoreleasePool.h"

#if defined(NIT_WIN32)
#	include "nitrender/GLESRenderView_win32.h"
#endif

#if defined(NIT_IOS)
#	include "nitrender/GLESRenderView_ios.h"
#endif

#if defined(NIT_ANDROID)
#	include "nitrender/GLESRenderView_android.h"
#endif

using namespace cocos2d;

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

CocosService::CocosService(Package* package) : Service("CocosService", package, SVC_COCOS)
{
	_director			= NULL;
	_configuration		= NULL;
	_touchDispatcher	= NULL;
	_keypadDispatcher	= NULL;
	_imeDispatcher		= NULL;
	_textureCache		= NULL;
	_spriteFrameCache	= NULL;
	_animationCache	= NULL;
	_actionManager		= NULL;
	_fontManager		= NULL;

	_renderView		= NULL;
}

void CocosService::onRegister()
{
	if (!require(g_Render)) NIT_THROW(EX_NOT_FOUND);
}

void CocosService::onUnregister()
{
}

void CocosService::onInit()
{
	CCLOG("CCLOG linked to nit.LogManager");

	const String& deviceForm = g_App->getConfig("device_form", "unknown");

	int viewWidth		= DataValue(g_App->getConfig(String("cocos/view_width@") + deviceForm, g_App->getConfig("cocos/view_width", "320"))).toInt();
	int viewHeight		= DataValue(g_App->getConfig(String("cocos/view_height@") + deviceForm, g_App->getConfig("cocos/view_height", "480"))).toInt();

	_director = new CCDirector();
	CCDirector::_setInstance(_director);
	_director->init();

#if defined(NIT_WIN32)

	HWND hwnd = (HWND) DataValue(g_App->getConfig("cocos/win32_hwnd", "0")).toInt();

	Ref<GLESRenderView_win32> view = new GLESRenderView_win32(g_App);

	if (hwnd)
	{
		if (!view->create(hwnd))
			NIT_THROW_FMT(EX_SYSTEM, "Cannot attach render view to hwnd %08X for cocos", hwnd);
		view->resize(viewWidth, viewHeight);
	}
	else
	{
		if (!view->create(g_App->getConfigs()->getTitle(), viewWidth, viewHeight))
			NIT_THROW_FMT(EX_SYSTEM, "Cannot create render view for cocos");
	}

	_renderView = view;
	view->centerWindow();
	view->show();
#endif

#if defined(NIT_IOS)
	Ref<GLESRenderView_ios> view = new GLESRenderView_ios();
	_renderView = view;
#endif

#if defined(NIT_ANDROID)
	Ref<GLESRenderView_android> view = new GLESRenderView_android(g_App);

	if (!NitRuntime::getSingleton()->waitForWindowInit())
	{
		NIT_THROW_FMT(EX_SYSTEM, "Cannot obtain render view for cocos");
	}

	_renderView = view;
#endif

	float contentScale = std::min(_renderView->getScaledWidth(), _renderView->getScaledHeight()) / std::min(viewWidth, viewHeight);

	contentScale = DataValue(g_App->getConfig("cocos/content_scale", DataValue(contentScale).toString()));
	_director->setContentScaleFactor(contentScale);

	createSingletons();
	switchSingletons();

	_touchDispatcher->SetInputUser(svc_Input->getDefaultUser());

	// enable High Resource Mode(2x, such as iphone4) and maintains low resource on other devices.
	//     pDirector->enableRetinaDisplay(true);

	_director->SetRenderView(_renderView);
	_director->setProjection(kCCDirectorProjectionDefault);

	_director->setDeviceOrientation(kCCDeviceOrientationLandscapeLeft);

	// turn off display FPS by default
	_director->setDisplayFPS(false);

	// Director is now ready, bind events //////////////////////////////////////////////////////

	g_App->channel()->bind(Events::OnAppLoop, this, &CocosService::onAppLoop);

	g_App->channel()->bind(Events::OnAppSuspend, this, &CocosService::onAppSuspend);
	g_App->channel()->bind(Events::OnAppInactive, this, &CocosService::onAppInactive);
	g_App->channel()->bind(Events::OnAppActive, this, &CocosService::onAppActive);
	g_App->channel()->bind(Events::OnAppResume, this, &CocosService::onAppResume);

	g_App->channel()->bind(Events::OnAppLowMemory, this, &CocosService::onAppLowMemory);
}

void CocosService::onFinish()
{
	// Unless we do unbind, decRef() call will provoke head-ache at EventHandler allocated with this DLL.
	// Weak has also problem with remaining WeakRef objects when DLL unloaded.
	// TODO: Find a nit way
	g_App->channel()->unbind(0, this);

	destroySingletons();
}

void CocosService::onContextEnter()
{
	switchSingletons();
}

void CocosService::onAppLoop(const Event* evt)
{
 	if (_director)
	{
		_director->updateScene();

		RenderView* view = _director->GetRenderView();
		RenderDevice* device = view ? view->getRenderDevice() : NULL;

		// TODO: Refactor this begin/end frame to RenderService.
		// NOTE: When render while suspended on android devices, handles will occur problem hence they're restored.
		if (device && device->isValid() && !g_App->isSuspended())
		{
			device->beginFrame();

			Weak<RenderContext> ctx = g_Render->beginContext(view);
			_director->drawScene(ctx);
			g_Render->endContext(ctx);

			device->endFrame();

			view->getRenderDevice()->swapBuffers();
		}

		// cleanup current autorelease pool
		CCPoolManager::getInstance()->pop();
	}
}

void CocosService::onAppSuspend(const Event* evt)
{
	_director->pause();
    _director->stopAnimation();
}

void CocosService::onAppInactive(const Event* evt)
{
	// TODO: Lower CPU usage
}

void CocosService::onAppActive(const Event* evt)
{
	// TODO: Restore CPU usage
}

void CocosService::onAppResume(const Event* evt)
{
	_director->resume();
    _director->startAnimation();
}

void CocosService::onAppLowMemory(const Event* evt)
{
	_director->purgeCachedData();

	RenderDevice* device = _renderView ? _renderView->getRenderDevice() : NULL;

	if (device)
	{
		device->clearCaches();
	}

	// TODO: More memory optimization
}

void CocosService::createSingletons()
{
	_configuration = new CCConfiguration();
	CCConfiguration::_setInstance(_configuration);
	_configuration->init();

	_touchDispatcher = new CCTouchDispatcher();
	CCTouchDispatcher::_setInstance(_touchDispatcher);
	_touchDispatcher->init();

	_keypadDispatcher = new CCKeypadDispatcher();
	CCKeypadDispatcher::_setInstance(_keypadDispatcher);

	_imeDispatcher = new CCIMEDispatcher();
	CCIMEDispatcher::_setInstance(_imeDispatcher);

	_textureCache = new CCTextureCache();
	CCTextureCache::_setInstance(_textureCache);

	_spriteFrameCache = new CCSpriteFrameCache();
	CCSpriteFrameCache::_setInstance(_spriteFrameCache);
	_spriteFrameCache->init();

	_animationCache = new CCAnimationCache();
	CCAnimationCache::_setInstance(_animationCache);
	_animationCache->init();

	_actionManager = new CCActionManager();
	CCActionManager::_setInstance(_actionManager);
	_actionManager->init();

	int maxFontFaces = DataValue(g_App->getConfig("cocos/font_max_faces", "4"));
	int maxFontSizes = DataValue(g_App->getConfig("cocos/font_max_sizes", "4"));
	int maxFontBytes = DataValue(g_App->getConfig("cocos/font_max_bytes", "262144"));

	_fontManager = new CCFontManager(maxFontFaces, maxFontSizes, maxFontBytes);
	CCFontManager::setSingleton(_fontManager);
}

void CocosService::switchSingletons()
{
	CCDirector::_setInstance(_director);
	CCConfiguration::_setInstance(_configuration);
	CCTouchDispatcher::_setInstance(_touchDispatcher);
	CCKeypadDispatcher::_setInstance(_keypadDispatcher);
	CCIMEDispatcher::_setInstance(_imeDispatcher);
	CCTextureCache::_setInstance(_textureCache);
	CCSpriteFrameCache::_setInstance(_spriteFrameCache);
	CCAnimationCache::_setInstance(_animationCache);
	CCActionManager::_setInstance(_actionManager);
	CCFontManager::setSingleton(_fontManager);
}

void CocosService::destroySingletons()
{
	_director->_purgeDirector();

	safeDelete(_director);
	safeDelete(_configuration);
	safeDelete(_touchDispatcher);
	safeDelete(_keypadDispatcher);
	safeDelete(_imeDispatcher);
	safeDelete(_textureCache);
	safeDelete(_spriteFrameCache);
	safeDelete(_animationCache);
	safeDelete(_actionManager);
	safeDelete(_fontManager);

	_renderView = NULL;

	switchSingletons();
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
