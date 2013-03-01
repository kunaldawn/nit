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

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define NS_IMPORT_NIT(NS, ORIGNAME, NAME) namespace NS { typedef ORIGNAME NAME; }

// The ref-count of cocos follows objective-C convention.
// So it'll be one if allocated by constructor. Because of this we need to dec by one at PostConsHook.

#define NB_TYPE_CC_REF(API, TClass, TBaseClass) \
	_NB_TYPE_PTR_BASE(API, TClass, TBaseClass) \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::constructHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { NitBindImpl::addCachedPtr(v, inst, p); ((TClass*)p)->retain(); return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::postConsHook(HSQUIRRELVM v, SQObjectRef& inst, SQUserPointer p, SQInteger size) { TClass* o = (TClass*)p; if (!o->isAutoReleased()) o->release(); return 0; } \
	template<> API SQInteger			NitBindImpl::TypeInfo<TClass>::releaseHook(HSQUIRRELVM v, SQUserPointer p, SQInteger size) { NitBindImpl::removeCachedPtr(v, p); ((TClass*)p)->release(); return 0; } \
	template<> API SQUserPointer		NitBindImpl::TypeInfo<TClass>::toUserPtr(TClass* ptr) { return (SQUserPointer*)ptr; } \
	template<> API TClass*				NitBindImpl::TypeInfo<TClass>::toClassPtr(SQUserPointer ptr) { return (TClass*)ptr; } 

////////////////////////////////////////////////////////////////////////////////

inline Color FromColor3B(const cocos2d::ccColor3B& c, GLubyte opacity=255)
{
	return Color(c.r/255.f, c.g/255.f, c.b/255.f, opacity/255.f);
}

inline Color FromColor4B(const cocos2d::ccColor4B& c)
{
	return Color(c.r/255.f, c.g/255.f, c.b/255.f, c.a / 255.f);
}

inline Color FromColor4F(const cocos2d::ccColor4F& c)
{
	return Color(c.r, c.g, c.b, c.a);
}

inline cocos2d::ccColor3B ToColor3B(const Color& c)
{
	cocos2d::ccColor3B c3 = { GLubyte(c.r * 255), GLubyte(c.g * 255), GLubyte(c.b * 255) };
	return c3;
}

inline cocos2d::ccColor4B ToColor4B(const Color& c)
{
	cocos2d::ccColor4B c4 = { GLubyte(c.r * 255), GLubyte(c.g * 255), GLubyte(c.b * 255), GLubyte(c.a * 255) };
	return c4;
}

inline cocos2d::ccColor4F ToColor4F(const Color& c)
{
	cocos2d::ccColor4F c4 = { c.r, c.g, c.b, c.a };
	return c4;
}

////////////////////////////////////////////////////////////////////////////////

class NB_CCRGBAProtocol : TNitInterface<cocos2d::CCRGBAProtocol>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(color),
			PROP_ENTRY	(opacity),
			PROP_ENTRY	(opacityModifyRgb),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind<TClass>(v, props, funcs);
	}

	NB_PROP_GET(color)					{ return push(v, FromColor3B(self(v)->getColor(), self(v)->getOpacity()));  }
	NB_PROP_GET(opacity)				{ return push(v, (int)self(v)->getOpacity()); }
	NB_PROP_GET(opacityModifyRgb)		{ return push(v, self(v)->getIsOpacityModifyRGB()); }

	NB_PROP_SET(color)					
	{ 
		Color& c = *get<Color>(v, 2); 
		self(v)->setColor(ToColor3B(c)); 
		if (!self(v)->getIsOpacityModifyRGB())
			self(v)->setOpacity(GLubyte(c.a * 255)); 
		return 0; 
	}

	NB_PROP_SET(opacity)				{ self(v)->setOpacity(GLubyte(getInt(v, 2))); return 0; }
	NB_PROP_SET(opacityModifyRgb)		{ self(v)->setIsOpacityModifyRGB(getBool(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

class NB_CCBlendProtocol : TNitInterface<cocos2d::CCBlendProtocol>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(blendFuncSrc),
			PROP_ENTRY	(blendFuncDst),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind<TClass>(v, props, funcs);

		addStatic<TClass>(v, "DST_BLEND_ZERO",						(int)GL_ZERO);
		addStatic<TClass>(v, "DST_BLEND_ONE",						(int)GL_ONE);
		addStatic<TClass>(v, "DST_BLEND_SRC_COLOR",					(int)GL_SRC_COLOR);
		addStatic<TClass>(v, "DST_BLEND_ONE_MINUS_SRC_COLOR",		(int)GL_ONE_MINUS_SRC_COLOR);
		addStatic<TClass>(v, "DST_BLEND_SRC_ALPHA",					(int)GL_SRC_ALPHA);
		addStatic<TClass>(v, "DST_BLEND_ONE_MINUS_SRC_ALPHA",		(int)GL_ONE_MINUS_SRC_ALPHA);
		addStatic<TClass>(v, "DST_BLEND_DST_ALPHA",					(int)GL_DST_ALPHA);
		addStatic<TClass>(v, "DST_BLEND_ONE_MINUS_DST_ALPHA",		(int)GL_ONE_MINUS_DST_ALPHA);

		addStatic<TClass>(v, "SRC_BLEND_ZERO",						(int)GL_ZERO);
		addStatic<TClass>(v, "SRC_BLEND_ONE",						(int)GL_ONE);
		addStatic<TClass>(v, "SRC_BLEND_DST_COLOR",					(int)GL_DST_COLOR);
		addStatic<TClass>(v, "SRC_BLEND_ONE_MINUS_DST_COLOR",		(int)GL_ONE_MINUS_DST_COLOR);
		addStatic<TClass>(v, "SRC_BLEND_SRC_ALPHA_SATURATE",		(int)GL_SRC_ALPHA_SATURATE);
		addStatic<TClass>(v, "SRC_BLEND_SRC_ALPHA",					(int)GL_SRC_ALPHA);
		addStatic<TClass>(v, "SRC_BLEND_ONE_MINUS_SRC_ALPHA",		(int)GL_ONE_MINUS_SRC_ALPHA);
		addStatic<TClass>(v, "SRC_BLEND_DST_ALPHA",					(int)GL_DST_ALPHA);
		addStatic<TClass>(v, "SRC_BLEND_ONE_MINUS_DST_ALPHA",		(int)GL_ONE_MINUS_DST_ALPHA);

	}

	NB_PROP_GET(blendFuncSrc)			{ return push(v, (int)self(v)->getBlendFunc().src); }
	NB_PROP_GET(blendFuncDst)			{ return push(v, (int)self(v)->getBlendFunc().dst); }

	NB_PROP_SET(blendFuncSrc)			{ cocos2d::ccBlendFunc bf = self(v)->getBlendFunc(); bf.src = (GLenum)getInt(v, 2); self(v)->setBlendFunc(bf); return 0; }
	NB_PROP_SET(blendFuncDst)			{ cocos2d::ccBlendFunc bf = self(v)->getBlendFunc(); bf.dst = (GLenum)getInt(v, 2); self(v)->setBlendFunc(bf); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

class NB_CCTextureProtocol : TNitInterface<cocos2d::CCTextureProtocol>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(texture),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind<TClass>(v, props, funcs);
	}

	NB_PROP_GET(texture)				{ return push(v, self(v)->getTexture()); }
	NB_PROP_SET(texture)				{ self(v)->setTexture(opt<cocos2d::CCTexture2D>(v, 2, NULL)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

class NB_CCLabelProtocol : TNitInterface<cocos2d::CCLabelProtocol>
{
public:
	template <typename TClass>
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(string),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind<TClass>(v, props, funcs);

		addStatic<TClass>(v, "ALIGN_LEFT",		(int)cocos2d::CCTextAlignmentLeft);
		addStatic<TClass>(v, "ALIGN_CENTER",	(int)cocos2d::CCTextAlignmentCenter);
		addStatic<TClass>(v, "ALIGN_RIGHT",		(int)cocos2d::CCTextAlignmentRight);
	}

	NB_PROP_GET(string)					{ return push(v, self(v)->getString()); }
	NB_PROP_SET(string)					{ self(v)->setString(NitBind::getString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
