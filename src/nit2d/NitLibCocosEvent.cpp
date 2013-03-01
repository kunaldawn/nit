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

#include "nit2d/NitLibCocos.h"

using namespace cocos2d;

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTouchEvent, TouchEvent);
NB_TYPE_REF(NIT2D_API, cc::TouchEvent, Event, incRefCount, decRefCount);

class NB_CCTouchEvent : TNitClass<CCTouchEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(sender),
			PROP_ENTRY_R(touch0),
			PROP_ENTRY_R(touch1),
			PROP_ENTRY_R(touch2),
			PROP_ENTRY_R(touches),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(this: cc.Object, touch: cc.Touch)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(sender)					{ return push(v, self(v)->Sender); }

	NB_PROP_GET(touch0)					{ return push(v, *self(v)->Touches->begin()); }
	NB_PROP_GET(touch1)				
	{
		if (self(v)->Touches->count() < 2)
			return 0;
		
		CCSetIterator itr = self(v)->Touches->begin();
		return push(v, *(++itr));
	}

	NB_PROP_GET(touch2)				
	{ 
		if (self(v)->Touches->count() < 3)
			return 0;
		
		CCSetIterator itr = self(v)->Touches->begin();
		for (int i = 0; i < 2; i++)
			++itr;

		return push(v, *itr);
	}

	NB_PROP_GET(touches)
	{ 
		sq_newarray(v, 0);
		for (CCSetIterator itr = self(v)->Touches->begin(), end = self(v)->Touches->end(); itr != end; ++itr)
		{
			arrayAppend(v, -1, *itr);
		}
		return 1;
	}

	NB_CONS()							
	{
		CCSet* pTouches = new CCSet;
		
		pTouches->addObject(get<CCTouch>(v,3));
		setSelf(v, new CCTouchEvent(get<CCObject>(v, 2), pTouches)); 
	
		pTouches->autorelease();
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTargetedTouchEvent, TargetedTouchEvent);
NB_TYPE_REF(NIT2D_API, cc::TargetedTouchEvent, Event, incRefCount, decRefCount);

class NB_CCTargetedTouchEvent : TNitClass<CCTargetedTouchEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(sender),
			PROP_ENTRY_R(touch),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(sender)					{ return push(v, self(v)->Sender); }
	NB_PROP_GET(touch)					{ return push(v, self(v)->Touch); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTouchModalEvent, TouchModalEvent);
NB_TYPE_REF(NIT2D_API, cc::TouchModalEvent, Event, incRefCount, decRefCount);

class NB_CCTouchModalEvent : TNitClass<CCTouchModalEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(layer),
			PROP_ENTRY_R(modal),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(layer)					{ return push(v, self(v)->Layer); }
	NB_PROP_GET(modal)					{ return push(v, self(v)->Modal); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCKeypadEvent, KeypadEvent);
NB_TYPE_REF(NIT2D_API, cc::KeypadEvent, Event, incRefCount, decRefCount);

class NB_CCKeypadEvent : TNitClass<CCKeypadEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(sender),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(sender)					{ return push(v, self(v)->Sender); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCMenuItemEvent, MenuItemEvent);
NB_TYPE_REF(NIT2D_API, cc::MenuItemEvent, Event, incRefCount, decRefCount);

class NB_CCMenuItemEvent : TNitClass<CCMenuItemEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(item),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(item: cc.MenuItem)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(item)					{ return push(v, self(v)->Item); }

	NB_CONS()							{ setSelf(v, new CCMenuItemEvent(get<CCMenuItem>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NIT2D_API SQRESULT NitLibCocosEvent(HSQUIRRELVM v)
{
	NB_CCTouchEvent::Register(v);
	NB_CCTargetedTouchEvent::Register(v);
	NB_CCTouchModalEvent::Register(v);
	NB_CCKeypadEvent::Register(v);
	NB_CCMenuItemEvent::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
