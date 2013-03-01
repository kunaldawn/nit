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

#include "nitwx_pch.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

extern SQRESULT NitLibWxBase(HSQUIRRELVM v);
extern SQRESULT NitLibWxControl(HSQUIRRELVM v);
extern SQRESULT NitLibWxDialog(HSQUIRRELVM v);
extern SQRESULT NitLibWxEvent(HSQUIRRELVM v);
extern SQRESULT NitLibWx(HSQUIRRELVM v);
extern SQRESULT NitLibWxPropGrid(HSQUIRRELVM v);
extern SQRESULT NitLibWxTextCtrl(HSQUIRRELVM v);
extern SQRESULT NitLibWxGraphics(HSQUIRRELVM v);
extern SQRESULT NitLibWxDataView(HSQUIRRELVM v);
extern SQRESULT NitLibWxGrid(HSQUIRRELVM v);
extern SQRESULT NitLibWxDragDrop(HSQUIRRELVM v);
extern SQRESULT NitLibWxAui(HSQUIRRELVM v);
extern SQRESULT NitLibWxNit(HSQUIRRELVM v);

#if defined(NIT_WIN32) // TODO: FIX WxSF
extern SQRESULT NitLibWxSF(HSQUIRRELVM v);
#endif

class NitLibWxModule
{
public:
	NitLibWxModule()
	{
		o.push_back(new NIT_LIB_ENTRY(NitLibWxBase,		"NitLibCore"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWxControl,	"NitLibWxBase NitLibWxEvent"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWxDialog,	"NitLibWxBase NitLibWxEvent"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWxEvent,	"NitLibWxBase"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWx,			"NitLibWxBase"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWxPropGrid,	"NitLibWxBase NitLibWxEvent"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWxTextCtrl,	"NitLibWxBase NitLibWxEvent"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWxGraphics,	"NitLibWxBase"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWxDataView,	"NitLibWxBase NitLibWxEvent"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWxGrid,		"NitLibWxBase NitLibWxEvent"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWxDragDrop,	"NitLibWxBase NitLibWxEvent"));
		o.push_back(new NIT_LIB_ENTRY(NitLibWxAui,		"NitLibWxBase NitLibWxControl NitLibWx NitLibWxEvent"));
#if defined(NIT_WIN32)
		o.push_back(new NIT_LIB_ENTRY(NitLibWxSF,		"NitLibWxBase NitLibWxGraphics NitLibWx NitLibWxEvent"));
#endif
		o.push_back(new NIT_LIB_ENTRY(NitLibWxNit,		"NitLibWxBase NitLibWxControl NitLibWxDialog "
														"NitLibWxEvent NitLibWx NitLibWxPropGrid "
														"NitLibWxTextCtrl NitLibWxGraphics "
														"NitLibWxDataView NitLibWxGrid NitLibWxDragDrop NitLibWxAui"));
	}

	~NitLibWxModule()
	{
		std::for_each(o.begin(), o.end(), DeleteFunc());
	}

private:
	std::vector<NitBindLibRegistry*> o;
};

NitLibWxModule s_NitLibWxModule;

void* RegisterStaticNitWxModule()
{
	return &s_NitLibWxModule;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
