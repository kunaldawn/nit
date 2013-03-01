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

#include "nit/app/Plugin.h"

#include "nit2d/CocosService.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

extern SQRESULT NitLibCocos(HSQUIRRELVM v);
extern SQRESULT NitLibCocosAction(HSQUIRRELVM v);
extern SQRESULT NitLibCocosTransition(HSQUIRRELVM v);
extern SQRESULT NitLibCocosParticle(HSQUIRRELVM v);
extern SQRESULT NitLibCocosEvent(HSQUIRRELVM v);
extern SQRESULT NitLibNit2d(HSQUIRRELVM v);

class NIT2D_API Nit2dPlugin : public Plugin
{
public:
	virtual void onInstall()
	{
		// HACK: Make resident to prevent DLL unloading on session.restart()
		getPackage()->setStayResident(true);

		Register(new CocosService(getPackage()));

		Register(new NIT_LIB_ENTRY(NitLibCocos,				"NitLibApp NitLibRender"));
		Register(new NIT_LIB_ENTRY(NitLibCocosAction,		"NitLibCocos"));
		Register(new NIT_LIB_ENTRY(NitLibCocosTransition,	"NitLibCocos"));
		Register(new NIT_LIB_ENTRY(NitLibCocosParticle,		"NitLibCocos"));
		Register(new NIT_LIB_ENTRY(NitLibCocosEvent,		"NitLibCocos"));
		Register(new NIT_LIB_ENTRY(NitLibNit2d,				"NitLibCocos"));
	}

	virtual void onUninstall()
	{

	}
};

////////////////////////////////////////////////////////////////////////////////

NIT_PLUGIN_DEFINE(NIT2D_API, Nit2dPlugin);

NS_NIT_END;
