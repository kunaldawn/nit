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

#include "nitwx/NitLibWx.h"

#include "nitwx/NitLibWxNitApp.h"

#include "wx/xrc/xmlres.h"
#include "wx/wfstream.h"

#include "nit/script/NitBind.h"
#include "nit/script/NitBindMacro.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

namespace wx { typedef wxClassInfo ClassInfo; }

NB_TYPE_RAW_PTR(NITWX_API, wx::ClassInfo, NULL);

class NB_WxClassInfo : TNitClass<wxClassInfo>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(baseClass1),
			PROP_ENTRY_R(baseClass2),
			PROP_ENTRY_R(className),
			PROP_ENTRY_R(dynamic),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(createObject,	"(): wx.Object"),
			FUNC_ENTRY_H(isKindOf,		"(info: wx.ClassInfo): bool"),
			FUNC_ENTRY_H(findClass,		"[class] (classname: string): wx.ClassInfo"),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(baseClass1)				{ return push(v, const_cast<wxClassInfo*>(self(v)->GetBaseClass1())); }
	NB_PROP_GET(baseClass2)				{ return push(v, const_cast<wxClassInfo*>(self(v)->GetBaseClass2())); }
	NB_PROP_GET(className)				{ return push(v, wxString(self(v)->GetClassName())); }
	NB_PROP_GET(dynamic)				{ return push(v, self(v)->IsDynamic()); }

	NB_FUNC(createObject)				{ return push(v, self(v)->CreateObject()); }
	NB_FUNC(isKindOf)					{ return push(v, self(v)->IsKindOf(GetWxClassInfo(v, 2))); }
	NB_FUNC(findClass)					{ return push(v, type::FindClass(getWxString(v, 2))); }

	NB_FUNC(_tostring)
	{
		wxClassInfo* o = self(v);
		wxString name = wxString(o->GetClassName());
		const wxScopedCharBuffer utf8 = name.utf8_str(); 
		return pushFmt(v, "(wx.ClassInfo '%s' -> %08x)", utf8.data(), self);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Point, NULL);

class NB_WxPoint : TNitClass<wxPoint>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(x),
			PROP_ENTRY	(y),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(x=0, y=0)"),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(x)						{ return push(v, self(v)->x); }
	NB_PROP_GET(y)						{ return push(v, self(v)->y); }

	NB_PROP_SET(x)						{ self(v)->x = getInt(v, 2); return 0; }
	NB_PROP_SET(y)						{ self(v)->y = getInt(v, 2); return 0; }

	NB_CONS()
	{
		new (self(v)) wxPoint(optInt(v, 2, 0), optInt(v, 3, 0));
		return 0;
	}

	NB_FUNC(_tostring)
	{
		wxPoint* o = self(v);
		pushFmt(v, "Point(%d, %d)", o->x, o->y);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Size, NULL);

class NB_WxSize : TNitClass<wxSize>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(x),
			PROP_ENTRY	(y),
			PROP_ENTRY	(width),
			PROP_ENTRY	(height),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(w=0, h=0)"),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(x)						{ return push(v, self(v)->x); }
	NB_PROP_GET(y)						{ return push(v, self(v)->y); }
	NB_PROP_GET(width)					{ return push(v, self(v)->GetWidth()); }
	NB_PROP_GET(height)					{ return push(v, self(v)->GetHeight()); }

	NB_PROP_SET(x)						{ self(v)->x = getInt(v, 2); return 0; }
	NB_PROP_SET(y)						{ self(v)->y = getInt(v, 2); return 0; }
	NB_PROP_SET(width)					{ self(v)->SetWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(height)					{ self(v)->SetHeight(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		new (self(v)) wxSize(optInt(v, 2, 0), optInt(v, 3, 0));
		return 0;
	}

	NB_FUNC(_tostring)
	{
		wxSize* o = self(v);
		pushFmt(v, "Size(%d, %d)", o->x, o->y);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Rect, NULL);

class NB_WxRect : TNitClass<wxRect>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(x),
			PROP_ENTRY	(y),
			PROP_ENTRY	(width),
			PROP_ENTRY	(height),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(x=0, y=0, w=0, h=0)\n"
										"(topLeft: Point, bottomRight: Point)\n"
										"(pt: Point, sz: Size)\n"
										"(sz: Size)\n"),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(x)						{ return push(v, self(v)->GetX()); }
	NB_PROP_GET(y)						{ return push(v, self(v)->GetY()); }
	NB_PROP_GET(width)					{ return push(v, self(v)->GetWidth()); }
	NB_PROP_GET(height)					{ return push(v, self(v)->GetHeight()); }

	NB_PROP_SET(x)						{ self(v)->SetX(getInt(v, 2)); return 0; }
	NB_PROP_SET(y)						{ self(v)->SetY(getInt(v, 2)); return 0; }
	NB_PROP_SET(width)					{ self(v)->SetWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(height)					{ self(v)->SetHeight(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		if (is<wxPoint>(v, 2))
		{
			if (is<wxPoint>(v, 3))
				new (self(v)) wxRect(*get<wxPoint>(v, 2), *get<wxPoint>(v, 3));
			else
				new (self(v)) wxRect(*get<wxPoint>(v, 2), *get<wxSize>(v, 3));
		}
		else if (is<wxSize>(v, 2))
			new (self(v)) wxRect(*get<wxSize>(v, 2));
		else
			new (self(v)) wxRect(optInt(v, 2, 0), optInt(v, 3, 0), optInt(v, 4, 0), optInt(v, 5, 0));
		return 0;			
	}

	NB_FUNC(_tostring)
	{
		wxRect* o = self(v);
		pushFmt(v, "Rect(%d, %d, %d, %d)", o->x, o->y, o->width, o->height);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Palette, NULL);

class NB_WxPalette : public TNitClass<wxPalette>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(colorCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(rgbArray: int[] // array of 0x00RRGGBB format"),
			FUNC_ENTRY_H(toIndex,		"(rgb: int): int"),
			FUNC_ENTRY_H(toRgb,			"(index: int): int"),
			FUNC_ENTRY_H(all,			"(): int[] // array of 0x00RRGGBB format"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(colorCount)				{ return push(v, self(v)->GetColoursCount()); }

	NB_CONS()
	{
		if (isNone(v, 2) || sq_gettype(v, 2) != OT_ARRAY)
			return sq_throwerror(v, "array of rgb expected");

		int size = sq_getsize(v, 2);
		vector<byte>::type r; r.reserve(size);
		vector<byte>::type g; r.reserve(size);
		vector<byte>::type b; r.reserve(size);

		for (NitIterator itr(v, 2); itr.hasNext(); itr.next())
		{
			int rgb = getInt(v, itr.valueIndex());
			r.push_back((rgb >> 16) & 0xFF);
			g.push_back((rgb >> 8) & 0xFF);
			b.push_back((rgb >> 0) & 0xFF);
		}

		new (self(v)) wxPalette(size, &r[0], &g[0], &b[0]);
		return 0;
	}
	
	NB_FUNC(toIndex)
	{ 
		int rgb = getInt(v, 2); 
		return self(v)->GetPixel(
			(rgb >> 16) & 0xFF,
			(rgb >> 8) & 0xFF,
			(rgb >> 0) & 0xFF);
	}

	NB_FUNC(toRgb)
	{
		int idx = getInt(v, 2);
		byte r, g, b;
		bool ok = self(v)->GetRGB(idx, &r, &g, &b);
		if (!ok) sq_throwerror(v, "invalid index");

		return push(v, (r << 24) | (g << 16) | (b));
	}

	NB_FUNC(all)
	{
		type* o = self(v);
		sq_newarray(v, 0);
		for (int idx=0; idx < o->GetColoursCount(); ++idx)
		{
			byte r, g, b;
			bool ok = o->GetRGB(idx, &r, &g, &b);
			if (!ok) sq_throwerror(v, "invalid index");
			arrayAppend(v, -1, (r << 24) | (g << 16) | (b));
		}
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Image, NULL);

class NB_WxImage : TNitClass<wxImage>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(width),
			PROP_ENTRY_R(height),
			PROP_ENTRY_R(size),
			PROP_ENTRY	(type),
			PROP_ENTRY_R(ok),
			PROP_ENTRY_R(hasAlpha),
			PROP_ENTRY	(hasMask),
			PROP_ENTRY	(maskColor),
			PROP_ENTRY	(palette),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(width, height: int, clear=true)\n"
										"(bitmap: wx.Bitmap)\n"
										"(name: string, type=wx.Bitmap.TYPE.ANY, index=-1)\n"
										"(source: StreamSource, type=wx.Bitmap.TYPE.ANY, index=-1)\n"),
			FUNC_ENTRY_H(toGreyscale,	"(): wx.Image\n"
										"(weight_r, weight_g, weight_b: float): wx.Image"),
			FUNC_ENTRY_H(toDisabled,	"(brightness=255): wx.Image"),
			FUNC_ENTRY_H(copy,			"(): wx.Image"),
			FUNC_ENTRY_H(destroy,		"() // purges instance"),
			FUNC_ENTRY_H(initAlpha,		"()"),
			FUNC_ENTRY_H(clear,			"()"),
			FUNC_ENTRY_H(blur,			"(radius: int): wx.Image"),
			FUNC_ENTRY_H(blurHorizontal,"(radius: int): wx.Image"),
			FUNC_ENTRY_H(blurVertical,	"(radius: int): wx.Image"),
			FUNC_ENTRY_H(mirror,		"(horizontally=true): wx.Image"),
			FUNC_ENTRY_H(paste,			"(image: wx.Image, x, y: int)"),
			FUNC_ENTRY_H(replace,		"(r1, g1, b1, r2, g2, b2)"),
			FUNC_ENTRY_H(crop,			"(rect: wx.Rect): wx.Image"),
			FUNC_ENTRY_H(getOption,		"(name: string): string"),
			FUNC_ENTRY_H(getOptionInt,	"(name: string): int"),
			FUNC_ENTRY_H(scale,			"(width, height: int): wx.Image"),
			FUNC_ENTRY_H(resize,		"(rect: wx.Rect, r=-1, g=-1, b=-1)"),
			FUNC_ENTRY_H(rotate,		"(radAngle: float, center: wx.Point): wx.Image"),
			FUNC_ENTRY_H(rotate90,		"(clockwise=true): wx.Image"),
			FUNC_ENTRY_H(rotate180,		"(): wx.Image"),
			FUNC_ENTRY_H(rotateHue,		"(angle: float) // angle: -1.0 ~ 1.0"),

			FUNC_ENTRY_H(loadFile,		"(reader: StreamReader, type=wx.Bitmap.TYPE.ANY, index=-1): bool"
			"\n"						"(name: string, type=wx.Bitmap.TYPE.ANY, index=-1): bool"),
			FUNC_ENTRY_H(saveFile,		"(writer: StreamWriter, type: wx.Bitmap.TYPE): bool"
			"\n"						"(name: string): bool"
			"\n"						"(name: string, type: wx.Bitmap.TYPE): bool"),

			FUNC_ENTRY	(_clone),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(width)					{ return push(v, self(v)->GetWidth()); }
	NB_PROP_GET(height)					{ return push(v, self(v)->GetHeight()); }
	NB_PROP_GET(size)					{ return push(v, self(v)->GetSize()); }
	NB_PROP_GET(type)					{ return push(v, (int)self(v)->GetType()); }
	NB_PROP_GET(ok)						{ return push(v, self(v)->IsOk()); }
	NB_PROP_GET(hasAlpha)				{ return push(v, self(v)->HasAlpha()); }
	NB_PROP_GET(hasMask)				{ return push(v, self(v)->HasMask()); }
	NB_PROP_GET(palette)				{ const wxPalette& pal = self(v)->GetPalette(); return pal.IsOk() ? push(v, pal) : 0; }

	NB_PROP_GET(maskColor)						
	{ 
		Color c;
		c.setAsRGBA(ARGB((self(v)->GetMaskRed() << 16) | (self(v)->GetMaskGreen() << 8) | (self(v)->GetMaskBlue())));

		return push(v, c); 
	}

	NB_PROP_SET(type)					{ self(v)->SetType((wxBitmapType)getInt(v, 2)); return 0; }
	NB_PROP_SET(hasMask)				{ self(v)->SetMask(getBool(v, 2)); return 0; }

	NB_PROP_SET(maskColor)
	{
		Color c = *get<Color>(v, 2);
		ARGB argb = c.getAsARGB();
		self(v)->SetMaskColour((argb & 0xFF0000) >> 16, (argb & 0xFF00) >> 8, (argb & 0xFF));
		return 0;
	}

	NB_PROP_SET(palette)				{ self(v)->SetPalette(*get<wxPalette>(v, 2)); return 0; }

	NB_CONS()
	{
		if (isInt(v, 2) && isInt(v, 3))
			new (self(v)) wxImage(getInt(v, 2), getInt(v, 3), optBool(v, 4, true));
		else if (is<wxBitmap>(v, 2))
		{
			new (self(v)) wxImage();
			*self(v) = get<wxBitmap>(v, 2)->ConvertToImage();
		}
		else if (isString(v, 2))
			new (self(v)) wxImage(getWxString(v, 2), (wxBitmapType)optInt(v, 3, wxBITMAP_TYPE_ANY), optInt(v, 4, -1));
		else if (is<StreamSource>(v, 2))
		{
			new (self(v)) wxImage();
			*self(v) = wxLoadNitImage(get<StreamSource>(v, 2), (wxBitmapType)optInt(v, 3, wxBITMAP_TYPE_ANY), optInt(v, 4, -1));
		}
		else
			return sq_throwerror(v, "invalid arguments");

		return 0;
	}

	NB_FUNC(toGreyscale)				
	{ 
		if (isNone(v, 2))
			return push(v, self(v)->ConvertToGreyscale());
		else
			return push(v, self(v)->ConvertToGreyscale(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4)));
	}

	NB_FUNC(toDisabled)					{ return push(v, self(v)->ConvertToDisabled(optInt(v, 2, 255))); }

	NB_FUNC(copy)						{ return push(v, self(v)->Copy()); }

	NB_FUNC(_clone)
	{
		*self(v) = self(v)->Copy();
		return 0;
	}

	NB_FUNC(destroy)					{ self(v)->Destroy(); sq_purgeinstance(v, 1); return 0; }

	NB_FUNC(initAlpha)					{ self(v)->InitAlpha(); return 0; }
	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }
	NB_FUNC(blur)						{ return push(v, self(v)->Blur(getInt(v, 2))); }
	NB_FUNC(blurHorizontal)				{ return push(v, self(v)->BlurHorizontal(getInt(v, 2))); }
	NB_FUNC(blurVertical)				{ return push(v, self(v)->BlurVertical(getInt(v, 2))); }
	NB_FUNC(mirror)						{ return push(v, self(v)->Mirror(optBool(v, 2, true))); }
	NB_FUNC(paste)						{ self(v)->Paste(*get<wxImage>(v, 2), getInt(v, 3), getInt(v, 4)); return 0; }
	NB_FUNC(replace)					{ self(v)->Replace(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), getInt(v, 6), getInt(v, 7)); return 0; }
	NB_FUNC(crop)						{ return push(v, self(v)->GetSubImage(*get<wxRect>(v, 2))); }
	NB_FUNC(getOption)					{ return push(v, self(v)->GetOption(getWxString(v, 2))); }
	NB_FUNC(getOptionInt)				{ return push(v, self(v)->GetOptionInt(getWxString(v, 2))); }
	NB_FUNC(scale)						{ return push(v, self(v)->Scale(getInt(v, 2), getInt(v, 3), wxIMAGE_QUALITY_BICUBIC)); }
	NB_FUNC(resize)						{ const wxRect& rt = *get<wxRect>(v, 2); self(v)->Resize(rt.GetSize(), rt.GetPosition(), optInt(v, 3, -1), optInt(v, 4, -1), optInt(v, 5, -1)); return 0; }
	NB_FUNC(rotate)						{ return push(v, self(v)->Rotate(getFloat(v, 2), *get<wxPoint>(v, 3), optBool(v, 4, true))); }
	NB_FUNC(rotate90)					{ return push(v, self(v)->Rotate90(optBool(v, 2, true))); }
	NB_FUNC(rotate180)					{ return push(v, self(v)->Rotate180()); }
	NB_FUNC(rotateHue)					{ self(v)->RotateHue(getFloat(v, 2)); return 0; }

	NB_FUNC(loadFile)
	{
		if (isString(v, 2))
			return push(v, self(v)->LoadFile(getString(v, 2), (wxBitmapType)optInt(v, 3, wxBITMAP_TYPE_ANY), optInt(v, 4, -1)));
		else
		{
			wxNitInputStream strm(get<StreamReader>(v, 2));
			return push(v, self(v)->LoadFile(strm, (wxBitmapType)optInt(v, 3, wxBITMAP_TYPE_ANY), optInt(v, 4, -1)));
		}
	}

	NB_FUNC(saveFile)
	{
		if (isString(v, 2))
		{
			if (isNone(v, 3))
				return push(v, self(v)->SaveFile(getString(v, 2)));
			else
				return push(v, self(v)->SaveFile(getString(v, 2), (wxBitmapType)getInt(v, 3)));
		}
		else
		{
			wxNitOutputStream strm(get<StreamWriter>(v, 2));
			return push(v, self(v)->SaveFile(strm, (wxBitmapType)getInt(v, 3)));
		}
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ImageList, wxObject);

class NB_WxImageList : TNitClass<wxImageList>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(imageCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()\n" 
										"(width, height: int, mask=true, initialCount=1)"),
			FUNC_ENTRY_H(add,			"(bitmap: wx.Bitmap, mask: wx.Bitmap=null): int\n"
										"(bitmap: wx.Bitmap, mask: color): int\n"
										"(icon: wx.Icon): int"),
			FUNC_ENTRY_H(getBitmap,		"(index: int): wx.Bitmap"),
			FUNC_ENTRY_H(getIcon,		"(index: int): wx.Icon"),
			FUNC_ENTRY_H(getSize,		"(index: int): wx.Size"),
			FUNC_ENTRY_H(remove,		"(index: int): bool"),
			FUNC_ENTRY_H(removeAll,		"(): bool"),
			FUNC_ENTRY_H(replace,		"(index: int, bitmap: wx.Bitmap, mask: wx.Bitmap=null): bool\n"
										"(index: int, icon: wx.Icon): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(imageCount)				{ return push(v, self(v)->GetImageCount()); }
	
	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new wxImageList());
		else
			setSelf(v, new wxImageList(getInt(v, 2), getInt(v, 3), optBool(v, 4, true), optInt(v, 5, 1)));
		return SQ_OK;
	}

	NB_FUNC(add)
	{
		if (is<wxIcon>(v, 2))
			return push(v, self(v)->Add(*get<wxIcon>(v, 2)));
		else if (isNone(v, 3) || is<wxBitmap>(v, 3))
			return push(v, self(v)->Add(*get<wxBitmap>(v, 2), *opt<wxBitmap>(v, 3, wxNullBitmap)));
		else
			return push(v, self(v)->Add(*get<wxBitmap>(v, 2), GetWxColor(v, 3)));
	}

	NB_FUNC(replace)
	{
		if (is<wxIcon>(v, 3))
			return push(v, self(v)->Replace(getInt(v, 2), *get<wxIcon>(v, 3)));
		else
			return push(v, self(v)->Replace(getInt(v, 2), *get<wxBitmap>(v, 3), *opt<wxBitmap>(v, 4, wxNullBitmap)));
	}

	NB_FUNC(getBitmap)					{ return push(v, self(v)->GetBitmap(getInt(v, 2))); }
	NB_FUNC(getIcon)					{ return push(v, self(v)->GetIcon(getInt(v, 2))); }
	NB_FUNC(getSize)					{ wxSize sz; return self(v)->GetSize(getInt(v, 2), sz.x, sz.y) ? push(v, sz) : 0; }
	NB_FUNC(remove)						{ return push(v, self(v)->Remove(getInt(v, 2))); }
	NB_FUNC(removeAll)					{ return push(v, self(v)->RemoveAll()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Bitmap, NULL);

class NB_WxBitmap : TNitClass<wxBitmap>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(depth),
			PROP_ENTRY	(width),
			PROP_ENTRY	(height),
			PROP_ENTRY_R(size),
			PROP_ENTRY_R(ok),
			PROP_ENTRY	(palette),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(width, height)\n"
										"(name: string, type=wx.Bitmap.TYPE.BITMAP_DEFAULT)\n"
										"(source: StreamSource)\n"
										"(image: wx.Image)\n"
										"(icon: wx.Icon)"),
			FUNC_ENTRY_H(saveFile,		"(name: string, type: TYPE, palette: wx.Palette=null): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "TYPE");
		newSlot(v, -1, "ANY",				(int)wxBITMAP_TYPE_ANY);
		newSlot(v, -1, "BITMAP_DEFAULT",	(int)wxBITMAP_DEFAULT_TYPE);
		newSlot(v, -1, "ICON_DEFAULT",		(int)wxICON_DEFAULT_TYPE);
		newSlot(v, -1, "CURSOR_DEFAULT",	(int)wxCURSOR_DEFAULT_TYPE);
		newSlot(v, -1, "INVALID",			(int)wxBITMAP_TYPE_INVALID);
		newSlot(v, -1, "BMP",				(int)wxBITMAP_TYPE_BMP);
		newSlot(v, -1, "BMP_RESOURCE",		(int)wxBITMAP_TYPE_BMP_RESOURCE);
		newSlot(v, -1, "ICO",				(int)wxBITMAP_TYPE_ICO);
		newSlot(v, -1, "ICO_RESOURCE",		(int)wxBITMAP_TYPE_ICO_RESOURCE);
		newSlot(v, -1, "CUR",				(int)wxBITMAP_TYPE_CUR);
		newSlot(v, -1, "CUR_RESOURCE",		(int)wxBITMAP_TYPE_CUR_RESOURCE);
		newSlot(v, -1, "TIFF",				(int)wxBITMAP_TYPE_TIFF);
		newSlot(v, -1, "TIFF_RESOURCE",		(int)wxBITMAP_TYPE_TIFF_RESOURCE);
		newSlot(v, -1, "GIF", 				(int)wxBITMAP_TYPE_GIF);
		newSlot(v, -1, "GIF_RESOURCE",		(int)wxBITMAP_TYPE_GIF_RESOURCE);
		newSlot(v, -1, "PNG", 				(int)wxBITMAP_TYPE_PNG);
		newSlot(v, -1, "PNG_RESOURCE", 		(int)wxBITMAP_TYPE_PNG_RESOURCE);
		newSlot(v, -1, "JPEG", 				(int)wxBITMAP_TYPE_JPEG);
		newSlot(v, -1, "JPEG_RESOURCE", 	(int)wxBITMAP_TYPE_JPEG_RESOURCE);
		newSlot(v, -1, "ICON", 				(int)wxBITMAP_TYPE_ICON);
		newSlot(v, -1, "ICON_RESOURCE", 	(int)wxBITMAP_TYPE_ICON_RESOURCE);
		newSlot(v, -1, "ANI", 				(int)wxBITMAP_TYPE_ANI);
		newSlot(v, -1, "IFF", 				(int)wxBITMAP_TYPE_IFF);
		newSlot(v, -1, "TGA", 				(int)wxBITMAP_TYPE_TGA);
		newSlot(v, -1, "XBM", 				(int)wxBITMAP_TYPE_XBM);
		newSlot(v, -1, "XPM", 				(int)wxBITMAP_TYPE_XPM);
		newSlot(v, -1, "MACCURSOR",			(int)wxBITMAP_TYPE_MACCURSOR);
		sq_poptop(v);
	}

	NB_PROP_GET(depth)					{ return push(v, self(v)->GetDepth()); }
	NB_PROP_GET(width)					{ return push(v, self(v)->GetWidth()); }
	NB_PROP_GET(height)					{ return push(v, self(v)->GetHeight()); }
	NB_PROP_GET(size)					{ return push(v, self(v)->GetSize()); }
	NB_PROP_GET(ok)						{ return push(v, self(v)->IsOk()); }
	NB_PROP_GET(palette)				{ wxPalette* pal = self(v)->GetPalette(); return pal && pal->IsOk() ? push(v, *pal) : 0; }

	NB_PROP_SET(depth)					{ self(v)->SetDepth(getInt(v, 2)); return 0; }
	NB_PROP_SET(width)					{ self(v)->SetWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(height)					{ self(v)->SetHeight(getInt(v, 2)); return 0; }
	NB_PROP_SET(palette)				{ self(v)->SetPalette(*get<wxPalette>(v, 2)); return 0; }

	NB_CONS()
	{
		if (isInt(v, 2) && isInt(v, 3))
			new (self(v)) wxBitmap(getInt(v, 2), getInt(v, 3));
		else if (isString(v, 2))
			new (self(v)) wxBitmap(getWxString(v, 2), (wxBitmapType)optInt(v, 3, wxBITMAP_DEFAULT_TYPE));
		else if (is<StreamSource>(v, 2))
			new (self(v)) wxBitmap(wxLoadNitImage(get<StreamSource>(v, 2)));
		else if (is<wxImage>(v, 2))
			new (self(v)) wxBitmap(*get<wxImage>(v, 2));
		else if (is<wxIcon>(v, 2))
		{
			new (self(v)) wxBitmap();
			self(v)->CopyFromIcon(*get<wxIcon>(v, 2));
		}
		else
			return sq_throwerror(v, "invalid arguments");

		return 0;
	}

	NB_FUNC(saveFile)					{ return push(v, self(v)->SaveFile(getWxString(v, 2), (wxBitmapType)getInt(v, 3), opt<wxPalette>(v, 4, NULL))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Icon, NULL);

class NB_WxIcon: TNitClass<wxIcon>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(depth),
			PROP_ENTRY	(width),
			PROP_ENTRY	(height),
			PROP_ENTRY_R(ok),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(bitmap: Bitmap)\n"
										"(name: string, type=wx.BITMAP_TYPE.ICON_DEFAULT, desiredWidth=-1, desiredHeight=-1)\n"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(depth)					{ return push(v, self(v)->GetDepth()); }
	NB_PROP_GET(width)					{ return push(v, self(v)->GetWidth()); }
	NB_PROP_GET(height)					{ return push(v, self(v)->GetHeight()); }
	NB_PROP_GET(ok)						{ return push(v, self(v)->IsOk()); }

	NB_PROP_SET(depth)					{ self(v)->SetDepth(getInt(v, 2)); return 0; }
	NB_PROP_SET(width)					{ self(v)->SetWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(height)					{ self(v)->SetHeight(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		if (is<wxBitmap>(v, 2))
		{
			new (self(v)) wxIcon();
			self(v)->CopyFromBitmap(*get<wxBitmap>(v, 2));
		}
		else if (isString(v, 2))
			new (self(v)) wxIcon(getWxString(v, 2), (wxBitmapType)optInt(v, 3, wxICON_DEFAULT_TYPE), optInt(v, 4, -1), optInt(v, 5, -1));
		else
			return sq_throwerror(v, "invalid arguments");

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Cursor, NULL);

class NB_WxCursor : TNitClass<wxCursor>
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
			CONS_ENTRY_H(
				"(name: string, type: wx.BITMAP.CURSOR_DEFAULT_TYPE, hotSpotX=0, hotSpotY=0)\n"
				"(cursorID: wx.CURSOR)\n"
				"(image: wx.Image)"),

			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isString(v, 2))
			new (self(v)) wxCursor(getWxString(v, 2), (wxBitmapType)optInt(v, 3, wxCURSOR_DEFAULT_TYPE), optInt(v, 4, 0), optInt(v, 5, 0));
		else if (is<wxImage>(v, 2))
			new (self(v)) wxCursor(*get<wxImage>(v, 2));
		else
			new (self(v)) wxCursor((wxStockCursor)getInt(v, 2));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ArtProvider, wxObject);

class NB_WxArtProvider : TNitClass<wxArtProvider>
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
			FUNC_ENTRY_H(hasNativeProvider,		"[class] (): bool"),
			FUNC_ENTRY_H(getBitmap,				"[class] (id: wx.ART, client=wx.ART_CLIENT.OTHER, size=wx.DEFAULT.SIZE): wx.Bitmap"),
			FUNC_ENTRY_H(getIcon,				"[class] (id: wx.ART, client=wx.ART_CLIENT.OTHER, size=wx.DEFAULT.SIZE): wx.Icon"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(hasNativeProvider)			{ return push(v, type::HasNativeProvider()); }
	NB_FUNC(getBitmap)					{ return push(v, type::GetBitmap(getString(v, 2), optString(v, 3, wxART_OTHER), *opt<wxSize>(v, 4, wxDefaultSize))); }
	NB_FUNC(getIcon)					{ return push(v, type::GetIcon(getString(v, 2), optString(v, 3, wxART_OTHER), *opt<wxSize>(v, 4, wxDefaultSize))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Font, NULL);

class NB_WxFont : TNitClass<wxFont>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(pointSize),
			PROP_ENTRY	(pixelSize),
			PROP_ENTRY	(family),
			PROP_ENTRY	(style),
			PROP_ENTRY	(weight),
			PROP_ENTRY	(faceName),
			PROP_ENTRY	(underlined),
			PROP_ENTRY	(encoding),
			PROP_ENTRY_R(fixedWidth),
			PROP_ENTRY_R(nativeFontInfo),
			PROP_ENTRY_R(nativeFontUserDesc),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(nativeFontInfo: string)\n"
										"(pointSize: int, family: FAMILY, style: STYLE, weight: WEIGHT, underline=false, facename=\"\", encoding=ENCODING.DEFAULT)\n"
										"(pixelSize: wx.Size, family: FAMILY, style: STYLE, weight: WEIGHT, underline=false, facename=\"\", encoding=ENCODING.DEFAULT)"),
 			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "FAMILY");
		newSlot(v, -1, "DEFAULT",		(int)wxFONTFAMILY_DEFAULT);
		newSlot(v, -1, "DECORATIVE",	(int)wxFONTFAMILY_DECORATIVE);
		newSlot(v, -1, "ROMAN",			(int)wxFONTFAMILY_ROMAN);
		newSlot(v, -1, "SCRIPT",		(int)wxFONTFAMILY_SCRIPT);
		newSlot(v, -1, "SWISS",			(int)wxFONTFAMILY_SWISS);
		newSlot(v, -1, "MODERN",		(int)wxFONTFAMILY_MODERN);
		newSlot(v, -1, "TELETYPE",		(int)wxFONTFAMILY_TELETYPE);
		sq_poptop(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "NORMAL",		(int)wxFONTSTYLE_NORMAL);
		newSlot(v, -1, "ITALIC",		(int)wxFONTSTYLE_ITALIC);
		newSlot(v, -1, "SLANT",			(int)wxFONTSTYLE_SLANT);
		sq_poptop(v);

		addStaticTable(v, "WEIGHT");
		newSlot(v, -1, "NORMAL",		(int)wxFONTWEIGHT_NORMAL);
		newSlot(v, -1, "LIGHT",			(int)wxFONTWEIGHT_LIGHT);
		newSlot(v, -1, "BOLD",			(int)wxFONTWEIGHT_BOLD);
		sq_poptop(v);

		addStaticTable(v, "ENCODING");
		newSlot(v, -1, "SYSTEM",		(int)wxFONTENCODING_SYSTEM);
		newSlot(v, -1, "DEFAULT",		(int)wxFONTENCODING_DEFAULT);
		newSlot(v, -1, "ISO8859_1",		(int)wxFONTENCODING_ISO8859_1);
		newSlot(v, -1, "CP437",  		(int)wxFONTENCODING_CP437);
		newSlot(v, -1, "CP850",  		(int)wxFONTENCODING_CP850);
		newSlot(v, -1, "CP852",  		(int)wxFONTENCODING_CP852);
		newSlot(v, -1, "CP855",  		(int)wxFONTENCODING_CP855);
		newSlot(v, -1, "CP866",  		(int)wxFONTENCODING_CP866);
		newSlot(v, -1, "CP874",  		(int)wxFONTENCODING_CP874);
		newSlot(v, -1, "CP932",  		(int)wxFONTENCODING_CP932);
		newSlot(v, -1, "CP936",  		(int)wxFONTENCODING_CP936);
		newSlot(v, -1, "CP949",  		(int)wxFONTENCODING_CP949);
		newSlot(v, -1, "CP950",  		(int)wxFONTENCODING_CP950);
		newSlot(v, -1, "CP1250", 		(int)wxFONTENCODING_CP1250);
		newSlot(v, -1, "CP1251", 		(int)wxFONTENCODING_CP1251);
		newSlot(v, -1, "CP1252", 		(int)wxFONTENCODING_CP1252);
		newSlot(v, -1, "CP1253", 		(int)wxFONTENCODING_CP1253);
		newSlot(v, -1, "CP1254", 		(int)wxFONTENCODING_CP1254);
		newSlot(v, -1, "CP1255", 		(int)wxFONTENCODING_CP1255);
		newSlot(v, -1, "CP1256", 		(int)wxFONTENCODING_CP1256);
		newSlot(v, -1, "CP1257", 		(int)wxFONTENCODING_CP1257);
		newSlot(v, -1, "UTF7",	 		(int)wxFONTENCODING_UTF7);
		newSlot(v, -1, "UTF8",	 		(int)wxFONTENCODING_UTF8);
		newSlot(v, -1, "UTF16", 		(int)wxFONTENCODING_UTF16);
		newSlot(v, -1, "UTF32", 		(int)wxFONTENCODING_UTF32);
		newSlot(v, -1, "UNICODE",		(int)wxFONTENCODING_UNICODE);
		newSlot(v, -1, "UTF16BE", 		(int)wxFONTENCODING_UTF16BE);
		newSlot(v, -1, "UTF16LE", 		(int)wxFONTENCODING_UTF16LE);
		newSlot(v, -1, "UTF32BE", 		(int)wxFONTENCODING_UTF32BE);
		newSlot(v, -1, "UTF32LE", 		(int)wxFONTENCODING_UTF32LE);
		newSlot(v, -1, "EUC_KR",		(int)wxFONTENCODING_EUC_KR);
		newSlot(v, -1, "SHIFT_JIS",		(int)wxFONTENCODING_SHIFT_JIS);
		sq_poptop(v);
	};

	NB_PROP_GET(pointSize)				{ return push(v, self(v)->GetPointSize()); }
	NB_PROP_GET(pixelSize)				{ return push(v, self(v)->GetPixelSize()); }
	NB_PROP_GET(family)					{ return push(v, (int)self(v)->GetFamily()); }
	NB_PROP_GET(style)					{ return push(v, (int)self(v)->GetStyle()); }
	NB_PROP_GET(weight)					{ return push(v, (int)self(v)->GetWeight()); }
	NB_PROP_GET(faceName)				{ return push(v, self(v)->GetFaceName()); }
	NB_PROP_GET(underlined)				{ return push(v, self(v)->GetUnderlined()); }
	NB_PROP_GET(encoding)				{ return push(v, (int)self(v)->GetEncoding()); }
	NB_PROP_GET(fixedWidth)				{ return push(v, self(v)->IsFixedWidth()); }
	NB_PROP_GET(nativeFontInfo)			{ return push(v, self(v)->GetNativeFontInfoDesc()); }
	NB_PROP_GET(nativeFontUserDesc)		{ return push(v, self(v)->GetNativeFontInfoUserDesc()); }

	NB_PROP_SET(pointSize)				{ self(v)->SetPointSize(getInt(v, 2)); return 0; }
	NB_PROP_SET(pixelSize)				{ self(v)->SetPixelSize(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(family)					{ self(v)->SetFamily((wxFontFamily)getInt(v, 2)); return 0; }
	NB_PROP_SET(style)					{ self(v)->SetStyle((wxFontStyle)getInt(v, 2)); return 0; }
	NB_PROP_SET(weight)					{ self(v)->SetWeight((wxFontWeight)getInt(v, 2)); return 0; }
	NB_PROP_SET(faceName)				{ self(v)->SetFaceName(getWxString(v, 2)); return 0; }
	NB_PROP_SET(underlined)				{ self(v)->SetUnderlined(getBool(v, 2)); return 0; }
	NB_PROP_SET(encoding)				{ self(v)->SetEncoding((wxFontEncoding)getInt(v, 2)); return 0; }

	NB_CONS()
	{
		if (isString(v, 2))
			new (self(v)) wxFont(getWxString(v, 2));
		else if (is<wxSize>(v, 2))
			new (self(v)) wxFont(*get<wxSize>(v, 2), (wxFontFamily)getInt(v, 3), (wxFontStyle)getInt(v, 4), (wxFontWeight)getInt(v, 5), optBool(v, 6, false), optWxString(v, 7, ""), (wxFontEncoding)optInt(v, 8, wxFONTENCODING_DEFAULT));
		else
			new (self(v)) wxFont(getInt(v, 2), (wxFontFamily)getInt(v, 3), (wxFontStyle)getInt(v, 4), (wxFontWeight)getInt(v, 5), optBool(v, 6, false), optWxString(v, 7, ""), (wxFontEncoding)optInt(v, 8, wxFONTENCODING_DEFAULT));

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Object, NULL);

class NB_WxObject : TNitClass<wxObject>
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
			FUNC_ENTRY_H(destroy,		"() // purges instance (using C++ delete)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(destroy)
	{ 
		wxObject* obj = self(v); 
		sq_purgeinstance(v, 1); 
		delete obj; 
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Event, wxObject);

class NB_WxEvent : TNitClass<wxEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(eventType),
			PROP_ENTRY_R(eventObject),
			PROP_ENTRY_R(timestamp),
			PROP_ENTRY_R(id),
			PROP_ENTRY_R(skipped),
			PROP_ENTRY_R(eventCategory),
			PROP_ENTRY_R(shouldPropagate),
			PROP_ENTRY_R(commandEvent),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(clone,				"(): Event"),
			FUNC_ENTRY_H(skip,				"(skip=true)"),
			FUNC_ENTRY_H(resumePropagation,	"(propagationLevel: int)"),
			FUNC_ENTRY_H(stopPropagation,	"(): int"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(eventType)				{ return push(v, self(v)->GetEventType()); }
	NB_PROP_GET(eventObject)			{ return push(v, self(v)->GetEventObject()); }
	NB_PROP_GET(timestamp)				{ return push(v, (int)self(v)->GetTimestamp()); }
	NB_PROP_GET(id)						{ return push(v, self(v)->GetId()); }
	NB_PROP_GET(skipped)				{ return push(v, self(v)->GetSkipped()); }
	NB_PROP_GET(eventCategory)			{ return push(v, (int)self(v)->GetEventCategory()); }
	NB_PROP_GET(shouldPropagate)		{ return push(v, self(v)->ShouldPropagate()); }
	NB_PROP_GET(commandEvent)			{ return push(v, self(v)->IsCommandEvent()); }

	NB_FUNC(clone)						{ return push(v, self(v)->Clone()); }
	NB_FUNC(skip)						{ self(v)->Skip(optBool(v, 2, true)); return 0; }
	NB_FUNC(resumePropagation)			{ self(v)->ResumePropagation(getInt(v, 2)); return 0; }
	NB_FUNC(stopPropagation)			{ return push(v, self(v)->StopPropagation()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, EvtHandler, wxObject);

class NB_WxEvtHandler : TNitClass<wxEvtHandler>
{
public:
	class EventClosure : public wxObject
	{
	public:
		EventClosure(HSQUIRRELVM v, int idx)
		{
			_closure = new ScriptClosure(v, idx);
		}

		virtual ~EventClosure()
		{
//			LOG(0, ".. EventClosure %08x destroyed\n", this);
		}

		ScriptClosure* GetClosure() { return _closure; }

		nit::Ref<ScriptClosure> _closure;
	};

	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(clientObject),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(queueEvent,			"(evt: Event)"),
			FUNC_ENTRY_H(deletePendingEvents,	"()"),

			FUNC_ENTRY_H(bind,					"(eventType: EVT, env, closure) // calls closure.bindenv(env) if env is not null"
			"\n"								"(eventType: EVT, idBegin: int, env, closure) // calls closure.bindenv(env) if env is not null"
			"\n"								"(eventType: EVT, idBegin: int, idEnd: int, env, closure) // calls closure.bindenv(env) if env is not null"),

			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(clientObject)			{ return PushWxScriptData(v, self(v)->GetClientObject()); }
	NB_PROP_SET(clientObject)									
	{ 
		type* o = self(v);
		wxClientData* obj = o->GetClientObject();
		wxClientData* nobj = GetWxScriptData(v, 2, obj);

		if (obj != nobj)
			o->SetClientObject(nobj);
		return 0;
	}

	NB_FUNC(queueEvent)					{ self(v)->QueueEvent(get<wxEvent>(v, 2)); return 0; }
	NB_FUNC(deletePendingEvents)		{ self(v)->DeletePendingEvents(); return 0; }

	NB_FUNC(bind)
	{
		int envIdx, closureIdx;
		int idBegin, idEnd;

		if (!isNone(v, 4) && sq_gettype(v, 4) == OT_CLOSURE)
		{
			envIdx = 3;
			closureIdx = 4;
			idBegin = idEnd = wxID_ANY;
		}
		else if (!isNone(v, 5) && sq_gettype(v, 5) == OT_CLOSURE)
		{
			envIdx = 4;
			closureIdx = 5;
			idBegin = idEnd = getInt(v, 3);
		}
		else if (!isNone(v, 6) && sq_gettype(v, 6) == OT_CLOSURE)
		{
			envIdx = 5;
			closureIdx = 6;
			idBegin = getInt(v, 3);
			idEnd = getInt(v, 4);
		}
		else
		{
			return sq_throwerror(v, "invalid params");
		}

		wxEvtHandler* o = self(v);
		wxEventType evtType = (wxEventType)getInt(v, 2);

		if (sq_gettype(v, envIdx) != OT_NULL)
		{
			sq_push(v, envIdx);
			sq_bindenv(v, closureIdx);
			sq_replace(v, closureIdx);
		}

		wxObject* closure = new EventClosure(v, closureIdx);

		o->Bind(evtType, _Dispatcher, idBegin, idEnd, closure);

		return 0;
	}

	static void _Dispatcher(wxEvent& event)
	{
		EventClosure* ec = (EventClosure*)event.m_callbackUserData;
		if (ec == NULL)
			return;

		ScriptClosure* cl = ec->GetClosure();
		TNitBind::push(cl->getRuntime()->getWorker(), &event);
		ScriptResult ret = cl->call(1, false);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Validator, wxEvtHandler);

class NB_WxValidator : TNitClass<wxValidator>
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
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, AppConsole, wxEvtHandler);

class NB_WxAppConsole : TNitClass<wxAppConsole>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(appDisplayName),
			PROP_ENTRY	(appName),
			PROP_ENTRY	(className),
			PROP_ENTRY	(vendorDisplayName),
			PROP_ENTRY	(vendorName),
			PROP_ENTRY_R(argc),
			PROP_ENTRY_R(argv),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(appDisplayName)			{ return push(v, self(v)->GetAppDisplayName()); }
	NB_PROP_GET(appName)				{ return push(v, self(v)->GetAppName()); }
	NB_PROP_GET(className)				{ return push(v, self(v)->GetClassName()); }
	NB_PROP_GET(vendorDisplayName)		{ return push(v, self(v)->GetVendorDisplayName()); }
	NB_PROP_GET(vendorName)				{ return push(v, self(v)->GetVendorName()); }

	NB_PROP_SET(appDisplayName)			{ self(v)->SetAppDisplayName(getWxString(v, 2)); return 0; }
	NB_PROP_SET(appName)				{ self(v)->SetAppName(getWxString(v, 2)); return 0; }
	NB_PROP_SET(className)				{ self(v)->SetClassName(getWxString(v, 2)); return 0; }
	NB_PROP_SET(vendorDisplayName)		{ self(v)->SetVendorDisplayName(getWxString(v, 2)); return 0; }
	NB_PROP_SET(vendorName)				{ self(v)->SetVendorName(getWxString(v, 2)); return 0; }

	NB_PROP_GET(argc)					{ return push(v, self(v)->argc); }

	NB_PROP_GET(argv)
	{
		int argc = self(v)->argc;
		wxChar** argv = self(v)->argv;

		sq_newarray(v, argc);
		for (int i=0; i<argc; ++i)
		{
			sq_pushinteger(v, i);
			push(v, wxString(argv[i]));
			sq_set(v, -3);
		}

		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, App, wxAppConsole);

class NB_WxApp : TNitClass<wxApp>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(topWindow),
			PROP_ENTRY	(exitOnFrameDelete),
			PROP_ENTRY_R(active),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(safeYield,		"(win: Window, onlyIfNeeded: bool): bool"), // TODO: really safe?
			FUNC_ENTRY_H(safeYieldFor,	"(win: Window, eventsToProcess: EVT): bool"), // TODO: really safe?
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "QUERY_END_SESSION",	(int)wxEVT_QUERY_END_SESSION);
		newSlot(v, -1, "END_SESSION",		(int)wxEVT_END_SESSION);
		newSlot(v, -1, "ACTIVATE_APP",		(int)wxEVT_ACTIVATE_APP);
		newSlot(v, -1, "HIBERNATE",			(int)wxEVT_HIBERNATE);
		newSlot(v, -1, "IDLE",				(int)wxEVT_IDLE);
		sq_poptop(v);
	}

	NB_PROP_GET(topWindow)				{ return push(v, self(v)->GetTopWindow()); }
	NB_PROP_GET(exitOnFrameDelete)		{ return push(v, self(v)->GetExitOnFrameDelete()); }
	NB_PROP_GET(active)					{ return push(v, self(v)->IsActive()); }

	NB_PROP_SET(topWindow)				{ self(v)->SetTopWindow(opt<wxWindow>(v, 2, NULL)); return 0; }
	NB_PROP_SET(exitOnFrameDelete)		{ self(v)->SetExitOnFrameDelete(getBool(v, 2)); return 0; }

	NB_FUNC(safeYield)					{ return push(v, self(v)->SafeYield(opt<wxWindow>(v, 2, NULL), getBool(v, 3))); }
	NB_FUNC(safeYieldFor)				{ return push(v, self(v)->SafeYieldFor(opt<wxWindow>(v, 2, NULL), getInt(v, 3))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_RAW_PTR(NITWX_API, nit::wxNitApp, wxApp);

class NB_WxNitApp : TNitClass<nit::wxNitApp>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(mainApp),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(mainApp)				{ return push(v, self(v)->getMainApp()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Window, wxEvtHandler);

class NB_WxWindow : TNitClass<wxWindow>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(name),
			PROP_ENTRY	(label),
			PROP_ENTRY	(id),
			PROP_ENTRY	(position),

			PROP_ENTRY	(size),
			PROP_ENTRY	(minSize),
			PROP_ENTRY	(maxSize),

			PROP_ENTRY	(foreColor),
			PROP_ENTRY	(backColor),

			PROP_ENTRY	(clientSize),
			PROP_ENTRY	(minClientSize),
			PROP_ENTRY	(maxClientSize),

			PROP_ENTRY	(virtualSize),

			PROP_ENTRY_R(handle),

			PROP_ENTRY	(parent),
			PROP_ENTRY_R(grandParent),
			PROP_ENTRY_R(nextSibling),
			PROP_ENTRY_R(prevSibling),

			PROP_ENTRY	(visible),
			PROP_ENTRY	(enabled),
			PROP_ENTRY_R(thisEnabled),
			PROP_ENTRY_R(shownOnScreen),
			PROP_ENTRY_R(topLevel),
			PROP_ENTRY_R(beingDeleted),

			PROP_ENTRY	(toolTip),
			PROP_ENTRY	(helpText),

			PROP_ENTRY	(sizer),
			PROP_ENTRY	(containingSizer),

			PROP_ENTRY	(windowStyle),
			PROP_ENTRY	(extraStyle),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: wx.Window=null, id=wx.ID.ANY, pos=null, size=null, style=0, name=null"),

			FUNC_ENTRY_H(destroy,		"(): bool // returns true if ok"),
			FUNC_ENTRY_H(destroyChildren, "(): bool"),

			FUNC_ENTRY_H(show,			"(show=true): bool // returns true if visibilty changed"),
			FUNC_ENTRY_H(hide,			"(): bool // == Show(false)"),

			FUNC_ENTRY_H(hasFocus,		"(): bool"),
			FUNC_ENTRY_H(setFocus,		"()"),

			FUNC_ENTRY_H(freeze,		"()"),
			FUNC_ENTRY_H(thaw,			"()"),

			FUNC_ENTRY_H(popupMenu,		"(menu: Menu, pos: Point=null): bool"),
			FUNC_ENTRY_H(getPopupMenuSelectionFromUser, "(menu: Menu, pos: Point=null): ID("),

			FUNC_ENTRY_H(captureMouse,	"()"),
			FUNC_ENTRY_H(releaseMouse,	"()"),

			FUNC_ENTRY_H(clientToScreen,"(clientPt: Point): Point"),
			FUNC_ENTRY_H(screenToClient,"(screenPt: Point): Point"),

			FUNC_ENTRY_H(setSizeHints,	"(minSize: wx.Size, maxSize=wx.DEFAULT.SIZE, incSize=wx.DEFAULT.SIZE)"),
			FUNC_ENTRY_H(setSizerAndFit,"(Sizer)"),
			FUNC_ENTRY_H(layout,		"()"),

			FUNC_ENTRY_H(setWindowStyle,"(style: int, on=true)"),
			FUNC_ENTRY_H(setExtraStyle,	"(style: int, on=true)"),
			FUNC_ENTRY_H(refresh,		"(eraseBackground=true, rect: Rect=null)"),

			FUNC_ENTRY_H(getTextExtent,	"(string): wx.Size"),

			FUNC_ENTRY_H(findWindow,	"[class] (id: int): wx.Window\n"
										"[class] (name: string): wx.Window\n"
										"(id: int, recursive=true): wx.Window\n"
										"(name: string, recursive=true): wx.Window"),

			FUNC_ENTRY_H(findFocus,		"[class] (): wx.Window"),
			FUNC_ENTRY_H(getCapture,	"[class] (): wx.Window"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "BORDER_DEFAULT",		(int)wxBORDER_DEFAULT);
		newSlot(v, -1, "BORDER_SIMPLE",			(int)wxBORDER_SIMPLE);
		newSlot(v, -1, "BORDER_STATIC",			(int)wxBORDER_STATIC);
		newSlot(v, -1, "BORDER_SUNKEN",			(int)wxBORDER_SUNKEN);
		newSlot(v, -1, "BORDER_RAISED",			(int)wxBORDER_RAISED);
		newSlot(v, -1, "BORDER_THEME",			(int)wxBORDER_THEME);
		newSlot(v, -1, "BORDER_NONE",			(int)wxBORDER_NONE);

		newSlot(v, -1, "TRANSPARENT",			(int)wxTRANSPARENT_WINDOW);
		newSlot(v, -1, "TAB_TRAVERSAL",			(int)wxTAB_TRAVERSAL);
		newSlot(v, -1, "WANTS_CHARS",			(int)wxWANTS_CHARS);
		newSlot(v, -1, "VSCROLL",				(int)wxVSCROLL);
		newSlot(v, -1, "HSCROLL",				(int)wxHSCROLL);
		newSlot(v, -1, "ALWAYS_SHOW_SB",		(int)wxALWAYS_SHOW_SB);
		newSlot(v, -1, "CLIP_CHILDREN",			(int)wxCLIP_CHILDREN);
		newSlot(v, -1, "RESIZE_BORDER",			(int)wxRESIZE_BORDER);

		newSlot(v, -1, "EX_VALIDATE_RECURSIVELY",(int)wxWS_EX_VALIDATE_RECURSIVELY);
		newSlot(v, -1, "EX_BLOCK_EVENTS",		(int)wxWS_EX_BLOCK_EVENTS);
		newSlot(v, -1, "EX_TRANSIENT",			(int)wxWS_EX_TRANSIENT);
		newSlot(v, -1, "EX_CONTEXTHELP",		(int)wxWS_EX_CONTEXTHELP);
		newSlot(v, -1, "EX_PROCESS_IDLE",		(int)wxWS_EX_PROCESS_IDLE);
		newSlot(v, -1, "EX_PROCESS_UI_UPDATES",	(int)wxWS_EX_PROCESS_UI_UPDATES);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "ACTIVATE",				(int)wxEVT_ACTIVATE);
		newSlot(v, -1, "CHILD_FOCUS",			(int)wxEVT_CHILD_FOCUS);
		newSlot(v, -1, "CONTEXT_MENU",			(int)wxEVT_CONTEXT_MENU);
		newSlot(v, -1, "HELP",					(int)wxEVT_HELP);
		newSlot(v, -1, "DROP_FILES",			(int)wxEVT_DROP_FILES);
		newSlot(v, -1, "ERASE_BACKGROUND",		(int)wxEVT_ERASE_BACKGROUND);
 		newSlot(v, -1, "SET_FOCUS",				(int)wxEVT_SET_FOCUS);
 		newSlot(v, -1, "KILL_FOCUS",			(int)wxEVT_KILL_FOCUS);
		newSlot(v, -1, "IDLE",					(int)wxEVT_IDLE);
		newSlot(v, -1, "KEY_DOWN",				(int)wxEVT_KEY_DOWN);
		newSlot(v, -1, "KEY_UP",				(int)wxEVT_KEY_UP);
		newSlot(v, -1, "CHAR",					(int)wxEVT_CHAR);
		newSlot(v, -1, "MOUSE_CAPTURE_LOST",	(int)wxEVT_MOUSE_CAPTURE_LOST);
		newSlot(v, -1, "MOUSE_CAPTURE_CHANGED",	(int)wxEVT_MOUSE_CAPTURE_CHANGED);
		newSlot(v, -1, "PAINT",					(int)wxEVT_PAINT);
		newSlot(v, -1, "SET_CURSOR",			(int)wxEVT_SET_CURSOR);
		newSlot(v, -1, "SHOW",					(int)wxEVT_SHOW);
		newSlot(v, -1, "SIZE",					(int)wxEVT_SIZE);
		newSlot(v, -1, "SYS_COLOR_CHANGED",		(int)wxEVT_SYS_COLOUR_CHANGED);
		newSlot(v, -1, "UPDATE_UI",				(int)wxEVT_UPDATE_UI);
		sq_poptop(v);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->GetName()); }
	NB_PROP_GET(label)					{ return push(v, self(v)->GetLabel()); }
	NB_PROP_GET(id)						{ return push(v, self(v)->GetId()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }

	NB_PROP_GET(size)					{ return push(v, self(v)->GetSize()); }
	NB_PROP_GET(minSize)				{ return push(v, self(v)->GetMinSize()); }
	NB_PROP_GET(maxSize)				{ return push(v, self(v)->GetMaxSize()); }

	NB_PROP_GET(foreColor)				{ return PushWxColor(v, self(v)->GetForegroundColour()); }
	NB_PROP_GET(backColor)				{ return PushWxColor(v, self(v)->GetBackgroundColour()); }

	NB_PROP_GET(clientSize)				{ return push(v, self(v)->GetClientSize()); }
	NB_PROP_GET(minClientSize)			{ return push(v, self(v)->GetMinClientSize()); }
	NB_PROP_GET(maxClientSize)			{ return push(v, self(v)->GetMaxClientSize()); }

	NB_PROP_GET(virtualSize)			{ return push(v, self(v)->GetVirtualSize()); }

	NB_PROP_GET(handle)					{ sq_pushinteger(v, (int)self(v)->GetHandle()); return 1; }

	NB_PROP_GET(parent)					{ return push(v, self(v)->GetParent()); }
	NB_PROP_GET(grandParent)			{ return push(v, self(v)->GetGrandParent()); }
	NB_PROP_GET(nextSibling)			{ type* o = self(v); return push(v, o->IsTopLevel() ? NULL : o->GetNextSibling()); }
	NB_PROP_GET(prevSibling)			{ type* o = self(v); return push(v, o->IsTopLevel() ? NULL : o->GetPrevSibling()); }

	NB_PROP_GET(visible)				{ return push(v, self(v)->IsShown()); }
	NB_PROP_GET(enabled)				{ return push(v, self(v)->IsEnabled()); }
	NB_PROP_GET(thisEnabled)			{ return push(v, self(v)->IsThisEnabled()); }
	NB_PROP_GET(shownOnScreen)			{ return push(v, self(v)->IsShownOnScreen()); }
	NB_PROP_GET(topLevel)				{ return push(v, self(v)->IsTopLevel()); }
	NB_PROP_GET(beingDeleted)			{ return push(v, self(v)->IsBeingDeleted()); }

	NB_PROP_GET(toolTip)				{ return push(v, self(v)->GetToolTipText()); }
	NB_PROP_GET(helpText)				{ return push(v, self(v)->GetHelpText()); }

	NB_PROP_GET(sizer)					{ return push(v, self(v)->GetSizer()); }
	NB_PROP_GET(containingSizer)		{ return push(v, self(v)->GetContainingSizer()); }

	NB_PROP_GET(windowStyle)			{ return push(v, (int)self(v)->GetWindowStyle()); }
	NB_PROP_GET(extraStyle)				{ return push(v, (int)self(v)->GetExtraStyle()); }

	NB_PROP_SET(name)					{ self(v)->SetName(getWxString(v, 2)); return 0; }
	NB_PROP_SET(label)					{ self(v)->SetLabel(getWxString(v, 2)); return 0; }
	NB_PROP_SET(id)						{ self(v)->SetId(getInt(v, 2)); return 0; }
	NB_PROP_SET(position)				{ self(v)->SetPosition(*get<wxPoint>(v, 2)); return 0; }

	NB_PROP_SET(size)					{ self(v)->SetSize(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(minSize)				{ self(v)->SetMinSize(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(maxSize)				{ self(v)->SetMaxSize(*get<wxSize>(v, 2)); return 0; }

	NB_PROP_SET(foreColor)				{ self(v)->SetForegroundColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(backColor)				{ self(v)->SetBackgroundColour(GetWxColor(v, 2)); return 0; }

	NB_PROP_SET(clientSize)				{ self(v)->SetClientSize(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(minClientSize)			{ self(v)->SetMinClientSize(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(maxClientSize)			{ self(v)->SetMaxClientSize(*get<wxSize>(v, 2)); return 0; }

	NB_PROP_SET(virtualSize)			{ self(v)->SetVirtualSize(*get<wxSize>(v, 2)); return 0; }

	NB_PROP_SET(parent)					{ self(v)->Reparent(opt<wxWindow>(v, 2, NULL)); return 0; }

	NB_PROP_SET(toolTip)				{ if (isNull(v, 2)) self(v)->UnsetToolTip(); else self(v)->SetToolTip(getWxString(v, 2)); return 0; }
	NB_PROP_SET(helpText)				{ self(v)->SetHelpText(getWxString(v, 2)); return 0; }

	NB_PROP_SET(sizer)					{ self(v)->SetSizer(opt<wxSizer>(v, 2, NULL)); return 0; }
	NB_PROP_SET(containingSizer)		{ self(v)->SetContainingSizer(opt<wxSizer>(v, 2, NULL)); return 0; }

	NB_PROP_SET(enabled)				{ self(v)->Enable(getBool(v, 2)); return 0; }
	NB_PROP_SET(visible)				{ self(v)->Show(getBool(v, 2)); return 0; }

	NB_PROP_SET(windowStyle)			{ self(v)->SetWindowStyle(getInt(v, 2)); return 0; }
	NB_PROP_SET(extraStyle)				{ self(v)->SetExtraStyle(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		wxWindow* window = new wxWindow(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			*opt<wxPoint>(v, 4, wxDefaultPosition),
			*opt<wxSize>(v, 5, wxDefaultSize),
			optInt(v, 6, 0),
			optWxString(v, 7, wxPanelNameStr));

		setSelf(v, window);
		return 0;
	}

	NB_FUNC(destroy)					{ return push(v, self(v)->Destroy()); }
	NB_FUNC(destroyChildren)			{ return push(v, self(v)->DestroyChildren()); }
	NB_FUNC(show)						{ return push(v, self(v)->Show(optBool(v, 2, true))); }
	NB_FUNC(hide)						{ return push(v, self(v)->Hide()); }

	NB_FUNC(hasFocus)					{ return push(v, self(v)->HasFocus()); }
	NB_FUNC(setFocus)					{ self(v)->SetFocus(); return 0; }

	NB_FUNC(freeze)						{ self(v)->Freeze(); return 0; }
	NB_FUNC(thaw)						{ self(v)->Thaw(); return 0; }

	NB_FUNC(popupMenu)					{ return push(v, self(v)->PopupMenu(get<wxMenu>(v, 2), *opt<wxPoint>(v, 3, wxDefaultPosition))); }
	NB_FUNC(getPopupMenuSelectionFromUser)	{ return push(v, self(v)->GetPopupMenuSelectionFromUser(*get<wxMenu>(v, 2), *opt<wxPoint>(v, 3, wxDefaultPosition))); }

	NB_FUNC(captureMouse)				{ self(v)->CaptureMouse(); return 0; }
	NB_FUNC(releaseMouse)				{ self(v)->ReleaseMouse(); return 0; }

	NB_FUNC(clientToScreen)				{ return push(v, self(v)->ClientToScreen(*get<wxPoint>(v, 2))); }
	NB_FUNC(screenToClient)				{ return push(v, self(v)->ScreenToClient(*get<wxPoint>(v, 2))); }

	NB_FUNC(setSizeHints)				{ self(v)->SetSizeHints(*get<wxSize>(v, 2), *opt<wxSize>(v, 3, wxDefaultSize), *opt<wxSize>(v, 4, wxDefaultSize)); return 0; }
	NB_FUNC(setSizerAndFit)				{ self(v)->SetSizerAndFit(get<wxSizer>(v, 2)); return 0; }
	NB_FUNC(layout)						{ self(v)->Layout(); return 0; }

	NB_FUNC(setWindowStyle)
	{
		type* o = self(v);
		BitSet flags = o->GetWindowStyle();
		flags.set(getInt(v, 2), optBool(v, 3, true));
		o->SetWindowStyle(flags);
		return 0;
	}

	NB_FUNC(setExtraStyle)
	{
		type* o = self(v);
		BitSet flags = o->GetExtraStyle();
		flags.set(getInt(v, 2), optBool(v, 3, true));
		o->SetExtraStyle(flags);
		return 0;
	}

	NB_FUNC(refresh)					{ self(v)->Refresh(optBool(v, 2, true), opt<wxRect>(v, 3, NULL)); return 0; }

	NB_FUNC(getTextExtent)				{ return push(v, self(v)->GetTextExtent(getWxString(v, 2))); }

	NB_FUNC(findWindow)
	{
		if (sq_gettype(v, 1) == OT_CLASS)
		{
			if (isString(v, 2))
				return push(v, wxWindow::FindWindowByName(getWxString(v, 2), NULL));
			else
				return push(v, wxWindow::FindWindowById(onlyInt(v, 2), NULL));
		}
		else
		{
			bool recursive = optBool(v, 3, true);
			if (recursive)
			{
				if (isString(v, 2))
					return push(v, wxWindow::FindWindowByName(getWxString(v, 2), self(v)));
				else
					return push(v, wxWindow::FindWindowById(onlyInt(v, 2), self(v)));
			}
			else
			{
				if (isString(v, 2))
					return push(v, self(v)->FindWindow(getWxString(v, 2)));
				else
					return push(v, self(v)->FindWindow(onlyInt(v, 2)));
			}
		}
	}

	NB_FUNC(getCapture)					{ return push(v, wxWindow::GetCapture()); }
	NB_FUNC(findFocus)					{ return push(v, wxWindow::FindFocus()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, TopLevelWindow, wxWindow);

class NB_WxTopLevelWindow : TNitClass<wxTopLevelWindow>
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
			FUNC_ENTRY_H(close,			"(force=false): bool // returns true if event handler honoured our request"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "ACTIVATE",		(int)wxEVT_ACTIVATE);
		newSlot(v, -1, "MOVE",			(int)wxEVT_MOVE);
		newSlot(v, -1, "MOVE_START",	(int)wxEVT_MOVE_START);
		newSlot(v, -1, "MOVE_END",		(int)wxEVT_MOVE_END);

		newSlot(v, -1, "CLOSE",			(int)wxEVT_CLOSE_WINDOW);
		newSlot(v, -1, "ICONIZE",		(int)wxEVT_ICONIZE);
		sq_poptop(v);
	}

	NB_FUNC(close)						{ return push(v, self(v)->Close(optBool(v, 2, false))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Frame, wxTopLevelWindow);

class NB_WxFrame : TNitClass<wxFrame>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(menuBar),
			PROP_ENTRY	(statusBar),
			PROP_ENTRY	(statusBarPane),
			PROP_ENTRY	(toolBar),
			PROP_ENTRY_R(clientAreaOrigin),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: Window=null, id=wx.ID.ANY, title=\"\", pos=null, size=null, style=STYLE.DEFAULT, name=null"),
			FUNC_ENTRY_H(createStatusBar,	"(number=1, style=STB.DEFAULT, id=0, name=null): StatusBar"),
			FUNC_ENTRY_H(createToolBar,		"(style=BORDER.NONE|TB.HORIZONTAL, id=ID.ANY, name=null): ToolBar"),
			FUNC_ENTRY_H(setStatusText,		"(text: string, number=0)"),
			FUNC_ENTRY_H(processCommand,	"(id: ID): bool"),
			FUNC_ENTRY_H(center,			"(dir=DIR.BOTH)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",			(int)wxDEFAULT_FRAME_STYLE);
		newSlot(v, -1, "ICONIZE",			(int)wxICONIZE);
		newSlot(v, -1, "CAPTION",			(int)wxCAPTION);
		newSlot(v, -1, "MINIMIZE",			(int)wxMINIMIZE);
		newSlot(v, -1, "MINIMIZE_BOX",		(int)wxMINIMIZE_BOX);
		newSlot(v, -1, "MAXIMIZE",			(int)wxMAXIMIZE);
		newSlot(v, -1, "MAXIMIZE_BOX",		(int)wxMAXIMIZE_BOX);
		newSlot(v, -1, "CLOSE_BOX",			(int)wxCLOSE_BOX);
		newSlot(v, -1, "STAY_ON_TOP",		(int)wxSTAY_ON_TOP);
		newSlot(v, -1, "RESIZE_BORDER",		(int)wxRESIZE_BORDER);
		newSlot(v, -1, "TOOL_WINDOW",		(int)wxFRAME_TOOL_WINDOW);
		newSlot(v, -1, "NO_TASKBAR",		(int)wxFRAME_NO_TASKBAR);
		newSlot(v, -1, "FLOAT_ON_PARENT",	(int)wxFRAME_FLOAT_ON_PARENT);
		newSlot(v, -1, "SHAPED",			(int)wxFRAME_SHAPED);
		newSlot(v, -1, "EX_CONTEXTHELP",	(int)wxFRAME_EX_CONTEXTHELP);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "MENU",				(int)wxEVT_COMMAND_MENU_SELECTED);
		newSlot(v, -1, "MENU_OPEN",			(int)wxEVT_MENU_OPEN);
		newSlot(v, -1, "MENU_CLOSE",		(int)wxEVT_MENU_CLOSE);
		newSlot(v, -1, "MENU_HIGHLIGHT",	(int)wxEVT_MENU_HIGHLIGHT);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxFrame* frame = new wxFrame(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optWxString(v, 4, wxEmptyString),
			*opt<wxPoint>(v, 5, wxDefaultPosition),
			*opt<wxSize>(v, 6, wxDefaultSize),
			optInt(v, 7, wxDEFAULT_FRAME_STYLE),
			optWxString(v, 8, wxFrameNameStr));

		sq_setinstanceup(v, 1, frame);
		return 0;
	}

	NB_PROP_GET(menuBar)				{ return push(v, self(v)->GetMenuBar()); }
	NB_PROP_GET(statusBar)				{ return push(v, self(v)->GetStatusBar()); }
	NB_PROP_GET(statusBarPane)			{ return push(v, self(v)->GetStatusBarPane()); }
	NB_PROP_GET(toolBar)				{ return push(v, self(v)->GetToolBar()); }
	NB_PROP_GET(clientAreaOrigin)		{ return push(v, self(v)->GetClientAreaOrigin()); }

	NB_PROP_SET(menuBar)				{ self(v)->SetMenuBar(opt<wxMenuBar>(v, 2, NULL)); return 0; }
	NB_PROP_SET(statusBar)				{ self(v)->SetStatusBar(opt<wxStatusBar>(v, 2, NULL)); return 0; }
	NB_PROP_SET(statusBarPane)			{ self(v)->SetStatusBarPane(getInt(v, 2)); return 0; }
	NB_PROP_SET(toolBar)				{ self(v)->SetToolBar(opt<wxToolBar>(v, 2, NULL)); return 0; }

	NB_FUNC(createStatusBar)			{ return push(v, self(v)->CreateStatusBar(optInt(v, 2, 1), optInt(v, 3, wxSTB_DEFAULT_STYLE), optInt(v, 4, 0), optWxString(v, 5, wxStatusLineNameStr))); }
	NB_FUNC(createToolBar)				{ return push(v, self(v)->CreateToolBar(optInt(v, 2, wxBORDER_NONE | wxTB_HORIZONTAL), optInt(v, 3, wxID_ANY), optWxString(v, 4, wxToolBarNameStr))); }
	NB_FUNC(setStatusText)				{ self(v)->SetStatusText(getWxString(v, 2), optInt(v, 3, 0)); return 0; }
	NB_FUNC(processCommand)				{ return push(v, self(v)->ProcessCommand(getInt(v, 2))); }
	NB_FUNC(center)						{ self(v)->Center(optInt(v, 2, wxBOTH)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Menu, wxEvtHandler);

class NB_WxMenu : TNitClass<wxMenu>
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
			CONS_ENTRY_H(					"(style=0)"
			"\n"							"(title, style=0)"
			"\n"							"(menuItems: [[]]) // [ [ ID, \"&Open...\\tCtrl-O\", \"Open a document\" ] ]"),
			FUNC_ENTRY_H(append,			"(id: int, item=\"\", help=\"\", itemKind=ITEM_NORMAL): MenuItem\n"
											"(item: MenuItem): MenuItem\n"
											"(id: int, item: string, sub: Menu, help=\"\"): MenuItem"),
			FUNC_ENTRY_H(appendCheckItem,	"(id: int, item: string, help=\"\"): MenuItem"),
			FUNC_ENTRY_H(appendRadioItem,	"(id: int, item: string, help=\"\"): MenuItem"),
			FUNC_ENTRY_H(appendSeparator,	"(): MenuItem"),
			FUNC_ENTRY_H(appendSubMenu,		"(sub: Menu, text: string, help=\"\"): MenuItem"),

			FUNC_ENTRY_H(break,				"()"),

			FUNC_ENTRY_H(check,				"(id: int, check: bool)"),
			FUNC_ENTRY_H(enable,			"(id: int, enable: bool)"),
			FUNC_ENTRY_H(isChecked,			"(id: int): bool"),
			FUNC_ENTRY_H(isEnabled,			"(id: int): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "MENU",			(int)wxEVT_COMMAND_MENU_SELECTED);
		sq_poptop(v);

		addStaticTable(v, "ITEM");
		newSlot(v, -1, "SEPARATOR",		(int)wxITEM_SEPARATOR);
		newSlot(v, -1, "NORMAL",		(int)wxITEM_NORMAL);
		newSlot(v, -1, "CHECK",			(int)wxITEM_CHECK);
		newSlot(v, -1, "RADIO",			(int)wxITEM_RADIO);
		newSlot(v, -1, "DROPDOWN",		(int)wxITEM_DROPDOWN);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxMenu* menu;
		if (isNone(v, 2))
			menu = new wxMenu();
		else if (isInt(v, 2))
			menu = new wxMenu(getInt(v, 2));
		else if (sq_gettype(v, 2) == OT_ARRAY)
		{
			menu = new wxMenu();

			for (NitIterator mi(v, 2); mi.hasNext(); mi.next())
			{
				if (sq_gettype(v, mi.valueIndex()) == OT_NULL)
				{
					menu->AppendSeparator();
				}
				else
				{
					NitIterator vi(v, mi.valueIndex());

					int id;
					wxString text;
					wxString help;
					wxItemKind kind = wxITEM_NORMAL;

					if (vi.hasNext())	{ id = getInt(v, vi.valueIndex()); vi.next(); }
					if (vi.hasNext())	{ text = getWxString(v, vi.valueIndex()); vi.next(); }
					if (vi.hasNext())	{ help = getWxString(v, vi.valueIndex()); vi.next(); }
					if (vi.hasNext())	{ kind = (wxItemKind)getInt(v, vi.valueIndex()); vi.next(); }

					menu->Append(id, text, help, kind);
				}
			}
		}
		else menu = new wxMenu(getWxString(v, 2), optInt(v, 3, 0));

		sq_setinstanceup(v, 1, menu);
		return 0;
	}

	NB_FUNC(append)
	{
		if (is<wxMenuItem>(v, 2))
			return push(v, self(v)->Append(get<wxMenuItem>(v, 2)));
		if (is<wxMenu>(v, 4))
			return push(v, self(v)->Append(getInt(v, 2), getWxString(v, 3), get<wxMenu>(v, 4), optWxString(v, 5, "")));

		return push(v, self(v)->Append(getInt(v, 2), optWxString(v, 3, ""), optWxString(v, 4, ""), (wxItemKind)optInt(v, 5, wxITEM_NORMAL)));
	}

	NB_FUNC(appendCheckItem)			{ return push(v, self(v)->AppendCheckItem(getInt(v, 2), getWxString(v, 3), optWxString(v, 4, ""))); }
	NB_FUNC(appendRadioItem)			{ return push(v, self(v)->AppendRadioItem(getInt(v, 2), getWxString(v, 3), optWxString(v, 4, ""))); }
	NB_FUNC(appendSeparator)			{ return push(v, self(v)->AppendSeparator()); }
	NB_FUNC(appendSubMenu)				{ return push(v, self(v)->AppendSubMenu(get<wxMenu>(v, 2), getWxString(v, 3), optWxString(v, 4, ""))); }

	NB_FUNC(break)						{ self(v)->Break(); return 0; }

	NB_FUNC(check)						{ self(v)->Check(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(enable)						{ self(v)->Enable(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(isChecked)					{ return push(v, self(v)->IsChecked(getInt(v, 2))); return 0; }
	NB_FUNC(isEnabled)					{ return push(v, self(v)->IsEnabled(getInt(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, MenuItem, wxObject);

class NB_WxMenuItem : TNitClass<wxMenuItem>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(itemLabel),
			PROP_ENTRY_R(menu),
			PROP_ENTRY	(id),
			PROP_ENTRY	(bitmap),
			PROP_ENTRY	(enabled),
			PROP_ENTRY	(checked),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: Menu=null, id=ID.SEPARATOR, name=\"\", help=\"\", kind=wx.ITEM.NORMAL, subMenu: Menu=null)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxMenuItem* item = new wxMenuItem(
			opt<wxMenu>(v, 2, NULL),
			optInt(v, 3, wxID_SEPARATOR),
			optWxString(v, 4, ""),
			optWxString(v, 5, ""),
			(wxItemKind)optInt(v, 6, wxITEM_NORMAL),
			opt<wxMenu>(v, 7, NULL));

		sq_setinstanceup(v, 1, item);
		return 0;
	}

	NB_PROP_GET(itemLabel)				{ return push(v, self(v)->GetItemLabel()); }
	NB_PROP_GET(menu)					{ return push(v, self(v)->GetMenu()); }
	NB_PROP_GET(id)						{ return push(v, self(v)->GetId()); }
	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
	NB_PROP_GET(enabled)				{ return push(v, self(v)->IsEnabled()); }
	NB_PROP_GET(checked)				{ return push(v, self(v)->IsChecked()); }

	NB_PROP_SET(itemLabel)				{ self(v)->SetItemLabel(getWxString(v, 2)); return 0; }
	NB_PROP_SET(id)						{ self(v)->SetId(getInt(v, 2)); return 0; }
	NB_PROP_SET(bitmap)					{ self(v)->SetBitmap(*opt<wxBitmap>(v, 2, wxNullBitmap)); return 0; }
	NB_PROP_SET(enabled)				{ self(v)->Enable(getBool(v, 2)); return 0; }
	NB_PROP_SET(checked)				{ self(v)->Check(getBool(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, MenuBar, wxWindow);

class NB_WxMenuBar : TNitClass<wxMenuBar>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(menuCount),
			PROP_ENTRY_R(frame),
			PROP_ENTRY_R(attached),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(style=0)"),
			FUNC_ENTRY_H(append,		"(menu: wx.Menu, title: string): bool"),
			FUNC_ENTRY_H(insert,		"(index: int, menu: wx.Menu, title: string): bool"),
			FUNC_ENTRY_H(check,			"(id: int, checked: bool)"),
			FUNC_ENTRY_H(enable,		"(id: int, enabled: bool)"),
			FUNC_ENTRY_H(isChecked,		"(id: int): bool"),
			FUNC_ENTRY_H(isEnabled,		"(id: int): bool"),
			FUNC_ENTRY_H(findItem,		"(id: int): wx.MenuItem"),
			FUNC_ENTRY_H(findMenu,		"(title: string): int // returns index of menu"),
			FUNC_ENTRY_H(findMenuItem,	"(menu: string, item: string): int // returns ID of menu item"),
			FUNC_ENTRY_H(getMenu,		"(menuIndex: int): wx.Menu"),
			FUNC_ENTRY_H(remove,		"(index: int): wx.Menu // you should Destroy() returned menu when not needed"),
			FUNC_ENTRY_H(replace,		"(index: int, newMenu: wx.Menu, title: string): wx.Menu // you should Destroy() returned menu when not needed"),
			FUNC_ENTRY_H(getHelpString,	"(id: int): string"),
			FUNC_ENTRY_H(getLabel,		"(id: int): string"),
			FUNC_ENTRY_H(getMenuLabel,	"(index: int): string"),
			FUNC_ENTRY_H(setHelpString,	"(id: int, help: string)"),
			FUNC_ENTRY_H(setLabel,		"(id: int, label: string)"),
			FUNC_ENTRY_H(setMenuLabel,	"(index: int, label: string)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(menuCount)				{ return push(v, self(v)->GetMenuCount()); }
	NB_PROP_GET(frame)					{ return push(v, self(v)->GetFrame()); }
	NB_PROP_GET(attached)				{ return push(v, self(v)->IsAttached()); }

	NB_CONS()
	{
		wxMenuBar* bar = new wxMenuBar(optInt(v, 2, 0));

		sq_setinstanceup(v, 1, bar);
		return 0;
	}

	NB_FUNC(append)						{ return push(v, self(v)->Append(get<wxMenu>(v, 2), getWxString(v, 3))); }
	NB_FUNC(insert)						{ return push(v, self(v)->Insert(getInt(v, 2), get<wxMenu>(v, 3), getWxString(v, 4))); }
	NB_FUNC(check)						{ self(v)->Check(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(enable)						{ self(v)->Enable(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(isChecked)					{ return push(v, self(v)->IsChecked(getInt(v, 2))); }
	NB_FUNC(isEnabled)					{ return push(v, self(v)->IsEnabled(getInt(v, 2))); }
	NB_FUNC(findItem)					{ return push(v, self(v)->FindItem(getInt(v, 2))); }
	NB_FUNC(findMenu)					{ return push(v, self(v)->FindMenu(getWxString(v, 2))); }
	NB_FUNC(findMenuItem)				{ return push(v, self(v)->FindMenuItem(getWxString(v, 2), getWxString(v, 3))); }
	NB_FUNC(getMenu)					{ return push(v, self(v)->GetMenu(getInt(v, 2))); }
	NB_FUNC(remove)						{ return push(v, self(v)->Remove(getInt(v, 2))); }
	NB_FUNC(replace)					{ return push(v, self(v)->Replace(getInt(v, 2), get<wxMenu>(v, 3), getWxString(v, 4))); }
	NB_FUNC(getHelpString)				{ return push(v, self(v)->GetHelpString(getInt(v, 2))); }
	NB_FUNC(getLabel)					{ return push(v, self(v)->GetLabel(getInt(v, 2))); }
	NB_FUNC(getMenuLabel)				{ return push(v, self(v)->GetMenuLabel(getInt(v, 2))); }
	NB_FUNC(setHelpString)				{ self(v)->SetHelpString(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setLabel)					{ self(v)->SetLabel(getInt(v, 2), getWxString(v, 3)); return 0; }
	NB_FUNC(setMenuLabel)				{ self(v)->SetMenuLabel(getInt(v, 2), getWxString(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StatusBar, wxWindow);

class NB_WxStatusBar : TNitClass<wxStatusBar>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(fieldsCount),
			PROP_ENTRY	(statusText),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: Window=null, id=ID.ANY, style=STYLE.DEFAULT, name=null)"),
			FUNC_ENTRY_H(getStatusText,		"(field=0): string"),
			FUNC_ENTRY_H(setStatusText,		"(string, field=0)"),
			FUNC_ENTRY_H(popStatusText,		"(field=0)"),
			FUNC_ENTRY_H(pushStatusText,	"(string, field=0"),
			FUNC_ENTRY_H(setMinHeight,		"(height: int)"),
			FUNC_ENTRY_H(getStatusWidth,	"(field: int): int"),
			FUNC_ENTRY_H(setStatusWidths,	"(widths: int[])"),
			FUNC_ENTRY_H(getStatusStyle,	"(field: int): int"),
			FUNC_ENTRY_H(setStatusStyles,	"(styles: int[])"),
			FUNC_ENTRY_H(getBorders,		"(): Size"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "SIZEGRIP",			wxSTB_SIZEGRIP);
		newSlot(v, -1, "SHOW_TIPS",			wxSTB_SHOW_TIPS);
		newSlot(v, -1, "ELLIPSIZE_START",	wxSTB_ELLIPSIZE_START);
		newSlot(v, -1, "ELLIPSIZE_MIDDLE",	wxSTB_ELLIPSIZE_MIDDLE);
		newSlot(v, -1, "ELLIPSIZE_END",		wxSTB_ELLIPSIZE_END);
		newSlot(v, -1, "DEFAULT",			wxSTB_DEFAULT_STYLE);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxWindow* wnd = new wxStatusBar(
			opt<wxWindow>(v, 2, NULL),
			optInt(v, 3, wxID_ANY),
			optInt(v, 4, wxSTB_DEFAULT_STYLE),
			optWxString(v, 5, wxStatusBarNameStr)
			);

		sq_setinstanceup(v, 1, wnd);
		return 0;
	}

	NB_PROP_GET(fieldsCount)			{ return push(v, self(v)->GetFieldsCount()); }
	NB_PROP_GET(statusText)				{ return push(v, self(v)->GetStatusText()); }

	NB_PROP_SET(fieldsCount)			{ self(v)->SetFieldsCount(getInt(v, 2)); return 0; }
	NB_PROP_SET(statusText)				{ self(v)->SetStatusText(getWxString(v, 2)); return 0; }

	NB_FUNC(getStatusText)				{ self(v)->GetStatusText(optInt(v, 2, 0)); return 0; }
	NB_FUNC(setStatusText)				{ self(v)->SetStatusText(getWxString(v, 2), optInt(v, 3, 0)); return 0; }
	NB_FUNC(pushStatusText)				{ self(v)->PushStatusText(getWxString(v, 2), optInt(v, 3, 0)); return 0; }
	NB_FUNC(popStatusText)				{ self(v)->PopStatusText(optInt(v, 2, 0)); return 0; }
	NB_FUNC(setMinHeight)				{ self(v)->SetMinHeight(getInt(v, 2)); return 0; }
	NB_FUNC(getStatusWidth)				{ return push(v, self(v)->GetStatusWidth(getInt(v, 2))); }
	NB_FUNC(getStatusStyle)				{ return push(v, self(v)->GetStatusStyle(getInt(v, 2))); }
	NB_FUNC(getBorders)					{ return push(v, self(v)->GetBorders()); }

	NB_FUNC(setStatusWidths)
	{
		if (sq_gettype(v, 2) != OT_ARRAY)
			return sq_throwerror(v, "invalid args");

		int n = sq_getsize(v, 2);

		if (n != self(v)->GetFieldsCount())
			return sq_throwerror(v, "element count mismatch");

		vector<int>::type widths;
		widths.resize(n);
		for (int i=0; i<n; ++i)
		{
			sq_pushinteger(v, i);
			sq_get(v, 2);
			widths[i] = getInt(v, -1);
			sq_pop(v, 1);
		}

		self(v)->SetStatusWidths(n, &*widths.begin());
		return 0;
	}

	NB_FUNC(setStatusStyles)
	{
		if (sq_gettype(v, 2) != OT_ARRAY)
			return sq_throwerror(v, "invalid args");

		int n = sq_getsize(v, 2);

		if (n != self(v)->GetFieldsCount())
			return sq_throwerror(v, "element count mismatch");

		vector<int>::type styles;
		styles.resize(n);
		for (int i=0; i<n; ++i)
		{
			sq_pushinteger(v, i);
			sq_get(v, 2);
			styles[i] = getInt(v, -1);
			sq_pop(v, 1);
		}

		self(v)->SetStatusWidths(n, &*styles.begin());
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, SizerFlags, NULL);

class NB_WxSizerFlags : TNitClass<wxSizerFlags>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(flags),
			PROP_ENTRY_R(proportionValue),
			PROP_ENTRY_R(borderInPixels),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(							"(proportion=0)"),

			FUNC_ENTRY_H(align,						"(align: ALIGN): this"),
			FUNC_ENTRY_H(border,					"(dir: DIR, borderInPixels: int): this\n"
													"(dir=DIR.ALL): this"),
			FUNC_ENTRY_H(bottom,					"(): this"),
			FUNC_ENTRY_H(center,					"(): this"),
			FUNC_ENTRY_H(doubleBorder,				"(dir=DIR.ALL): this"),
			FUNC_ENTRY_H(doubleHorzBorder,			"(): this"),
			FUNC_ENTRY_H(expand,					"(): this"),
			FUNC_ENTRY_H(fixedMinSize,				"(): this"),
			FUNC_ENTRY_H(reserveSpaceEvenIfHidden,	"(): this"),
			FUNC_ENTRY_H(left,						"(): this"),
			FUNC_ENTRY_H(proportion,				"(proportion: int): this"),
			FUNC_ENTRY_H(right,						"(): this"),
			FUNC_ENTRY_H(shaped,					"(): this"),
			FUNC_ENTRY_H(top,						"(): this"),
			FUNC_ENTRY_H(tripleBorder,				"(dir=DIR.ALL): this"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(flags)					{ return push(v, self(v)->GetFlags()); }
	NB_PROP_GET(proportionValue)		{ return push(v, self(v)->GetProportion()); }
	NB_PROP_GET(borderInPixels)			{ return push(v, self(v)->GetBorderInPixels()); }	

	NB_CONS()
	{
		new (self(v)) wxSizerFlags(optInt(v, 2, 0));
		return 0;
	}

	NB_FUNC(align)						{ self(v)->Align(getInt(v, 2)); sq_push(v, 1); return 1; }
	NB_FUNC(border)						{ self(v)->Border(optInt(v, 2, wxALL), optInt(v, 3, wxSizerFlags::GetDefaultBorder())); sq_push(v, 1); return 1; }
	NB_FUNC(bottom)						{ self(v)->Bottom(); sq_push(v, 1); return 1; }
	NB_FUNC(center)						{ self(v)->Center(); sq_push(v, 1); return 1; }
	NB_FUNC(doubleBorder)				{ self(v)->DoubleBorder(optInt(v, 2, wxALL)); sq_push(v, 1); return 1; }
	NB_FUNC(doubleHorzBorder)			{ self(v)->DoubleHorzBorder(); sq_push(v, 1); return 1; }
	NB_FUNC(expand)						{ self(v)->Expand(); sq_push(v, 1); return 1; }
	NB_FUNC(fixedMinSize)				{ self(v)->FixedMinSize(); sq_push(v, 1); return 1; }
	NB_FUNC(reserveSpaceEvenIfHidden)	{ self(v)->ReserveSpaceEvenIfHidden(); sq_push(v, 1); return 1; }
	NB_FUNC(left)						{ self(v)->Left(); sq_push(v, 1); return 1; }
	NB_FUNC(proportion)					{ self(v)->Proportion(getInt(v, 2)); sq_push(v, 1); return 1; }
	NB_FUNC(right)						{ self(v)->Right(); sq_push(v, 1); return 1; }
	NB_FUNC(shaped)						{ self(v)->Shaped(); sq_push(v, 1); return 1; }
	NB_FUNC(top)						{ self(v)->Top(); sq_push(v, 1); return 1; }
	NB_FUNC(tripleBorder)				{ self(v)->TripleBorder(optInt(v, 2, wxALL)); sq_push(v, 1); return 1; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, SizerItem, wxObject);

class NB_WxSizerItem : TNitClass<wxSizerItem>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(sizer),
			PROP_ENTRY	(window),
			PROP_ENTRY_R(spacer),
			PROP_ENTRY	(border),
			PROP_ENTRY	(flag),
			PROP_ENTRY	(id),
			PROP_ENTRY	(minSize),
			PROP_ENTRY_R(position),
			PROP_ENTRY	(proportion),
			PROP_ENTRY	(ratio),
			PROP_ENTRY_R(rect),
			PROP_ENTRY_R(size),
			PROP_ENTRY	(visible),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(isWindow,		"(): bool"),
			FUNC_ENTRY_H(isSizer,		"(): bool"),
			FUNC_ENTRY_H(isSpacer,		"(): bool"),

			FUNC_ENTRY_H(calcMin,		"(): Size"),
			FUNC_ENTRY_H(setDimension,	"(pos: Point, sz: Size)"),
			FUNC_ENTRY_H(setInitSize,	"(x, y: int)"),
			FUNC_ENTRY_H(setRatio,		"(width, height: int)"),
			FUNC_ENTRY_H(deleteWindows,	"()"),
			FUNC_ENTRY_H(detachSizer,	"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(sizer)					{ return push(v, self(v)->GetSizer()); }
	NB_PROP_GET(window)					{ return push(v, self(v)->GetWindow()); }
	NB_PROP_GET(spacer)					{ return push(v, self(v)->GetSpacer()); }
	NB_PROP_GET(border)					{ return push(v, self(v)->GetBorder()); }
	NB_PROP_GET(flag)					{ return push(v, self(v)->GetFlag()); }
	NB_PROP_GET(id)						{ return push(v, self(v)->GetId()); }
	NB_PROP_GET(minSize)				{ return push(v, self(v)->GetMinSize()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
	NB_PROP_GET(proportion)				{ return push(v, self(v)->GetProportion()); }
	NB_PROP_GET(ratio)					{ return push(v, self(v)->GetRatio()); }
	NB_PROP_GET(rect)					{ return push(v, self(v)->GetRect()); }
	NB_PROP_GET(size)					{ return push(v, self(v)->GetSize()); }
	NB_PROP_GET(visible)				{ return push(v, self(v)->IsShown()); }

	NB_PROP_SET(sizer)					{ self(v)->AssignSizer(opt<wxSizer>(v, 2, NULL)); return 0; }
	NB_PROP_SET(window)					{ self(v)->AssignWindow(opt<wxWindow>(v, 2, NULL)); return 0; }
	NB_PROP_SET(border)					{ self(v)->SetBorder(getInt(v, 2)); return 0; }
	NB_PROP_SET(flag)					{ self(v)->SetFlag(getInt(v, 2)); return 0; }
	NB_PROP_SET(id)						{ self(v)->SetId(getInt(v, 2)); return 0; }
	NB_PROP_SET(minSize)				{ self(v)->SetMinSize(*get<wxSize>(v, 2)); return 0; }
	NB_PROP_SET(proportion)				{ self(v)->SetProportion(getInt(v, 2)); return 0; }
	NB_PROP_SET(ratio)					{ self(v)->SetRatio(getFloat(v, 2)); return 0; }
	NB_PROP_SET(visible)				{ self(v)->Show(getBool(v, 2)); return 0; }

	NB_FUNC(isWindow)					{ return push(v, self(v)->IsWindow()); }
	NB_FUNC(isSizer)					{ return push(v, self(v)->IsSizer()); }
	NB_FUNC(isSpacer)					{ return push(v, self(v)->IsSpacer()); }
	NB_FUNC(calcMin)					{ return push(v, self(v)->CalcMin()); }
	NB_FUNC(setDimension)				{ self(v)->SetDimension(*get<wxPoint>(v, 2), *get<wxSize>(v, 3)); return 0; }
	NB_FUNC(setInitSize)				{ self(v)->SetInitSize(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(setRatio)					{ self(v)->SetRatio(getInt(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(deleteWindows)				{ self(v)->DeleteWindows(); return 0; }
	NB_FUNC(detachSizer)				{ self(v)->DetachSizer(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Sizer, wxObject);

class NB_WxSizer : TNitClass<wxSizer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY_R(itemCount),
			PROP_ENTRY	(minSize),
			PROP_ENTRY_R(position),
			PROP_ENTRY_R(size),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(clear,				"(deleteWindows=false)"),
			FUNC_ENTRY_H(fit,				"(window)"),
			FUNC_ENTRY_H(fitInside,			"(window)"),
			FUNC_ENTRY_H(layout,			"()"),

			FUNC_ENTRY_H(add,				"(window: Window, flags: SizerFlags): SizerItem\n"
											"(window: Window, proportion=0, flag=0, border=0): SizerItem\n"
											"(sizer: Sizer, flags: SizerFlags): SizerItem\n"
											"(sizer: Sizer, proportion=0, flag=0, border=0): SizerItem\n"
											"(width, height: int, proportion=0, flag=0, border=0): SizerItem"),
			FUNC_ENTRY_H(addSpacer,			"(size: int): SizerItem"),
			FUNC_ENTRY_H(addStretchSpacer,	"(prop=1): SizerItem"),
			FUNC_ENTRY_H(detach,			"(wxWindow: Window): bool\n"
											"(sizer: Sizer): bool\n"
											"(index: int): bool"),
			FUNC_ENTRY_H(replace,			"(old, new: Window, recursive=false): bool\n"
											"(old, new: Sizer, recursive=false): bool\n"
											"(index: int, new: SizerItem): bool"),

			FUNC_ENTRY_H(show,				"(window, show=true, recursive=false): bool\n"
											"(sizer, show=true, recursive=false): bool\n"
											"(index, show=true): bool"),

			FUNC_ENTRY_H(getItem,			"(window, recursive=false): SizerItem\n"
											"(sizer, recursive=false): SizerItem\n"
											"(index): SizerItem"),

			FUNC_ENTRY_H(getItemById,		"(id: int, recursive=false)"),
			FUNC_ENTRY_H(setSizeHints,		"(window: wx.Window)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(itemCount)				{ return push(v, self(v)->GetItemCount()); }
	NB_PROP_GET(minSize)				{ return push(v, self(v)->GetMinSize()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->GetPosition()); }
	NB_PROP_GET(size)					{ return push(v, self(v)->GetSize()); }

	NB_PROP_SET(minSize)				{ self(v)->SetMinSize(*get<wxSize>(v, 2)); return 0; }

	NB_FUNC(clear)						{ self(v)->Clear(optBool(v, 2, false)); return 0; }
	NB_FUNC(fit)						{ self(v)->Fit(get<wxWindow>(v, 2)); return 0; }
	NB_FUNC(fitInside)					{ self(v)->Fit(get<wxWindow>(v, 2)); return 0; }
	NB_FUNC(layout)						{ self(v)->Layout(); return 0; }

	NB_FUNC(add)
	{
		if (is<wxWindow>(v, 2))
		{
			if (isNone(v, 3) || isInt(v, 3))
				return push(v, self(v)->Add(get<wxWindow>(v, 2), optInt(v, 3, 0), optInt(v, 4, 0), optInt(v, 5, 0)));
			else
				return push(v, self(v)->Add(get<wxWindow>(v, 2), *get<wxSizerFlags>(v, 3)));
		}
		if (is<wxSizer>(v, 2))
		{
			if (isNone(v, 3) || isInt(v, 3))
				return push(v, self(v)->Add(get<wxSizer>(v, 2), optInt(v, 3, 0), optInt(v, 4, 0), optInt(v, 5, 0)));
			else
				return push(v, self(v)->Add(get<wxSizer>(v, 2), *get<wxSizerFlags>(v, 3)));
		}

		if (!isNone(v, 4) && is<wxSizerFlags>(v, 4))
			return push(v, self(v)->Add(getInt(v, 2), getInt(v, 3), *get<wxSizerFlags>(v, 4)));
		else
			return push(v, self(v)->Add(getInt(v, 2), getInt(v, 3), optInt(v, 4, 0), optInt(v, 5, 0), optInt(v, 6, 0)));
	}

	NB_FUNC(addSpacer)
	{
		return push(v, self(v)->AddSpacer(getInt(v, 2))); 
	}

	NB_FUNC(addStretchSpacer)
	{
		return push(v, self(v)->AddStretchSpacer(optInt(v, 2, 1)));
	}

	NB_FUNC(detach)
	{
		if (is<wxWindow>(v, 2))
			return push(v, self(v)->Detach(get<wxWindow>(v, 2)));
		else if (is<wxSizer>(v, 2))
			return push(v, self(v)->Detach(get<wxSizer>(v, 2)));
		else
			return push(v, self(v)->Detach(getInt(v, 2)));
	}

	NB_FUNC(replace)
	{
		if (is<wxWindow>(v, 2))
			return push(v, self(v)->Replace(get<wxWindow>(v, 2), get<wxWindow>(v, 3), optBool(v, 4, false)));
		else if (is<wxSizer>(v, 2))
			return push(v, self(v)->Replace(get<wxSizer>(v, 2), get<wxSizer>(v, 3), optBool(v, 4, false)));
		else
			return push(v, self(v)->Replace(getInt(v, 2), get<wxSizerItem>(v, 3)));
	}

	NB_FUNC(show)
	{
		if (is<wxWindow>(v, 2))
			return push(v, self(v)->Show(get<wxWindow>(v, 2), optBool(v, 3, true), optBool(v, 4, false)));
		else if (is<wxSizer>(v, 2))
			return push(v, self(v)->Show(get<wxSizer>(v, 2), optBool(v, 3, true), optBool(v, 4, false)));
		else
			return push(v, self(v)->Show(getInt(v, 2), optBool(v, 3, true)));
	}

	NB_FUNC(getItem)
	{
		if (is<wxWindow>(v, 2))
			return push(v, self(v)->GetItem(get<wxWindow>(v, 2), optBool(v, 3, false)));
		else if (is<wxSizer>(v, 2))
			return push(v, self(v)->GetItem(get<wxSizer>(v, 2), optBool(v, 3, false)));
		else
			return push(v, self(v)->GetItem(getInt(v, 2)));
	}

	NB_FUNC(getItemById)
	{
		return push(v, self(v)->GetItemById(getInt(v, 2), optBool(v, 3, false)));
	}

	NB_FUNC(setSizeHints)				{ self(v)->SetSizeHints(get<wxWindow>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, BoxSizer, wxSizer);

class NB_WxBoxSizer : TNitClass<wxBoxSizer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY_R(orientation),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(orient=ORIENT.VERTICAL)"),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "VERTICAL",		(int)wxVERTICAL);
		addStatic(v, "HORIZONTAL",		(int)wxHORIZONTAL);
	}

	NB_CONS()
	{
		sq_setinstanceup(v, 1, new wxBoxSizer(optInt(v, 2, wxVERTICAL)));
		return 0;
	}

	NB_PROP_GET(orientation)			{ return push(v, self(v)->GetOrientation()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StaticBoxSizer, wxBoxSizer);

class NB_WxStaticBoxSizer : TNitClass<wxStaticBoxSizer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY_R(staticBox),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(orient=ORIENT.VERTICAL, parent: Window, label=\"\")\n"
										"(box: StaticBox, orient=ORIENT.VERTICAL)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (is<wxStaticBox>(v, 2))
			sq_setinstanceup(v, 1, new wxStaticBoxSizer(get<wxStaticBox>(v, 2), optInt(v, 3, wxVERTICAL)));
		else
			// TODO: Check if it's allowed when parent is NULL
			sq_setinstanceup(v, 1, new wxStaticBoxSizer(optInt(v, 2, wxVERTICAL), get<wxWindow>(v, 3), optWxString(v, 3, "")));
		return 0;
	}

	NB_PROP_GET(staticBox)				{ return push(v, self(v)->GetStaticBox()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, StdDialogButtonSizer, wxSizer);

class NB_WxStdDialogButtonSizer : TNitClass<wxStdDialogButtonSizer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(affirmativeButton),
			PROP_ENTRY	(negativeButton),
			PROP_ENTRY	(cancelButton),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY_H(addButton,		"(button: Button)"),
			FUNC_ENTRY_H(realize,		"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ sq_setinstanceup(v, 1, new wxStdDialogButtonSizer()); return 0; }

	NB_PROP_GET(affirmativeButton)		{ return push(v, self(v)->GetAffirmativeButton()); }
	NB_PROP_GET(negativeButton)			{ return push(v, self(v)->GetNegativeButton()); }
	NB_PROP_GET(cancelButton)			{ return push(v, self(v)->GetCancelButton()); }

	NB_PROP_SET(affirmativeButton)		{ self(v)->SetAffirmativeButton(get<wxButton>(v, 2)); return 0; }
	NB_PROP_SET(negativeButton)			{ self(v)->SetNegativeButton(get<wxButton>(v, 2)); return 0; }
	NB_PROP_SET(cancelButton)			{ self(v)->SetCancelButton(get<wxButton>(v, 2)); return 0; }

	NB_FUNC(addButton)					{ self(v)->AddButton(get<wxButton>(v, 2)); return 0; }
	NB_FUNC(realize)					{ self(v)->Realize(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, GridSizer, wxSizer);

class NB_WxGridSizer : TNitClass<wxGridSizer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(cols),
			PROP_ENTRY	(rows),
			PROP_ENTRY	(hgap),
			PROP_ENTRY	(vgap),
			PROP_ENTRY_R(colsCount),
			PROP_ENTRY_R(rowsCount),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(rows, cols, vgap, hgap: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(cols)					{ return push(v, self(v)->GetCols()); }
	NB_PROP_GET(rows)					{ return push(v, self(v)->GetRows()); }
	NB_PROP_GET(hgap)					{ return push(v, self(v)->GetHGap()); }
	NB_PROP_GET(vgap)					{ return push(v, self(v)->GetVGap()); }
	NB_PROP_GET(colsCount)				{ return push(v, self(v)->GetEffectiveColsCount()); }
	NB_PROP_GET(rowsCount)				{ return push(v, self(v)->GetEffectiveRowsCount()); }

	NB_PROP_SET(cols)					{ self(v)->SetCols(getInt(v, 2)); return 0; }
	NB_PROP_SET(rows)					{ self(v)->SetRows(getInt(v, 2)); return 0; }
	NB_PROP_SET(hgap)					{ self(v)->SetHGap(getInt(v, 2)); return 0; }
	NB_PROP_SET(vgap)					{ self(v)->SetVGap(getInt(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, new wxGridSizer(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5))); return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FlexGridSizer, wxSizer);

class NB_WxFlexGridSizer : TNitClass<wxFlexGridSizer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(flexibleDirection),
			PROP_ENTRY	(nonFlexibleGrowMode),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(cols, vgap=0, hgap=0)\n"
											"(rows, cols, vgap, hgap)"),

			FUNC_ENTRY_H(addGrowableCol, 	"(idx, proportion=0)"),
			FUNC_ENTRY_H(addGrowableRow, 	"(idx, proportion=0)"),
			FUNC_ENTRY_H(isColGrowable,		"(idx): bool"),
			FUNC_ENTRY_H(isRowGrowable, 	"(idx): bool"),
			FUNC_ENTRY_H(removeGrowableCol, "(idx)"),
			FUNC_ENTRY_H(removeGrowableRow, "(idx)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "GROWMODE");
		newSlot(v, -1, "ALL",			(int)wxFLEX_GROWMODE_ALL);
		newSlot(v, -1, "NONE",			(int)wxFLEX_GROWMODE_NONE);
		newSlot(v, -1, "SPECIFIED",		(int)wxFLEX_GROWMODE_SPECIFIED);
		sq_poptop(v);
	}

	NB_CONS()
	{
		if (isNone(v, 5))
			sq_setinstanceup(v, 1, new wxFlexGridSizer(getInt(v, 2), optInt(v, 3, 0), optInt(v, 4, 0)));
		else
			sq_setinstanceup(v, 1, new wxFlexGridSizer(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5)));
		return 0;
	}

	NB_PROP_GET(flexibleDirection)		{ return push(v, self(v)->GetFlexibleDirection()); }
	NB_PROP_GET(nonFlexibleGrowMode)	{ return push(v, (int)self(v)->GetNonFlexibleGrowMode()); }

	NB_PROP_SET(flexibleDirection)		{ self(v)->SetFlexibleDirection(getInt(v, 2)); return 0; }
	NB_PROP_SET(nonFlexibleGrowMode)	{ self(v)->SetNonFlexibleGrowMode((wxFlexSizerGrowMode)getInt(v, 2)); return 0; }

	NB_FUNC(addGrowableCol)				{ self(v)->AddGrowableCol(getInt(v, 2), optInt(v, 3, 0)); return 0; }
	NB_FUNC(addGrowableRow)				{ self(v)->AddGrowableRow(getInt(v, 2), optInt(v, 3, 0)); return 0; }

	NB_FUNC(isColGrowable)				{ return push(v, self(v)->IsColGrowable(getInt(v, 2))); }
	NB_FUNC(isRowGrowable)				{ return push(v, self(v)->IsRowGrowable(getInt(v, 2))); }

	NB_FUNC(removeGrowableCol)			{ self(v)->AddGrowableCol(getInt(v, 2)); return 0; }
	NB_FUNC(removeGrowableRow)			{ self(v)->AddGrowableRow(getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, GridBagSizer, wxFlexGridSizer);

class NB_WxGridBagSizer : TNitClass<wxGridBagSizer>
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
			CONS_ENTRY_H(					"(vgap=0, hgap=0)"),
			FUNC_ENTRY_H(add,				"(window: wx.Window, row, col: int, rowspan=1, colspan=1, flag=0, border=0): wx.SizerItem"
			"\n"							"(sizer: Sizer, row, col: int, rowspan=1, colspan=1, flag=0, border=0): wx.SizerItem"
			"\n"							"(width, height: int, row, col: int, rowspan=1, colspan=1, flag=0, border=0): wx.SizerItem"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxGridBagSizer(optInt(v, 2, 0), optInt(v, 3, 0))); return SQ_OK; }

	NB_FUNC(add)
	{
		if (is<wxWindow>(v, 2))
			return push(v, self(v)->Add(get<wxWindow>(v, 2), 
				wxGBPosition(getInt(v, 3), getInt(v, 4)), 
				wxGBSpan(optInt(v, 5, 1), optInt(v, 6, 1)), 
				optInt(v, 7, 0), optInt(v, 8, 0)));
		else if (is<wxSizer>(v, 2))
			return push(v, self(v)->Add(get<wxSizer>(v, 2),
				wxGBPosition(getInt(v, 3), getInt(v, 4)), 
				wxGBSpan(optInt(v, 5, 1), optInt(v, 6, 1)), 
				optInt(v, 7, 0), optInt(v, 8, 0)));
		else
			return push(v, self(v)->Add(getInt(v, 2), getInt(v, 3),
				wxGBPosition(getInt(v, 4), getInt(v, 5)), 
				wxGBSpan(optInt(v, 6, 1), optInt(v, 7, 1)), 
				optInt(v, 8, 0), optInt(v, 9, 0)));
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Panel, wxWindow);

class NB_WxPanel : TNitClass<wxPanel>
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
			CONS_ENTRY_H(					"(parent: Window=null, id=ID.ANY, pos=null, size=null, style=WS.TAB_TRAVERSAL, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "NATIGATION_KEY",	(int)wxEVT_NAVIGATION_KEY);
		newSlot(v, -1, "INIT_DIALOG",		(int)wxEVT_INIT_DIALOG);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxWindow* wnd = new wxPanel(
			opt<wxWindow>		(v, 2, NULL),
			optInt				(v, 3, wxID_ANY),
			*opt<wxPoint>		(v, 4, wxDefaultPosition),
			*opt<wxSize>		(v, 5, wxDefaultSize),
			optInt				(v, 6, wxTAB_TRAVERSAL),
			optWxString			(v, 7, wxPanelNameStr)
			);
		sq_setinstanceup(v, 1, wnd);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ScrolledWindow, wxPanel);

class NB_WxScrolledWindow : TNitClass<wxScrolledWindow>
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
			CONS_ENTRY_H(					"(parent: Window=null, id=ID.ANY, pos=null, size=null, style=STYLE.HSCROLL | STYLE.VSCROLL, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxScrollHelper::Register<type>(v);
	}

	NB_CONS()
	{
		wxWindow* wnd = new wxScrolledWindow(
			opt<wxWindow>		(v, 2, NULL),
			optInt				(v, 3, wxID_ANY),
			*opt<wxPoint>		(v, 4, wxDefaultPosition),
			*opt<wxSize>		(v, 5, wxDefaultSize),
			optInt				(v, 6, wxHSCROLL | wxVSCROLL),
			optWxString			(v, 7, "scrolledWindow")
			);
		sq_setinstanceup(v, 1, wnd);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ScrolledCanvas, wxWindow);

class NB_WxScrolledCanvas : TNitClass<wxScrolledCanvas>
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
			CONS_ENTRY_H(					"(parent: Window=null, id=ID.ANY, pos=null, size=null, style=STYLE.HSCROLL | STYLE.VSCROLL, name=null)"),
			NULL
		};

		bind(v, props, funcs);

		NB_WxScrollHelper::Register<type>(v);
	}

	NB_CONS()
	{
		wxWindow* wnd = new wxScrolledCanvas(
			opt<wxWindow>		(v, 2, NULL),
			optInt				(v, 3, wxID_ANY),
			*opt<wxPoint>		(v, 4, wxDefaultPosition),
			*opt<wxSize>		(v, 5, wxDefaultSize),
			optInt				(v, 6, wxHSCROLL | wxVSCROLL),
			optWxString			(v, 7, "scrolledCanvas")
			);
		sq_setinstanceup(v, 1, wnd);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, SplitterWindow, wxWindow);

class NB_WxSplitterWindow : TNitClass<wxSplitterWindow>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(minPaneSize),
			PROP_ENTRY	(sashGravity),
			PROP_ENTRY	(sashPosition),
			PROP_ENTRY	(sashSize),
			PROP_ENTRY_R(defaultSashSize),
			PROP_ENTRY	(splitMode),
			PROP_ENTRY_R(window1),
			PROP_ENTRY_R(window2),
			PROP_ENTRY	(sashVisible),
			PROP_ENTRY_R(split),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(					"(parent: Window=null, id=ID.ANY, pos=null, size=null, style=STYLE.DEFAULT, name=null)"),
			FUNC_ENTRY_H(initialize,		"(win: wx.Window)"),
			FUNC_ENTRY_H(splitHorizontally,	"(win1: wx.Window, win2: wx.Window, sashPos = 0): bool"),
			FUNC_ENTRY_H(splitVertically,	"(win1: wx.Window, win2: wx.Window, sashPos = 0): bool"),
			FUNC_ENTRY_H(replaceWindow,		"(old: wx.Window, new: wx.Window): bool"),
			FUNC_ENTRY_H(unsplit,			"(toRemove: wx.Window=null): bool"),
			FUNC_ENTRY_H(updateSize,		"()"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "SPLIT");
		newSlot(v, -1, "HORIZONTAL",	(int)wxSPLIT_HORIZONTAL);
		newSlot(v, -1, "VERTICAL",		(int)wxSPLIT_VERTICAL);
		sq_poptop(v);

		addStaticTable(v, "SPLIT_DRAG");
		newSlot(v, -1, "NONE",			(int)wxSPLIT_DRAG_NONE);
		newSlot(v, -1, "DRAGGING",		(int)wxSPLIT_DRAG_DRAGGING);
		newSlot(v, -1, "LEFT_DOWN",		(int)wxSPLIT_DRAG_LEFT_DOWN);
		sq_poptop(v);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",		(int)wxSP_3D);
		newSlot(v, -1, "LOOK3D",		(int)wxSP_3D);
		newSlot(v, -1, "THIN_SASH",		(int)wxSP_THIN_SASH);
		newSlot(v, -1, "SASH3D",		(int)wxSP_3DSASH);
		newSlot(v, -1, "BORDER3D",		(int)wxSP_3DBORDER);
		newSlot(v, -1, "BORDER",		(int)wxSP_BORDER);
		newSlot(v, -1, "NOBORDER",		(int)wxSP_NOBORDER);
		newSlot(v, -1, "NO_XP_THEME",	(int)wxSP_NO_XP_THEME);
		newSlot(v, -1, "PERMIT_UNSPLIT",(int)wxSP_PERMIT_UNSPLIT);
		newSlot(v, -1, "LIVE_UPDATE",	(int)wxSP_LIVE_UPDATE);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "SASH_POS_CHANGING",		(int)wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGING);
		newSlot(v, -1, "SASH_POS_CHANGED",		(int)wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED);
		newSlot(v, -1, "UNSPLIT",				(int)wxEVT_COMMAND_SPLITTER_UNSPLIT);
		newSlot(v, -1, "DCLICK",				(int)wxEVT_COMMAND_SPLITTER_DOUBLECLICKED);
		sq_poptop(v);
	}

	NB_PROP_GET(minPaneSize)			{ return push(v, self(v)->GetMinimumPaneSize()); }
	NB_PROP_GET(sashGravity)			{ return push(v, self(v)->GetSashGravity()); }
	NB_PROP_GET(sashPosition)			{ return push(v, self(v)->GetSashPosition()); }
	NB_PROP_GET(sashSize)				{ return push(v, self(v)->GetSashSize()); }
	NB_PROP_GET(defaultSashSize)		{ return push(v, self(v)->GetDefaultSashSize()); }
	NB_PROP_GET(splitMode)				{ return push(v, (int)self(v)->GetSplitMode()); }
	NB_PROP_GET(window1)				{ return push(v, self(v)->GetWindow1()); }
	NB_PROP_GET(window2)				{ return push(v, self(v)->GetWindow2()); }
	NB_PROP_GET(sashVisible)			{ return push(v, !self(v)->IsSashInvisible()); }
	NB_PROP_GET(split)					{ return push(v, self(v)->IsSplit()); }

	NB_PROP_SET(minPaneSize)			{ self(v)->SetMinimumPaneSize(getInt(v, 2)); return 0; }
	NB_PROP_SET(sashGravity)			{ self(v)->SetSashGravity(getFloat(v, 2)); return 0; }
	NB_PROP_SET(sashPosition)			{ self(v)->SetSashPosition(getInt(v, 2)); return 0; }
	NB_PROP_SET(sashSize)				{ self(v)->SetSashSize(getInt(v, 2)); return 0; }
	NB_PROP_SET(splitMode)				{ self(v)->SetSplitMode(getInt(v, 2)); return 0; }
	NB_PROP_SET(sashVisible)			{ self(v)->SetSashInvisible(!getBool(v, 2)); return 0; }

	NB_CONS()
	{
		wxWindow* wnd = new wxSplitterWindow(
			opt<wxWindow>		(v, 2, NULL),
			optInt				(v, 3, wxID_ANY),
			*opt<wxPoint>		(v, 4, wxDefaultPosition),
			*opt<wxSize>		(v, 5, wxDefaultSize),
			optInt				(v, 6, wxTAB_TRAVERSAL),
			optWxString			(v, 7, "splitter")
			);
		sq_setinstanceup(v, 1, wnd);
		return 0;
	}

	NB_FUNC(initialize)					{ self(v)->Initialize(get<wxWindow>(v, 2)); return 0; }
	NB_FUNC(splitHorizontally)			{ return push(v, self(v)->SplitHorizontally(get<wxWindow>(v, 2), get<wxWindow>(v, 3), optInt(v, 4, 0))); }
	NB_FUNC(splitVertically)			{ return push(v, self(v)->SplitVertically(get<wxWindow>(v, 2), get<wxWindow>(v, 3), optInt(v, 4, 0))); }
	NB_FUNC(replaceWindow)				{ return push(v, self(v)->ReplaceWindow(get<wxWindow>(v, 2), get<wxWindow>(v, 3))); }
	NB_FUNC(unsplit)					{ return push(v, self(v)->Unsplit(opt<wxWindow>(v, 2, NULL))); }
	NB_FUNC(updateSize)					{ self(v)->UpdateSize(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Dialog, wxTopLevelWindow);

class NB_WxDialog : TNitClass<wxDialog>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(modal),
			PROP_ENTRY	(affirmativeId),
			PROP_ENTRY	(escapeId),
			PROP_ENTRY_R(returnCode),
			PROP_ENTRY_R(iconized),
			PROP_ENTRY_R(mainButtonIds),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(							"(parent: Window=null, id=wx.ID.ANY, title=\"\", pos=null, size=null, style=STYLE.DEFAULT, name=null)"),
			FUNC_ENTRY_H(showModal,					"(): ID"),
			FUNC_ENTRY_H(endModal,					"(retCode: int)"),
			FUNC_ENTRY_H(addMainButtonId,			"(id: ID)"),

			FUNC_ENTRY_H(center,					"(dir=DIR.BOTH)"),
			FUNC_ENTRY_H(iconize,					"(iconize=true)"),
			FUNC_ENTRY_H(setIcon,					"(icon: Icon)"),

			FUNC_ENTRY_H(createButtonSizer,			"(btnFlags: int): Sizer // use BTN_XX for flags"),
			FUNC_ENTRY_H(createSeparatedButtonSizer,"(btnFlags: int): Sizer // use BTN_XX for flags"),
			FUNC_ENTRY_H(createStdDialogButtonSizer,"(btnFlags: int): StdDialogButtonSizer // use BTN_XX for flags"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "DEFAULT",					(int)wxDEFAULT_DIALOG_STYLE);
		newSlot(v, -1, "CAPTION",					(int)wxCAPTION);
		newSlot(v, -1, "DEFAULT",					(int)wxDEFAULT_DIALOG_STYLE);
		newSlot(v, -1, "SYSTEM_MENU",				(int)wxSYSTEM_MENU);
		newSlot(v, -1, "CLOSE_BOX",					(int)wxCLOSE_BOX);
		newSlot(v, -1, "MAXIMIZE_BOX",				(int)wxMAXIMIZE_BOX);
		newSlot(v, -1, "MINIMIZE_BOX",				(int)wxMINIMIZE_BOX);
		newSlot(v, -1, "STAY_ON_TOP",				(int)wxSTAY_ON_TOP);
		newSlot(v, -1, "NO_PARENT",					(int)wxDIALOG_NO_PARENT);
		newSlot(v, -1, "EX_CONTEXTHELP",			(int)wxDIALOG_EX_CONTEXTHELP);
		sq_poptop(v);

		addStaticTable(v, "BTN");
		newSlot(v, -1, "OK",						(int)wxOK);
		newSlot(v, -1, "CANCEL",					(int)wxCANCEL);
		newSlot(v, -1, "YES",						(int)wxYES);
		newSlot(v, -1, "NO",						(int)wxNO);
		newSlot(v, -1, "APPLY",						(int)wxAPPLY);
		newSlot(v, -1, "CLOSE",						(int)wxCLOSE);
		newSlot(v, -1, "HELP",						(int)wxHELP);
		newSlot(v, -1, "NO_DEFAULT",				(int)wxNO_DEFAULT);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "CLOSE",						(int)wxEVT_CLOSE_WINDOW);
		newSlot(v, -1, "INIT_DIALOG",				(int)wxEVT_INIT_DIALOG);
		sq_poptop(v);
	}

	NB_CONS()
	{
		wxDialog* dlg = new wxDialog(
			opt<wxWindow>	(v, 2, NULL),
			optInt			(v, 3, wxID_ANY),
			optWxString		(v, 4, wxEmptyString),
			*opt<wxPoint>	(v, 5, wxDefaultPosition),
			*opt<wxSize>	(v, 6, wxDefaultSize),
			optInt			(v, 7, wxDEFAULT_DIALOG_STYLE),
			optWxString		(v, 8, wxDialogNameStr)
			);

		sq_setinstanceup(v, 1, dlg);
		return 0;
	}

	NB_PROP_GET(modal)					{ return push(v, self(v)->IsModal()); }
	NB_PROP_GET(affirmativeId)			{ return push(v, self(v)->GetAffirmativeId()); }
	NB_PROP_GET(escapeId)				{ return push(v, self(v)->GetEscapeId()); }
	NB_PROP_GET(mainButtonIds)			{ return PushWxArray(v, self(v)->GetMainButtonIds()); }
	NB_PROP_GET(returnCode)				{ return push(v, self(v)->GetReturnCode()); }
	NB_PROP_GET(iconized)				{ return push(v, self(v)->IsIconized()); }

	NB_PROP_SET(affirmativeId)			{ self(v)->SetAffirmativeId(getInt(v, 2)); return 0; }
	NB_PROP_SET(escapeId)				{ self(v)->SetEscapeId(getInt(v, 2)); return 0; }

	NB_FUNC(showModal)					{ return push(v, self(v)->ShowModal()); }
	NB_FUNC(endModal)					{ self(v)->EndModal(getInt(v, 2)); return 0; }
	NB_FUNC(addMainButtonId)			{ self(v)->AddMainButtonId(getInt(v, 2)); return 0; }

	NB_FUNC(center)						{ self(v)->Center(optInt(v, 2, wxBOTH)); return 0; }
	NB_FUNC(iconize)					{ self(v)->Iconize(getBool(v, 2)); return 0; }
	NB_FUNC(setIcon)					{ self(v)->SetIcon(*get<wxIcon>(v, 2)); return 0; }

	NB_FUNC(createButtonSizer)			{ return push(v, self(v)->CreateButtonSizer(getInt(v, 2))); }
	NB_FUNC(createSeparatedButtonSizer)	{ return push(v, self(v)->CreateSeparatedButtonSizer(getInt(v, 2))); }
	NB_FUNC(createStdDialogButtonSizer)	{ return push(v, self(v)->CreateStdDialogButtonSizer(getInt(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Wizard, wxDialog);

class NB_WxWizard : TNitClass<wxWizard>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(bitmap),
			PROP_ENTRY	(bitmapBackColor),
			PROP_ENTRY	(bitmapPlacement),
			PROP_ENTRY_R(currentPage),
			PROP_ENTRY	(minBitmapWidth),
			PROP_ENTRY_R(pageAreaSizer),
			PROP_ENTRY	(pageSize),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: wx.Window=null, id=wx.ID.ANY, title=\"\", bitmap=null, pos=null, style=STYLE.DEFAULT)"),
			FUNC_ENTRY_H(fitToPage,		"(page: wx.WizardPage)"),
			FUNC_ENTRY_H(hasNextPage,	"(page: wx.WizardPage): bool"),
			FUNC_ENTRY_H(hasPrevPage,	"(page: wx.WizardPage): bool"),
			FUNC_ENTRY_H(runWizard,		"(first: wx.WizardPage): bool"),
			FUNC_ENTRY_H(setBorder,		"(border: int)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "EX_HELPBUTTON",				(int)wxWIZARD_EX_HELPBUTTON);
		sq_poptop(v);

		addStaticTable(v, "EVT");
		newSlot(v, -1, "PAGE_CHANGED",				(int)wxEVT_WIZARD_PAGE_CHANGED);
		newSlot(v, -1, "PAGE_CHANGING",				(int)wxEVT_WIZARD_PAGE_CHANGING);
		newSlot(v, -1, "BEFORE_PAGE_CHANGED",		(int)wxEVT_WIZARD_BEFORE_PAGE_CHANGED);
		newSlot(v, -1, "PAGE_SHOWN",				(int)wxEVT_WIZARD_PAGE_SHOWN);
		newSlot(v, -1, "CANCEL",					(int)wxEVT_WIZARD_CANCEL);
		newSlot(v, -1, "HELP",						(int)wxEVT_WIZARD_HELP);
		newSlot(v, -1, "FINISHED",					(int)wxEVT_WIZARD_FINISHED);
		sq_poptop(v);
	}

	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
	NB_PROP_GET(bitmapBackColor)		{ return PushWxColor(v, self(v)->GetBitmapBackgroundColour()); }
	NB_PROP_GET(bitmapPlacement)		{ return push(v, self(v)->GetBitmapPlacement()); }
	NB_PROP_GET(currentPage)			{ return push(v, self(v)->GetCurrentPage()); }
	NB_PROP_GET(minBitmapWidth)			{ return push(v, self(v)->GetMinimumBitmapWidth()); }
	NB_PROP_GET(pageAreaSizer)			{ return push(v, self(v)->GetPageAreaSizer()); }
	NB_PROP_GET(pageSize)				{ return push(v, self(v)->GetPageSize()); }

	NB_PROP_SET(bitmap)					{ self(v)->SetBitmap(*opt<wxBitmap>(v, 2, wxNullBitmap)); return 0; }
	NB_PROP_SET(bitmapBackColor)		{ self(v)->SetBitmapBackgroundColour(GetWxColor(v, 3)); return 0; }
	NB_PROP_SET(bitmapPlacement)		{ self(v)->SetBitmapPlacement(getInt(v, 2)); return 0; }
	NB_PROP_SET(minBitmapWidth)			{ self(v)->SetMinimumBitmapWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(pageSize)				{ self(v)->SetPageSize(*get<wxSize>(v, 2)); return 0; }

	NB_CONS()							
	{ 
		setSelf(v, new type(
			opt<wxWindow>(v, 2, NULL), 
			optInt(v, 3, wxID_ANY), 
			optWxString(v, 4, wxEmptyString), 
			*opt<wxBitmap>(v, 5, wxNullBitmap), 
			*opt<wxPoint>(v, 6, wxDefaultPosition), 
			optInt(v, 7, wxDEFAULT_DIALOG_STYLE))
			);
		return SQ_OK;
	}

	NB_FUNC(fitToPage)					{ self(v)->FitToPage(get<wxWizardPage>(v, 2)); return 0; }
	NB_FUNC(hasNextPage)				{ return push(v, self(v)->HasNextPage(get<wxWizardPage>(v, 2))); }
	NB_FUNC(hasPrevPage)				{ return push(v, self(v)->HasPrevPage(get<wxWizardPage>(v, 2))); }
	NB_FUNC(runWizard)					{ return push(v, self(v)->RunWizard(get<wxWizardPage>(v, 2))); }
	NB_FUNC(setBorder)					{ self(v)->SetBorder(getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, WizardPage, wxPanel);

class NB_WxWizardPage : TNitClass<wxWizardPage>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(next),
			PROP_ENTRY_R(prev),
			PROP_ENTRY_R(bitmap),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(prev)					{ return push(v, self(v)->GetPrev()); }
	NB_PROP_GET(next)					{ return push(v, self(v)->GetNext()); }
	NB_PROP_GET(bitmap)					{ return push(v, self(v)->GetBitmap()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, WizardPageSimple, wxWizardPage);

class NB_WxWizardPageSimple : TNitClass<wxWizardPageSimple>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(next),
			PROP_ENTRY	(prev),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(parent: wx.Wizard, prev: wx.WizardPage=null, next: wx.WizardPage=null, bitmap=null)"),
			FUNC_ENTRY_H(chain,			"[class] (prev, next: wx.WizardPageSimple)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(next)					{ return push(v, self(v)->GetNext()); }
	NB_PROP_GET(prev)					{ return push(v, self(v)->GetPrev()); }

	NB_PROP_SET(next)					{ self(v)->SetNext(opt<wxWizardPage>(v, 2, NULL)); return 0; }
	NB_PROP_SET(prev)					{ self(v)->SetPrev(opt<wxWizardPage>(v, 2, NULL)); return 0; }

	NB_CONS()							{ setSelf(v, new type(get<wxWizard>(v, 2), opt<wxWizardPage>(v, 3, NULL), opt<wxWizardPage>(v, 4, NULL), *opt<wxBitmap>(v, 5, wxNullBitmap))); return SQ_OK; }

	NB_FUNC(chain)						{ type::Chain(get<wxWizardPageSimple>(v, 2), get<wxWizardPageSimple>(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, Control, wxWindow);

class NB_WxControl : TNitClass<wxControl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(font),
			PROP_ENTRY	(label),
			PROP_ENTRY	(labelText),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(command,					"(type: EVT, cmdInt=0, extraLong=0, cmdString=\"\", clientObject=null): bool // returns skipped"
			"\n"									"(eventObject: wx.Object, type: EVT, id: int, cmdInt=0, extraLong=0, cmdString=\"\", clientObject=null): bool // returns skipped"),

			FUNC_ENTRY_H(removeMnemonics,			"[class] (label: string): string"),
			FUNC_ENTRY_H(escapeMnemonics,			"[class] (text: string): string"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(font)					{ const wxFont& font = self(v)->GetFont(); return font.IsOk() ? push(v, font) : 0; }
	NB_PROP_GET(label)					{ return push(v, self(v)->GetLabel()); }
	NB_PROP_GET(labelText)				{ return push(v, self(v)->GetLabelText()); }

	NB_PROP_SET(font)					{ self(v)->SetFont(*get<wxFont>(v, 2)); return 0; }
	NB_PROP_SET(label)					{ self(v)->SetLabel(getWxString(v, 2)); return 0; }
	NB_PROP_SET(labelText)				{ self(v)->SetLabelText(getWxString(v, 2)); return 0; }

	NB_FUNC(command)
	{
		int nargs = sq_gettop(v);
		type* o = self(v);

		if (is<wxObject>(v, 2))
		{
			wxCommandEvent evt((wxEventType)getInt(v, 3), getInt(v, 4));
			evt.SetEventObject(get<wxObject>(v, 2));

			if (nargs >= 5)				evt.SetInt(getInt(v, 5));
			if (nargs >= 6)				evt.SetExtraLong(getInt(v, 6));
			if (nargs >= 7)				evt.SetString(getWxString(v, 7));
			if (nargs >= 8)				evt.SetClientObject(GetWxScriptData(v, 8, NULL));

			o->Command(evt);
			return push(v, evt.GetSkipped());
		}
		else
		{
			wxCommandEvent evt((wxEventType)getInt(v, 2), o->GetId());

			if (nargs >= 3)				evt.SetInt(getInt(v, 3));
			if (nargs >= 4)				evt.SetExtraLong(getInt(v, 4));
			if (nargs >= 5)				evt.SetString(getWxString(v, 5));
			if (nargs >= 6)				evt.SetClientObject(GetWxScriptData(v, 6, NULL));

			o->Command(evt);
			return push(v, evt.GetSkipped());
		}
	}

	NB_FUNC(removeMnemonics)			{ return push(v, self(v)->RemoveMnemonics(getWxString(v, 2))); }
	NB_FUNC(escapeMnemonics)			{ return push(v, self(v)->EscapeMnemonics(getWxString(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ScriptFrame, wxFrame);

class NB_WxScriptFrame : TNitClass<wxScriptFrame>
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
			CONS_ENTRY_H("(parent: Window=null, id=wx.ID.ANY, title=\"\", pos=null, size=null, style=STYLE.DEFAULT, name=null"),
			FUNC_ENTRY	(_inherited),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxScriptFrame* f = new wxScriptFrame(new ScriptPeer(v, 1));
		try
		{
			f->Create(
				opt<wxWindow>(v, 2, NULL),
				optInt(v, 3, wxID_ANY),
				optWxString(v, 4, wxEmptyString),
				*opt<wxPoint>(v, 5, wxDefaultPosition),
				*opt<wxSize>(v, 6, wxDefaultSize),
				optInt(v, 7, wxDEFAULT_FRAME_STYLE),
				optWxString(v, 8, wxFrameNameStr));

			setSelf(v, f);
			return SQ_OK;
		}
		catch (...)
		{
			delete f;
			throw;
		}
	}

	NB_FUNC(_inherited)					{ return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ScriptDialog, wxDialog);

class NB_WxScriptDialog : TNitClass<wxScriptDialog>
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
			CONS_ENTRY_H("(parent: Window=null, id=wx.ID.ANY, title=\"\", pos=null, size=null, style=STYLE.DEFAULT, name=null)"),
			FUNC_ENTRY	(_inherited),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxScriptDialog* d = new wxScriptDialog(new ScriptPeer(v, 1));
		try
		{
			d->Create(
				opt<wxWindow>	(v, 2, NULL),
				optInt			(v, 3, wxID_ANY),
				optWxString		(v, 4, wxEmptyString),
				*opt<wxPoint>	(v, 5, wxDefaultPosition),
				*opt<wxSize>	(v, 6, wxDefaultSize),
				optInt			(v, 7, wxDEFAULT_DIALOG_STYLE),
				optWxString		(v, 8, wxDialogNameStr)
				);

			setSelf(v, d);
			return SQ_OK;
		}
		catch (...)
		{
			delete d;
			throw;
		}
	}

	NB_FUNC(_inherited)					{ return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ScriptWizard, wxWizard);

class NB_WxScriptWizard : TNitClass<wxScriptWizard>
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
			CONS_ENTRY_H("(parent: wx.Window=null, id=wx.ID.ANY, title=\"\", bitmap=null, pos=null, style=STYLE.DEFAULT)"),
			FUNC_ENTRY	(_inherited),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		type* o = new type(new ScriptPeer(v, 1));

		try
		{
			o->Create(
				opt<wxWindow>(v, 2, NULL), 
				optInt(v, 3, wxID_ANY), 
				optWxString(v, 4, wxEmptyString), 
				*opt<wxBitmap>(v, 5, wxNullBitmap), 
				*opt<wxPoint>(v, 6, wxDefaultPosition), 
				optInt(v, 7, wxDEFAULT_DIALOG_STYLE)
				);

			setSelf(v, o);
			return SQ_OK;
		}
		catch (...)
		{
			delete o;
			throw;
		}
	}

	NB_FUNC(_inherited)					{ return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ScriptWizardPage, wxWizardPage);

class NB_WxScriptWizardPage : TNitClass<wxScriptWizardPage>
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
			CONS_ENTRY_H("(parent: wx.Wizard, bitmap=null)"),
			FUNC_ENTRY	(_inherited),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		type* o = new type(new ScriptPeer(v, 1));

		try
		{
			o->Create(
				get<wxWizard>(v, 2), 
				*opt<wxBitmap>(v, 3, wxNullBitmap)
				);

			setSelf(v, o);
			return SQ_OK;
		}
		catch (...)
		{
			delete o;
			throw;
		}
	}

	NB_FUNC(_inherited)					{ return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, ScriptPanel, wxPanel);

class NB_WxScriptPanel : TNitClass<wxScriptPanel>
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
			CONS_ENTRY_H("(parent: Window=null, id=ID.ANY, pos=null, size=null, style=WS.TAB_TRAVERSAL, name=null)"),
			FUNC_ENTRY	(_inherited),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxScriptPanel* p = new wxScriptPanel(new ScriptPeer(v, 1));

		try
		{
			p->Create(
				opt<wxWindow>		(v, 2, NULL),
				optInt				(v, 3, wxID_ANY),
				*opt<wxPoint>		(v, 4, wxDefaultPosition),
				*opt<wxSize>		(v, 5, wxDefaultSize),
				optInt				(v, 6, wxTAB_TRAVERSAL),
				optWxString			(v, 7, wxPanelNameStr)
				);

			setSelf(v, p);
			return SQ_OK;
		}
		catch (...)
		{
			delete p;
			throw;
		}
	}

	NB_FUNC(_inherited)					{ return SQ_OK; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FileSystem, wxObject);

class NB_WxFileSystem : public TNitClass<wxFileSystem>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(path),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY_H(openFile,		"(location: string): wx.FSFile"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(path)					{ return push(v, self(v)->GetPath()); }

	NB_CONS()							{ setSelf(v, new wxFileSystem()); return SQ_OK; }

	NB_FUNC(openFile)					{ return push(v, self(v)->OpenFile(getWxString(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, FSFile, wxObject);

class NB_WxFSFile : public TNitClass<wxFSFile>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(anchor),
			PROP_ENTRY_R(location),
			PROP_ENTRY_R(mimeType),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(anchor)					{ return push(v, self(v)->GetAnchor()); }
	NB_PROP_GET(location)				{ return push(v, self(v)->GetLocation()); }
	NB_PROP_GET(mimeType)				{ return push(v, self(v)->GetMimeType()); }
};

////////////////////////////////////////////////////////////////////////////////

class wxNitXmlResource : public wxXmlResource
{
public:
	wxNitXmlResource(int flags = wxXRC_USE_LOCALE, const wxString& domain = wxEmptyString)
		: wxXmlResource(flags, domain)
	{
	}

	wxNitXmlResource(const wxString& filemask, int flags = wxXRC_USE_LOCALE, const wxString& domain = wxEmptyString)
		: wxXmlResource(filemask, flags, domain)
	{
	}

public:
	bool LoadSource(nit::Ref<StreamSource> source)
	{
		wxString filename = wxString::FromUTF8(source->getUrl().c_str());
		wxLogTrace(wxT("xrc"), wxT("opening '%s'"), filename);

		wxInputStream *stream = NULL;

		wxNitInputStream gstream(source->open());
		stream = &gstream;

		wxXmlDocument* const doc = DoLoadStream(filename, stream);
		if (!doc)
			return false;
		else
			AddDataRecord(filename, doc);

		return true;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, XmlResource, wxObject);

class NB_WxXmlResource : TNitClass<wxXmlResource>
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
			CONS_ENTRY_H(				"(flags=XRC.USE_LOCALE, domain=\"\")\n"
										"(filemask: string, flags=XRC.USE_LOCALE, domain=\"\")"),
			FUNC_ENTRY_H(initAllHandlers,	"[class] ()"),
			FUNC_ENTRY_H(load,			"(filemask: string): bool\n"
										"(source: StreamSource): bool\n"),

			FUNC_ENTRY_H(loadObject,	"(parent: wx.Window, name: string, classname: string, recursive=false): wx.Object"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		wxXmlResource* xrc;
		
		if (isString(v, 2))
			xrc = new wxNitXmlResource(getWxString(v, 2), optInt(v, 3, wxXRC_USE_LOCALE), optWxString(v, 4, ""));
		else
			xrc = new wxNitXmlResource(optInt(v, 2, wxXRC_USE_LOCALE), optWxString(v, 3, ""));

		setSelf(v, xrc);

		xrc->InitAllHandlers();

		return SQ_OK;
	}

	NB_FUNC(initAllHandlers)			{ self(v)->InitAllHandlers(); return 0; }

	NB_FUNC(load)
	{ 
		if (is<StreamSource>(v, 2))
		{
			wxNitXmlResource* gxrc = dynamic_cast<wxNitXmlResource*>(self(v));
			if (gxrc)
				return push(v, gxrc->LoadSource(get<StreamSource>(v, 2)));
			else
				return sq_throwerror(v, "not supported");
		}

		return push(v, self(v)->Load(getWxString(v, 2))); 
	}

	NB_FUNC(loadObject)				
	{ 
		if (optBool(v, 5, false))
			return push(v, self(v)->LoadObjectRecursively(opt<wxWindow>(v, 2, NULL), getWxString(v, 3), getWxString(v, 4)));
		else
			return push(v, self(v)->LoadObject(opt<wxWindow>(v, 2, NULL), getWxString(v, 3), getWxString(v, 4))); 
	}
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxBase(HSQUIRRELVM v)
{
	NB_WxObject::Register(v);

	NB_WxClassInfo::Register(v);

	NB_WxPoint::Register(v);
	NB_WxSize::Register(v);
	NB_WxRect::Register(v);

	NB_WxPalette::Register(v);
	NB_WxImage::Register(v);
	NB_WxImageList::Register(v);
	NB_WxBitmap::Register(v);
	NB_WxIcon::Register(v);
	NB_WxCursor::Register(v);
	NB_WxArtProvider::Register(v);
	NB_WxFont::Register(v);

	NB_WxEvtHandler::Register(v);

	NB_WxEvent::Register(v);

	NB_WxValidator::Register(v);

	NB_WxAppConsole::Register(v);
	NB_WxApp::Register(v);
	NB_WxNitApp::Register(v);

	NB_WxWindow::Register(v);
	NB_WxTopLevelWindow::Register(v);
	NB_WxFrame::Register(v);
	NB_WxPanel::Register(v);

	NB_WxScrolledWindow::Register(v);
	NB_WxScrolledCanvas::Register(v);

	NB_WxSplitterWindow::Register(v);

	NB_WxDialog::Register(v);

	NB_WxWizard::Register(v);
	NB_WxWizardPage::Register(v);
	NB_WxWizardPageSimple::Register(v);

	NB_WxControl::Register(v);

	NB_WxScriptFrame::Register(v);
	NB_WxScriptDialog::Register(v);
	NB_WxScriptPanel::Register(v);
	NB_WxScriptWizard::Register(v);
	NB_WxScriptWizardPage::Register(v);

	NB_WxSizerFlags::Register(v);
	NB_WxSizerItem::Register(v);
	NB_WxSizer::Register(v);
	NB_WxBoxSizer::Register(v);
	NB_WxStaticBoxSizer::Register(v);
	NB_WxStdDialogButtonSizer::Register(v);
	NB_WxGridSizer::Register(v);
	NB_WxFlexGridSizer::Register(v);
	NB_WxGridBagSizer::Register(v);

	NB_WxMenuItem::Register(v);
	NB_WxMenu::Register(v);
	NB_WxMenuBar::Register(v);

	NB_WxStatusBar::Register(v);

	NB_WxFileSystem::Register(v);
	NB_WxFSFile::Register(v);

	NB_WxXmlResource::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
