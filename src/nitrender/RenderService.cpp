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

#include "NITRENDER_pch.h"

#include "nitrender/RenderService.h"
#include "nitrender/RenderContext.h"
#include "nitrender/RenderView.h"
#include "nitrender/RenderHandle.h"
#include "nitrender/RenderDevice.h"

#include "nit/runtime/NitRuntime.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

RenderService::RenderService(Package* package)
: Service("RenderService", package, SVC_RENDER)
{
	_currentContext = NULL;

	_textureNPOTSupported = true;
	_renderTargetNPOTSupported = true;
}

void RenderService::onRegister()
{

}

void RenderService::onUnregister()
{

}

void RenderService::onInit()
{

}

void RenderService::onFinish()
{

}

void RenderService::onContextEnter()
{

}

RenderContext* RenderService::beginContext(RenderView* view)
{
	RenderContext* ctx = NULL;

	if (_currentContext == NULL)
		ctx = view->_newRenderContext();
	else if (_currentContext->getView() == view)
		ctx = _currentContext;
	else
		NIT_THROW_FMT(EX_NOT_SUPPORTED, "only one view context supported at a time (yet)");

	_currentContext = ctx;
	ctx->_service = this;

	if (ctx->_beginCount++ == 0)
	{
		ctx->getDevice()->beginContext(ctx);
	}

	return ctx;
}

void RenderService::endContext(RenderContext* ctx)
{
	--ctx->_beginCount;
	if (ctx->_beginCount == 0)
	{
		ctx->getDevice()->endContext(ctx);
		if (ctx == _currentContext)
			_currentContext = NULL;
		delete ctx;
	}
}

// Following methods are provided for a situation some user code wants to access RenderSpec
// before activation. (e.g. layout purpose)

// If there are more than one device and not all of them supports NPOT,
// treat as NPOT not supported.

// TODO: improve

void RenderService::textureNeedPOT()
{
	_textureNPOTSupported = false;
}

void RenderService::renderTargetNeedPOT()
{
	_renderTargetNPOTSupported = false;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
