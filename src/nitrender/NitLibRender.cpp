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

#include "nitrender/RenderSpec.h"
#include "nitrender/RenderService.h"
#include "nitrender/RenderContext.h"
#include "nitrender/RenderView.h"
#include "nitrender/GLESTexture.h"

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NITRENDER_API, nit::RenderSpec, NULL);

class NB_RenderSpec : TNitClass<RenderSpec>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(numWorldMatrices),
			PROP_ENTRY_R(numTextureUnits),
			PROP_ENTRY_R(numVertexBlendMatrices),
			PROP_ENTRY_R(numVertexTextureUnits),
			PROP_ENTRY_R(maxTextureSize),
			PROP_ENTRY_R(maxViewportWidth),
			PROP_ENTRY_R(maxViewportHeight),
			PROP_ENTRY_R(numDepthBufferBits),
			PROP_ENTRY_R(numStencilBufferBits),
			PROP_ENTRY_R(numVertexShaderFloatConsts),
			PROP_ENTRY_R(numVertexShaderIntConsts),
			PROP_ENTRY_R(numVertexShaderBoolConsts),
			PROP_ENTRY_R(numGeometryShaderFloatConsts),
			PROP_ENTRY_R(numGeometryShaderIntConsts),
			PROP_ENTRY_R(numGeometryShaderBoolConsts),
			PROP_ENTRY_R(numPixelShaderFloatConsts),
			PROP_ENTRY_R(numPixelShaderIntConsts),
			PROP_ENTRY_R(numPixelShaderBoolConsts),
			PROP_ENTRY_R(numMultiRenderTargets),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(isCapable,		"(spec: int): bool"),
			FUNC_ENTRY_H(report,		"(ch: LogChannel = null)"),
			NULL
		};

		bind(v, props, funcs);

		vector<std::pair<String, uint32> >::type specs;
		RenderSpec::allSpecs(specs);

		for (uint i=0; i<specs.size(); ++i)
		{
			const String& name = specs[i].first;
			size_t ppos = name.find('.');
			if (ppos != name.npos)
			{
				addStaticTable(v, name.substr(0, ppos).c_str(), false, false);
				newSlot(v, -1, name.substr(ppos+1), specs[i].second);
				sq_poptop(v);
			}
			else
			{
				addStatic(v, name.c_str(), specs[i].second);
			}
		}
	}

	NB_PROP_GET(numWorldMatrices)				{ return push(v, self(v)->getNumWorldMatrices()); }
	NB_PROP_GET(numTextureUnits)				{ return push(v, self(v)->getNumTextureUnits()); }
	NB_PROP_GET(numVertexBlendMatrices)			{ return push(v, self(v)->getNumVertexBlendMatrices()); }
	NB_PROP_GET(numVertexTextureUnits)			{ return push(v, self(v)->getNumVertexTextureUnits()); }
	NB_PROP_GET(maxTextureSize)					{ return push(v, self(v)->getMaxTextureSize()); }
	NB_PROP_GET(maxViewportWidth)				{ return push(v, self(v)->getMaxViewportWidth()); }
	NB_PROP_GET(maxViewportHeight)				{ return push(v, self(v)->getMaxViewportHeight()); }
	NB_PROP_GET(numDepthBufferBits)				{ return push(v, self(v)->getNumDepthBufferBits()); }
	NB_PROP_GET(numStencilBufferBits)			{ return push(v, self(v)->getNumStencilBufferBits()); }
	NB_PROP_GET(numVertexShaderFloatConsts)		{ return push(v, self(v)->getNumVertexShaderFloatConsts()); }
	NB_PROP_GET(numVertexShaderIntConsts)		{ return push(v, self(v)->getNumVertexShaderIntConsts()); }
	NB_PROP_GET(numVertexShaderBoolConsts)		{ return push(v, self(v)->getNumVertexShaderBoolConsts()); }
	NB_PROP_GET(numGeometryShaderFloatConsts)	{ return push(v, self(v)->getNumGeometryShaderFloatConsts()); }
	NB_PROP_GET(numGeometryShaderIntConsts)		{ return push(v, self(v)->getNumGeometryShaderIntConsts()); }
	NB_PROP_GET(numGeometryShaderBoolConsts)	{ return push(v, self(v)->getNumGeometryShaderBoolConsts()); }
	NB_PROP_GET(numPixelShaderFloatConsts)		{ return push(v, self(v)->getNumPixelShaderFloatConsts()); }
	NB_PROP_GET(numPixelShaderIntConsts)		{ return push(v, self(v)->getNumPixelShaderIntConsts()); }
	NB_PROP_GET(numPixelShaderBoolConsts)		{ return push(v, self(v)->getNumPixelShaderBoolConsts()); }
	NB_PROP_GET(numMultiRenderTargets)			{ return push(v, self(v)->getNumMultiRenderTargets()); }

	NB_FUNC(isCapable)							{ return push(v, self(v)->isCapable(getInt(v, 2))); }
	NB_FUNC(report)								{ self(v)->report(opt<LogChannel>(v, 2, NULL)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NITRENDER_API, nit::RenderContext, NULL);

class NB_RenderContext : TNitClass<RenderContext>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(spec),
			PROP_ENTRY_R(view),
			PROP_ENTRY_R(device),
			PROP_ENTRY_R(service),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(spec)					{ return push(v, self(v)->getSpec()); }
	NB_PROP_GET(view)					{ return push(v, self(v)->getView()); }
	NB_PROP_GET(device)					{ return push(v, self(v)->getDevice()); }
	NB_PROP_GET(service)				{ return push(v, self(v)->getService()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NITRENDER_API, nit::RenderDevice, NULL);

class NB_RenderDevice : TNitClass<RenderDevice>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(ready),
			PROP_ENTRY_R(spec),
			PROP_ENTRY_R(handleCount),
			PROP_ENTRY_R(handleFootprint),
			PROP_ENTRY_R(wiredHandleCount),
			PROP_ENTRY_R(activeHandleCount),
			PROP_ENTRY_R(inactiveHandleCount),
			PROP_ENTRY_R(inactiveFootprint),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(reset,			"(): bool"),
			FUNC_ENTRY_H(invalidate,	"()"),
 			FUNC_ENTRY_H(clearCaches,	"()"),
			FUNC_ENTRY_H(allCaches,		"()"),
			FUNC_ENTRY_H(getCache,		"(name: string): CacheManager"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(ready)					{ return push(v, self(v)->isValid()); }
	NB_PROP_GET(spec)					{ return push(v, self(v)->getSpec()); }
	NB_PROP_GET(handleCount)			{ return push(v, self(v)->getHandleCount()); }
	NB_PROP_GET(handleFootprint)		{ return push(v, self(v)->getHandleFootprint()); }
	NB_PROP_GET(wiredHandleCount)		{ return push(v, self(v)->getWiredHandleCount()); }
	NB_PROP_GET(activeHandleCount)		{ return push(v, self(v)->getActiveHandleCount()); }
	NB_PROP_GET(inactiveHandleCount)	{ return push(v, self(v)->getInactiveHandleCount()); }
	NB_PROP_GET(inactiveFootprint)		{ return push(v, self(v)->getInactiveFootprint()); }

	NB_FUNC(reset)						{ return push(v, self(v)->reset()); }
	NB_FUNC(invalidate)					{ self(v)->invalidate(); return 0; }
 	NB_FUNC(clearCaches)				{ self(v)->clearCaches(); return 0; }
	NB_FUNC(getCache)					{ return push(v, self(v)->getCache(getString(v, 2))); }

	NB_FUNC(allCaches)
	{
		sq_newtable(v);
		type::CacheLookup& lookup = self(v)->getCacheLookup();
		for (type::CacheLookup::iterator itr = lookup.begin(), end = lookup.end(); itr != end; ++itr)
		{
			newSlot(v, -1, itr->first, itr->second);
		}
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NITRENDER_API, nit::RenderView, NULL);

class NB_RenderView : TNitClass<RenderView>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(orientation),
			PROP_ENTRY_R(portrait),
			PROP_ENTRY_R(landscape),
			PROP_ENTRY_R(left),
			PROP_ENTRY_R(top),
			PROP_ENTRY_R(width),
			PROP_ENTRY_R(height),
			PROP_ENTRY	(scale),
			PROP_ENTRY_R(scaledWidth),
			PROP_ENTRY_R(scaledHeight),
			PROP_ENTRY_R(renderDevice),
			PROP_ENTRY_R(inputDevice),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "ORIENT");
		newSlot(v, -1, "PORTRAIT",			(int)type::ORIENT_PORTRAIT);
		newSlot(v, -1, "PORTRAIT_FLIP",		(int)type::ORIENT_PORTRAIT_FLIP);
		newSlot(v, -1, "LANDSCAPE",			(int)type::ORIENT_LANDSCAPE);
		newSlot(v, -1, "LANDSCAPE_FLIP",	(int)type::ORIENT_LANDSCAPE_FLIP);
		sq_poptop(v);
	}

	NB_PROP_GET(orientation)			{ return push(v, (int)self(v)->getOrientation()); }
	NB_PROP_GET(portrait)				{ return push(v, self(v)->isPortrait()); }
	NB_PROP_GET(landscape)				{ return push(v, self(v)->isLandscape()); }
	NB_PROP_GET(left)					{ return push(v, self(v)->getLeft()); }
	NB_PROP_GET(top)					{ return push(v, self(v)->getTop()); }
	NB_PROP_GET(width)					{ return push(v, self(v)->getWidth()); }
	NB_PROP_GET(height)					{ return push(v, self(v)->getHeight()); }
	NB_PROP_GET(scale)					{ return push(v, self(v)->getScale()); }
	NB_PROP_GET(scaledWidth)			{ return push(v, self(v)->getScaledWidth()); }
	NB_PROP_GET(scaledHeight)			{ return push(v, self(v)->getScaledHeight()); }
	NB_PROP_GET(renderDevice)			{ return push(v, self(v)->getRenderDevice()); }
	NB_PROP_GET(inputDevice)			{ return push(v, self(v)->getInputDevice()); }

	NB_PROP_SET(orientation)			{ self(v)->setOrientation((type::Orientation)getInt(v, 2)); return 0; }
	NB_PROP_SET(scale)					{ self(v)->setScale(getFloat(v, 2)); return 0; }

	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITRENDER_API, nit::RenderService, Service, incRefCount, decRefCount);

class NB_RenderService : TNitClass<RenderService>
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
			FUNC_ENTRY_H(beginContext,	"(view: RenderView): RenderContext"),
			FUNC_ENTRY_H(endContext,	"(ctx: RenderContext)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(beginContext)				{ return push(v, self(v)->beginContext(get<RenderView>(v, 2))); }
	NB_FUNC(endContext)					{ self(v)->endContext(get<RenderContext>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

// TODO: Do not expose GLES*

NB_TYPE_REF(NITRENDER_API, nit::GLESTexture, Texture, incRefCount, decRefCount);

class NB_GLESTexture : TNitClass<GLESTexture>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(activatedProxy),
			PROP_ENTRY_R(activated),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(sourceImage: Image, manager: TextureManager=NULL)\n"
										"(source: StreamSource, manager: TextureManager=NULL)"),
			FUNC_ENTRY_H(activate,		"(ctx: RenderContext): AR"),
			FUNC_ENTRY_H(deactivate,	"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "AR");
		newSlot(v, -1, "OK",			(int)GLESTexture::AR_OK);
		newSlot(v, -1, "NOT_LOADED",	(int)GLESTexture::AR_NOT_LOADED);
		newSlot(v, -1, "HAS_ERROR",		(int)GLESTexture::AR_HAS_ERROR);
		newSlot(v, -1, "LOADING",		(int)GLESTexture::AR_LOADING);
		newSlot(v, -1, "DISPOSED",		(int)GLESTexture::AR_DISPOSED);
	};

	NB_PROP_GET(activatedProxy)			{ return push(v, self(v)->getActivatedProxy()); }
	NB_PROP_GET(activated)				{ return push(v, self(v)->isActivated()); }

	NB_CONS()
	{ 
		if (is<Image>(v, 2))
			setSelf(v, new GLESTexture(get<Image>(v, 2), opt<GLESTextureManager>(v, 3, NULL))); 
		else
			setSelf(v, new GLESTexture(get<StreamSource>(v, 2), opt<GLESTextureManager>(v, 3, NULL)));
		return 0; 
	}

	NB_FUNC(activate)					{ return push(v, (int)self(v)->activate(get<RenderContext>(v, 2))); return 0; }
	NB_FUNC(deactivate)					{ self(v)->deactivate(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NITRENDER_API, nit::GLESTextureManager, TextureManager, incRefCount, decRefCount);

class NB_GLESTextureManager : TNitClass<GLESTextureManager>
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
			NULL
		};

		bind(v, props, funcs);
	};
};

////////////////////////////////////////////////////////////////////////////////

NITRENDER_API SQRESULT NitLibRender(HSQUIRRELVM v)
{
	NB_RenderSpec::Register(v);
	NB_RenderContext::Register(v);
	NB_RenderDevice::Register(v);
	NB_RenderView::Register(v);
	NB_RenderService::Register(v);

	// TODO: Do not expose GLES*
	NB_GLESTexture::Register(v);
	NB_GLESTextureManager::Register(v);

	////////////////////////////////////

	sq_pushroottable(v);

	if (g_Service)
	{
		NitBind::newSlot(v, -1, "render", g_Render);
	}

	sq_poptop(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
