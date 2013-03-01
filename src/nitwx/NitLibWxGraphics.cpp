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

#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#ifdef NIT_WIN32
#   include <wx/msw/enhmeta.h>
#endif

NS_NIT_BEGIN;

// We do not use wxNullPen or wxNullBrush in script, Opt<> will return them as default when the value null

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Pen, NULL);

class NB_WxPen : TNitClass<wxPen>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(cap),
			PROP_ENTRY	(join),
			PROP_ENTRY	(style),
			PROP_ENTRY	(width),
			PROP_ENTRY	(color),
			PROP_ENTRY	(stipple),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()\n"
										"(col: Color, width=1, style=STYLE.SOLID)\n"
										"(stipple: wx.Bitmap, width: int)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "SOLID",				(int)wxPENSTYLE_SOLID);
		newSlot(v, -1, "DOT",				(int)wxPENSTYLE_DOT);
		newSlot(v, -1, "LONG_DASH",			(int)wxPENSTYLE_LONG_DASH);
		newSlot(v, -1, "SHORT_DASH",		(int)wxPENSTYLE_SHORT_DASH);
		newSlot(v, -1, "DOT_DASH",			(int)wxPENSTYLE_DOT_DASH);
		newSlot(v, -1, "TRANSPARENT",		(int)wxPENSTYLE_TRANSPARENT);
		newSlot(v, -1, "STIPPLE",			(int)wxPENSTYLE_STIPPLE);
		newSlot(v, -1, "BDIAGONAL_HATCH",	(int)wxPENSTYLE_BDIAGONAL_HATCH);
		newSlot(v, -1, "CROSSDIAG_HATCH",	(int)wxPENSTYLE_CROSSDIAG_HATCH);
		newSlot(v, -1, "FDIAGONAL_HATCH",	(int)wxPENSTYLE_FDIAGONAL_HATCH);
		newSlot(v, -1, "CROSS_HATCH",		(int)wxPENSTYLE_CROSS_HATCH);
		newSlot(v, -1, "HORIZONTAL_HATCH",	(int)wxPENSTYLE_HORIZONTAL_HATCH);
		newSlot(v, -1, "VERTICAL_HATCH",	(int)wxPENSTYLE_VERTICAL_HATCH);
		sq_poptop(v);

		addStaticTable(v, "CAP");
		newSlot(v, -1, "ROUND",			(int)wxCAP_ROUND);
		newSlot(v, -1, "PROJECTING",	(int)wxCAP_PROJECTING);
		newSlot(v, -1, "BUTT",			(int)wxCAP_BUTT);
		sq_poptop(v);

		addStaticTable(v, "JOIN");
		newSlot(v, -1, "BEVEL",			(int)wxJOIN_BEVEL);
		newSlot(v, -1, "MITER",			(int)wxJOIN_MITER);
		newSlot(v, -1, "ROUND",			(int)wxJOIN_ROUND);
		sq_poptop(v);

 		addStatic(v, "BLACK_DASHED",	*wxBLACK_DASHED_PEN);
		addStatic(v, "BLACK",			*wxBLACK_PEN);
		addStatic(v, "BLUE",			*wxBLUE_PEN);
		addStatic(v, "CYAN",			*wxCYAN_PEN);
		addStatic(v, "GREEN",			*wxGREEN_PEN);
		addStatic(v, "YELLOW",			*wxYELLOW_PEN);
		addStatic(v, "LIGHT_GREY",		*wxLIGHT_GREY_PEN);
		addStatic(v, "MEDIUM_GREY",		*wxMEDIUM_GREY_PEN);
		addStatic(v, "RED",				*wxRED_PEN);
		addStatic(v, "TRANSPARENT",		*wxTRANSPARENT_PEN);
		addStatic(v, "WHITE",			*wxWHITE_PEN);
	}

	NB_PROP_GET(cap)					{ return push(v, (int)self(v)->GetCap()); }
	NB_PROP_GET(join)					{ return push(v, (int)self(v)->GetJoin()); }
	NB_PROP_GET(style)					{ return push(v, (int)self(v)->GetStyle()); }
	NB_PROP_GET(width)					{ return push(v, self(v)->GetWidth()); }
	NB_PROP_GET(color)					{ return PushWxColor(v, self(v)->GetColour()); }
	NB_PROP_GET(stipple)				{ wxBitmap* s = self(v)->GetStipple(); return s ? push(v, *s) : 0; }

	NB_PROP_SET(cap)					{ self(v)->SetCap(wxPenCap(getInt(v, 2))); return 0; }
	NB_PROP_SET(join)					{ self(v)->SetJoin(wxPenJoin(getInt(v, 2))); return 0; }
	NB_PROP_SET(style)					{ self(v)->SetStyle(wxPenStyle(getInt(v, 2))); return 0; }
	NB_PROP_SET(width)					{ self(v)->SetWidth(getInt(v, 2)); return 0; }
	NB_PROP_SET(color)					{ self(v)->SetColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(stipple)				{ self(v)->SetStipple(*get<wxBitmap>(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			new (self(v)) wxPen();
		else if (is<wxBitmap>(v, 2))
			new (self(v)) wxPen(*get<wxBitmap>(v, 2), getInt(v, 3));
		else
			new (self(v)) wxPen(GetWxColor(v, 2), optInt(v, 3, 1), (wxPenStyle)optInt(v, 4, wxPENSTYLE_SOLID));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Brush, NULL);

class NB_WxBrush : TNitClass<wxBrush>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(style),
			PROP_ENTRY	(color),
			PROP_ENTRY	(stipple),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()\n"
										"(col: Color, style=STYLE.SOLID)\n"
										"(stipple: wx.Bitmap)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "STYLE");
		newSlot(v, -1, "SOLID",				(int)wxBRUSHSTYLE_SOLID);
		newSlot(v, -1, "TRANSPARENT",		(int)wxBRUSHSTYLE_TRANSPARENT);
		newSlot(v, -1, "STIPPLE",			(int)wxBRUSHSTYLE_STIPPLE);
		newSlot(v, -1, "BDIAGONAL_HATCH",	(int)wxBRUSHSTYLE_BDIAGONAL_HATCH);
		newSlot(v, -1, "CROSSDIAG_HATCH",	(int)wxBRUSHSTYLE_CROSSDIAG_HATCH);
		newSlot(v, -1, "FDIAGONAL_HATCH",	(int)wxBRUSHSTYLE_FDIAGONAL_HATCH);
		newSlot(v, -1, "CROSS_HATCH",		(int)wxBRUSHSTYLE_CROSS_HATCH);
		newSlot(v, -1, "HORIZONTAL_HATCH",	(int)wxBRUSHSTYLE_HORIZONTAL_HATCH);
		newSlot(v, -1, "VERTICAL_HATCH",	(int)wxBRUSHSTYLE_VERTICAL_HATCH);
		sq_poptop(v);

		addStatic(v, "BLACK",			*wxBLACK_BRUSH);
		addStatic(v, "BLUE",			*wxBLUE_BRUSH);
		addStatic(v, "CYAN",			*wxCYAN_BRUSH);
		addStatic(v, "GREEN",			*wxGREEN_BRUSH);
		addStatic(v, "YELLOW",			*wxYELLOW_BRUSH);
		addStatic(v, "LIGHT_GREY",		*wxLIGHT_GREY_BRUSH);
		addStatic(v, "MEDIUM_GREY",		*wxMEDIUM_GREY_BRUSH);
		addStatic(v, "RED",				*wxRED_BRUSH);
		addStatic(v, "TRANSPARENT",		*wxTRANSPARENT_BRUSH);
		addStatic(v, "WHITE",			*wxWHITE_BRUSH);
	}

	NB_PROP_GET(style)					{ return push(v, (int)self(v)->GetStyle()); }
	NB_PROP_GET(color)					{ return PushWxColor(v, self(v)->GetColour()); }
	NB_PROP_GET(stipple)				{ wxBitmap* s = self(v)->GetStipple(); return s ? push(v, *s) : 0; }

	NB_PROP_SET(style)					{ self(v)->SetStyle(wxPenStyle(getInt(v, 2))); return 0; }
	NB_PROP_SET(color)					{ self(v)->SetColour(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(stipple)				{ self(v)->SetStipple(*get<wxBitmap>(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 2))
			new (self(v)) wxBrush();
		else if (is<wxBitmap>(v, 2))
			new (self(v)) wxBrush(*get<wxBitmap>(v, 2));
		else
			new (self(v)) wxBrush(GetWxColor(v, 2), (wxBrushStyle)optInt(v, 3, wxBRUSHSTYLE_SOLID));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

class wxPointArray : public wxRefCounter
{
public:
	wxPointArray() 
	{
	}

	virtual ~wxPointArray()
	{
	}

	SQRESULT Item(HSQUIRRELVM v)
	{
		int idx = NitBind::getInt(v, 2);
		if (idx < 0 || idx >= (int)_array.size())
			return sq_throwerror(v, "out of index");
		return NitBind::push(v, _array[idx]);
	}

	SQRESULT SetItem(HSQUIRRELVM v)
	{
		int idx = NitBind::getInt(v, 2);
		if (idx < 0 || idx >= (int)_array.size())
			return sq_throwerror(v, "out of index");
		if (NitBind::isInt(v, 3))
			_array[idx] = wxPoint(NitBind::getInt(v, 3), NitBind::getInt(v, 4));
		else
			_array[idx] = *NitBind::get<wxPoint>(v, 3);
		return 0;
	}

	SQRESULT Append(HSQUIRRELVM v, int objIdx)
	{
		if (sq_gettype(v, objIdx) != OT_ARRAY)
			return sq_throwerror(v, "invalid arg");

		for (NitIterator itr(v, objIdx); itr.hasNext(); itr.next())
		{
			int vidx = itr.valueIndex();
			if (sq_gettype(v, vidx) == OT_INTEGER)
			{
				int x = NitBind::getInt(v, vidx);
				itr.next();
				if (!itr.hasNext())
					return sq_throwerror(v, "unbalanced x, y pair");
				int y = NitBind::getInt(v, vidx);

				_array.push_back(wxPoint(x, y));
			}
			else
			{
				_array.push_back(*NitBind::get<wxPoint>(v, vidx));
			}
		}

		return 0;
	}

	size_t GetCount() { return _array.size(); }

	wxPoint* GetPoints() { return _array.empty() ? NULL : &_array[0]; }

	vector<wxPoint>::type _array;
};

NB_TYPE_WXREF(NITWX_API, PointArray, NULL);

class NB_WxPointArray : TNitClass<wxPointArray>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(count),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(
				"()\n"
				"(pts: int[])\n"
				"(pts: wx.Point[]"),
			FUNC_ENTRY_H(item,			"(idx: int): wx.Point"),
			FUNC_ENTRY_H(setItem,		"(idx: int, pt: wx.Point)\n"
										"(idx: int, x, y: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(count)
	{
		return push(v, self(v)->_array.size());
	}

	NB_FUNC(item)
	{
		return push(v, self(v)->Item(v));
	}

	NB_FUNC(setItem)
	{
		return push(v, self(v)->SetItem(v));
	}

	NB_CONS()
	{
		type& self = *setSelf(v, new wxPointArray());

		if (isNone(v, 2)) 
			return 0;

		return self.Append(v, 2);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXVALUE(NITWX_API, Region, NULL);

class NB_WxRegion : public TNitClass<wxRegion>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(empty),
			PROP_ENTRY_R(box),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(
				"()\n"
				"(x, y, width, height)\n"
				"(rect: wx.Rect)\n"
				"(pts: wx.PointArray, fillStyle = wx.DC.POLY_FILL.ODDEVEN) // can use [int, int] or [wx.Point] too\n"
				"(bitmap: wx.Bitmap): bool\n"
				"(bitmap: wx.Bitmap, transparent: Color, tolerance=0): bool"),
			FUNC_ENTRY_H(clear,			"()"),
			FUNC_ENTRY_H(toBitmap,		"(): wx.Bitmap"),
			FUNC_ENTRY_H(contains,		"(pt: wx.Point): CONTAIN\n"
										"(rect: wx.Rect): CONTAIN"),
			FUNC_ENTRY_H(intersect,		"(rect: wx.Rect): bool\n"
										"(region: wx.Region): bool"),
			FUNC_ENTRY_H(subtract,		"(rect: wx.Rect): bool\n"
										"(region: wx.Region): bool"),
			FUNC_ENTRY_H(union,			"(rect: wx.Rect): bool\n"
										"(region: wx.Region): bool\n"
										"(bitmap: wx.Bitmap): bool\n"
										"(bitmap: wx.Bitmap, transparent: Color, tolerance=0): bool"),
			FUNC_ENTRY_H(xor,			"(rect: wx.Rect): bool\n"
										"(region: wx.Region): bool"),
			FUNC_ENTRY_H(offset,		"(x, y: int): bool\n"
										"(delta: wx.Point): bool"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "CONTAIN");
		newSlot(v, -1, "OUT",			(int)wxOutRegion);
		newSlot(v, -1, "PART",			(int)wxPartRegion);
		newSlot(v, -1, "IN",			(int)wxInRegion);
		sq_poptop(v);
	}

	NB_PROP_GET(empty)					{ return push(v, self(v)->IsEmpty()); }
	NB_PROP_GET(box)					{ return push(v, self(v)->GetBox()); }

	NB_CONS()
	{
		if (isNone(v, 2))
			new (self(v)) wxRegion();
		else if (is<wxRect>(v, 2))
			new (self(v)) wxRegion(*get<wxRect>(v, 2));
		else if (is<wxPointArray>(v, 2))
		{
			wxPointArray* pts = get<wxPointArray>(v, 2);
			new (self(v)) wxRegion(pts->GetCount(), pts->GetPoints(), (wxPolygonFillMode)optInt(v, 5, wxODDEVEN_RULE));
		}
		else if (sq_gettype(v, 2) == OT_ARRAY)
		{
			wxPointArray pts;
			SQRESULT sr = pts.Append(v, 2);
			if (SQ_FAILED(sr)) return sr;
			new (self(v)) wxRegion(pts.GetCount(), pts.GetPoints(), (wxPolygonFillMode)optInt(v, 5, wxODDEVEN_RULE));
		}
		else if (is<wxBitmap>(v, 2))
		{
			if (isNone(v, 3))
				new (self(v)) wxRegion(*get<wxBitmap>(v, 2));
			else
				new (self(v)) wxRegion(*get<wxBitmap>(v, 2), GetWxColor(v, 3), optInt(v, 4, 0));
		}
		else
			sq_throwerror(v, "invalid args");
		return 0;
	}

	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }

	NB_FUNC(toBitmap)					{ return push(v, self(v)->ConvertToBitmap()); }

	NB_FUNC(contains)
	{
		if (is<wxPoint>(v, 2))
			return push(v, (int)self(v)->Contains(*get<wxPoint>(v, 2)));
		else
			return push(v, (int)self(v)->Contains(*get<wxRect>(v, 2)));
	}

	NB_FUNC(intersect)
	{
		if (is<wxRect>(v, 2))
			return push(v, self(v)->Intersect(*get<wxRect>(v, 2)));
		else
			return push(v, self(v)->Intersect(*get<wxRegion>(v, 2)));
	}

	NB_FUNC(subtract)
	{
		if (is<wxRect>(v, 2))
			return push(v, self(v)->Subtract(*get<wxRect>(v, 2)));
		else
			return push(v, self(v)->Subtract(*get<wxRegion>(v, 2)));
	}

	NB_FUNC(union)
	{
		if (is<wxRect>(v, 2))
			return push(v, self(v)->Union(*get<wxRect>(v, 2)));
		else if (is<wxBitmap>(v, 2))
		{
			if (isNone(v, 3))
				return push(v, self(v)->Union(*get<wxBitmap>(v, 2)));
			else
				return push(v, self(v)->Union(*get<wxBitmap>(v, 2), GetWxColor(v, 3), optInt(v, 4, 0)));
		}
		else
			return push(v, self(v)->Union(*get<wxRegion>(v, 2)));
	}

	NB_FUNC(xor)
	{
		if (is<wxRect>(v, 2))
			return push(v, self(v)->Xor(*get<wxRect>(v, 2)));
		else
			return push(v, self(v)->Xor(*get<wxRegion>(v, 2)));
	}

	NB_FUNC(offset)
	{
		if (is<wxPoint>(v, 2))
			self(v)->Offset(*get<wxPoint>(v, 2));
		else
			self(v)->Offset(getInt(v, 2), getInt(v, 3));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

typedef wxAffineMatrix2D wxAffine;

NB_TYPE_WXVALUE(NITWX_API, Affine, NULL);

class NB_WxAffine : public TNitClass<wxAffineMatrix2D>
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
			CONS_ENTRY_H(				"() // constructs identity"),
			FUNC_ENTRY_H(translate,		"(tx, ty: float): wx.Affine"),
			FUNC_ENTRY_H(rotate,		"(radian: float): wx.Affine"),
			FUNC_ENTRY_H(scale,			"(sx, sy: float): wx.Affine"),
			FUNC_ENTRY_H(invert,		"(): bool"),
			FUNC_ENTRY_H(mirror,		"(dir = wx.ORIENT.HORIZONTAL): wx.Affine"),
			FUNC_ENTRY_H(concat,		"(t: wx.Affine)"),
			FUNC_ENTRY	(_mul),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ new (self(v)) wxAffineMatrix2D(); return 0; }

	NB_FUNC(translate)					
	{ 
		wxAffineMatrix2D t = *self(v);
		t.Translate(getFloat(v, 2), getFloat(v, 3)); 
		return push(v, t);
	}

	NB_FUNC(rotate)					
	{ 
		wxAffineMatrix2D t = *self(v);
		t.Rotate(getFloat(v, 2));
		return push(v, t);
	}

	NB_FUNC(scale)						
	{ 
		wxAffineMatrix2D t = *self(v);
		t.Scale(getFloat(v, 2), getFloat(v, 3));
		return push(v, t);
	}

	NB_FUNC(invert)
	{ 
		wxAffineMatrix2D t = *self(v);
		t.Invert(); 
		return push(v, t);
	}

	NB_FUNC(mirror)
	{ 
		wxAffineMatrix2D t = *self(v);
		t.Mirror(optInt(v, 2, wxHORIZONTAL));
		return push(v, t);
	}

	NB_FUNC(concat)
	{ 
		wxAffineMatrix2D t = *self(v);
		t.Concat(*get<wxAffineMatrix2D>(v, 2));
		return push(v, t);
	}

	NB_FUNC(_mul)
	{
		wxAffineMatrix2D t = *self(v);
		double x, y;

		if (is<Vector2>(v, 2))
		{
			Vector2& p = *get<Vector2>(v, 2);
			x = p.x; y = p.y;
			t.TransformPoint(&x, &y);
			return push(v, Vector2(float(x), float(y)));
		}
		else if (is<wxAffineMatrix2D>(v, 2))
			t.Concat(*get<wxAffineMatrix2D>(v, 2));
		else
		{
			float scale = getFloat(v, 2);
			t.Scale(scale, scale);
		}

		return push(v, t);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, DC, NULL, delete);

class NB_WxDC : TNitClass<wxDC>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(font),
			PROP_ENTRY	(pen),
			PROP_ENTRY	(brush),
			PROP_ENTRY	(backgroundMode),
			PROP_ENTRY	(textForeground),
			PROP_ENTRY	(textBackground),

			PROP_ENTRY	(background),
			PROP_ENTRY	(deviceOrigin),
			PROP_ENTRY	(mapMode),
			PROP_ENTRY	(logicalOP),
			PROP_ENTRY	(logicalOrigin),

			PROP_ENTRY_R(ppi),
			PROP_ENTRY_R(size),
			PROP_ENTRY	(userScale),
			PROP_ENTRY_R(clippingBox),

			PROP_ENTRY_R(charHeight),
			PROP_ENTRY_R(charWidth),

			PROP_ENTRY_R(depth),

			PROP_ENTRY	(transform),

			PROP_ENTRY_R(boundingBox),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(destroy,		"() // purges instance"),

			FUNC_ENTRY_H(clear,			"()"),

			FUNC_ENTRY_H(drawLine,		"(pt0, pt1: wx.Point)\n"
										"(x0, y0, x1, y1: int)\n"),
			FUNC_ENTRY_H(drawText,		"(text: string, pt: wx.Point)\n"
										"(text: string, x, y: int)"),
			FUNC_ENTRY_H(drawBitmap,	"(bitmap: wx.Bitmap, pt: wx.Point, useMask=false)\n"
										"(bitmap: wx.Bitmap, x, y: int, useMask=false)"),
			FUNC_ENTRY_H(drawArc,		"(s, e, c: wx.Point)\n"
										"(sx, sy, ex, ey, cx, cy: int)"),
			FUNC_ENTRY_H(drawCircle,	"(pt: wx.Point, radius:int)\n"
										"(cx, cy: int, radius: int)"),
			FUNC_ENTRY_H(drawEllipse,	"(pt: wx.Point, size: wx.Size)\n"
										"(cx, cy, w, h: int)"),
			FUNC_ENTRY_H(drawEllipticArc, "(pt: wx.Point, size: wx.Size, sa, ea: float)\n"
										"(cx, cy, w, h: int, sa, ea: float)"),
			FUNC_ENTRY_H(drawLabel,		"(text: string, bitmap: wx.Bitmap, rect: wx.Rect, align=(wx.ALIGN.LEFT|wx.ALIGN.TOP))\n"
										"(text: string, rect: wx.Rect, align=(wx.ALIGN.LEFT|wx.ALIGN.TOP))"),
			FUNC_ENTRY_H(drawPoint,		"(pt: wx.Point)"
										"(x, y: int)"),
			FUNC_ENTRY_H(drawRect,		"(pt: wx.Point, sz: wx.Size)\n"
										"(rect: wx.Rect)\n"
										"(x, y, w, h: int)"),
			FUNC_ENTRY_H(drawRoundRect,	"(pt: wx.Point, sz: wx.Size, radius: float)\n"
										"(rect: wx.Rect, radius: float)\n"
										"(x, y, w, h: int, radius: float)"),
			FUNC_ENTRY_H(floodFill,		"(pt: wx.Point, col: wx.Color, fillStyle=FLOOD_FILL.SURFACE)\n"
										"(x, y: int, col: wx.Color, fillStyle=FLOOD_FILL.SURFACE)"),
			FUNC_ENTRY_H(crossHair,		"(pt: wx.Point)\n"
										"(x, y: int)"),
			FUNC_ENTRY_H(drawLines,		"(pts: wx.PointArray, offx=0, offy=0) // can use [int, int] or [wx.Point] too"),
			FUNC_ENTRY_H(drawPolygon,	"(pts: wx.PointArray, offx=0, offy=0, fillMode=POLY_FILL.ODDEVEN) // can use [int, int] or [wx.Point] too"),
			FUNC_ENTRY_H(drawSpline,	"(pts: wx.PointArray) // can use [] too"
										"(x1, y1, x2, y2, x3, y3: int)"),

			FUNC_ENTRY_H(setClippingRegion, "(rect: wx.Rect)"),

			FUNC_ENTRY_H(getTextExtent, "(string, font: wx.Font=null): wx.Size)"),

			FUNC_ENTRY_H(setAxisOrient,	"(toRight: bool, toUp: bool)"),
			FUNC_ENTRY_H(toLogical,		"(deviceCoord: wx.Point): wx.Point"),
			FUNC_ENTRY_H(toLogicalRel,	"(deviceCoord: wx.Point): wx.Point"),
			FUNC_ENTRY_H(toDevice,		"(logicalCoord: wx.Point): wx.Point"),
			FUNC_ENTRY_H(toDeviceRel,	"(logicalCoord: wx.Point): wx.Point"),

			FUNC_ENTRY_H(resetTransform, "()"),

			FUNC_ENTRY_H(blit,			"(dstRect: wx.Rect, source: wx.DC, srcPt: wx.Point=null, logicalOp=OP.COPY, useMask=false, srcMask: wx.Point=null): bool"),
			FUNC_ENTRY_H(stretchBlit,	"(dstRect: wx.Rect, source: wx.DC, srcRect: wx.Rect, logicalOp=OP.COPY, useMask=false, srcMask: wx.Point=null): bool"),

			FUNC_ENTRY_H(startDoc,		"(message: string): bool"),
			FUNC_ENTRY_H(startPage,		"()"),
			FUNC_ENTRY_H(endDoc,		"()"),
			FUNC_ENTRY_H(endPage,		"()"),

			FUNC_ENTRY_H(copyAttributes,"(dc: wx.DC)"),

			FUNC_ENTRY_H(getPixel,		"(x, y: int): Color"),

			FUNC_ENTRY_H(gradientFillConcentric, "(rect: wx.Rect, c0: Color, c1: Color, center: wx.Point=null)"),
			FUNC_ENTRY_H(gradientFillLinear, "(rect: wx.Rect, c0: Color, c1: Color, dir=wx.DIR.RIGHT)"),

			FUNC_ENTRY_H(resetBoundingBox, "()"),
			FUNC_ENTRY_H(calcBoundingBox, "(x, y: int)"),

			FUNC_ENTRY_H(setPalette,	"(palette: wx.Palette=null)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "POLY_FILL");
		newSlot(v, -1, "ODDEVEN",		(int)wxODDEVEN_RULE);
		newSlot(v, -1, "WINDING",		(int)wxWINDING_RULE);
		sq_poptop(v);

		addStaticTable(v, "FLOOD_FILL");
		newSlot(v, -1, "BORDER",		(int)wxFLOOD_BORDER);
		newSlot(v, -1, "SURFACE",		(int)wxFLOOD_SURFACE);
		sq_poptop(v);

		addStaticTable(v, "BG_MODE");
		newSlot(v, -1, "TRANSPARENT",	(int)wxTRANSPARENT);
		newSlot(v, -1, "SOLID",			(int)wxSOLID);
		sq_poptop(v);

		addStaticTable(v, "OP");
		newSlot(v, -1, "CLEAR",			(int)wxCLEAR);
		newSlot(v, -1, "XOR",			(int)wxXOR);
		newSlot(v, -1, "INVERT",		(int)wxINVERT);
		newSlot(v, -1, "OR_REVERSE",	(int)wxOR_REVERSE);
		newSlot(v, -1, "AND_REVERSE",	(int)wxAND_REVERSE);
		newSlot(v, -1, "COPY",			(int)wxCOPY);
		newSlot(v, -1, "AND",			(int)wxAND);
		newSlot(v, -1, "AND_INVERT",	(int)wxAND_INVERT);
		newSlot(v, -1, "NO_OP",			(int)wxNO_OP);
		newSlot(v, -1, "NOR",			(int)wxNOR);
		newSlot(v, -1, "EQUIV",			(int)wxEQUIV);
		newSlot(v, -1, "SRC_INVERT",	(int)wxSRC_INVERT);
		newSlot(v, -1, "OR_INVERT",		(int)wxOR_INVERT);
		newSlot(v, -1, "NAND",			(int)wxNAND);
		newSlot(v, -1, "OR",			(int)wxOR);
		newSlot(v, -1, "SET",			(int)wxSET);
		sq_poptop(v);

		addStaticTable(v, "MM");
		newSlot(v, -1, "TEXT",			(int)wxMM_TEXT);
		newSlot(v, -1, "METRIC",		(int)wxMM_METRIC);
		newSlot(v, -1, "LOMETRIC",		(int)wxMM_LOMETRIC);
		newSlot(v, -1, "TWIPS",			(int)wxMM_TWIPS);
		newSlot(v, -1, "POINTS",		(int)wxMM_POINTS);
		sq_poptop(v);
	}

	NB_PROP_GET(font)					{ return push(v, self(v)->GetFont()); }
	NB_PROP_GET(pen)					{ const wxPen& pen = self(v)->GetPen(); return pen.IsOk() ? push(v, pen) : 0; }
	NB_PROP_GET(brush)					{ const wxBrush& brush = self(v)->GetBrush(); return brush.IsOk() ? push(v, brush) : 0; }
	NB_PROP_GET(background)				{ const wxBrush& brush = self(v)->GetBackground(); return brush.IsOk() ? push(v, brush) : 0; }
	NB_PROP_GET(textForeground)			{ return PushWxColor(v, self(v)->GetTextForeground()); }
	NB_PROP_GET(textBackground)			{ return PushWxColor(v, self(v)->GetTextBackground()); }

	NB_PROP_GET(backgroundMode)			{ return push(v, (int)self(v)->GetBackgroundMode()); }
	NB_PROP_GET(deviceOrigin)			{ return push(v, self(v)->GetDeviceOrigin()); }
	NB_PROP_GET(mapMode)				{ return push(v, (int)self(v)->GetMapMode()); }
	NB_PROP_GET(logicalOP)				{ return push(v, (int)self(v)->GetLogicalFunction()); }
	NB_PROP_GET(logicalOrigin)			{ return push(v, self(v)->GetLogicalOrigin()); }

	NB_PROP_GET(ppi)					{ return push(v, self(v)->GetPPI()); }
	NB_PROP_GET(size)					{ return push(v, self(v)->GetSize()); }
	NB_PROP_GET(userScale)				{ double sx, sy; self(v)->GetUserScale(&sx, &sy); return push(v, Vector2((float)sx, (float)sy)); }
	NB_PROP_GET(clippingBox)			{ wxRect rt; self(v)->GetClippingBox(&rt.x, &rt.y, &rt.width, &rt.height); return push(v, rt); }

	NB_PROP_GET(charHeight)				{ return push(v, self(v)->GetCharHeight()); }
	NB_PROP_GET(charWidth)				{ return push(v, self(v)->GetCharWidth()); }

	NB_PROP_GET(depth)					{ return push(v, self(v)->GetDepth()); }

	NB_PROP_GET(transform)				{ return push(v, self(v)->GetTransformMatrix()); }

	NB_PROP_GET(boundingBox)			
	{ 
		wxRect box;
		type* o = self(v);
		box.x = o->MinX();
		box.y = o->MinY();
		box.width = o->MaxX() - box.x;
		box.height = o->MaxY() - box.y;
		return push(v, box);
	}

	NB_PROP_SET(font)					{ self(v)->SetFont(*opt<wxFont>(v, 2, &wxNullFont)); return 0; }
	NB_PROP_SET(pen)					{ self(v)->SetPen(*opt<wxPen>(v, 2, &wxNullPen)); return 0; }
	NB_PROP_SET(brush)					{ self(v)->SetBrush(*opt<wxBrush>(v, 2, &wxNullBrush)); return 0; }
	NB_PROP_SET(background)				{ self(v)->SetBackground(*opt<wxBrush>(v, 2, &wxNullBrush)); return 0; }
	NB_PROP_SET(textForeground)			{ self(v)->SetTextForeground(GetWxColor(v, 2)); return 0; }
	NB_PROP_SET(textBackground)			{ self(v)->SetTextBackground(GetWxColor(v, 2)); return 0; }

	NB_PROP_SET(backgroundMode)			{ self(v)->SetBackgroundMode(getInt(v, 2)); return 0; }
	NB_PROP_SET(deviceOrigin)			{ wxPoint& o = *get<wxPoint>(v, 2); self(v)->SetDeviceOrigin(o.x, o.y); return 0; }
	NB_PROP_SET(mapMode)				{ self(v)->SetMapMode((wxMappingMode)getInt(v, 2)); return 0; }
	NB_PROP_SET(logicalOP)				{ self(v)->SetLogicalFunction((wxRasterOperationMode)getInt(v, 2)); return 0; }
	NB_PROP_SET(logicalOrigin)			{ wxPoint& o = *get<wxPoint>(v, 2); self(v)->SetLogicalOrigin(o.x, o.y); return 0; }
	NB_PROP_SET(userScale)				{ Vector2& s = *get<Vector2>(v, 2); self(v)->SetUserScale(s.x, s.y); return 0; }

	NB_PROP_SET(transform)				{ self(v)->SetTransformMatrix(*get<wxAffineMatrix2D>(v, 2)); return 0; }

	NB_FUNC(destroy)					{ sq_purgeinstance(v, 1); return 0; }

	NB_FUNC(clear)						{ self(v)->Clear(); return 0; }

	NB_FUNC(drawLine)					
	{ 
		if (is<wxPoint>(v, 2))
			self(v)->DrawLine(*get<wxPoint>(v, 2), *get<wxPoint>(v, 3));
		else
			self(v)->DrawLine(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5));
		return 0; 
	}

	NB_FUNC(drawText)					
	{ 
		if (is<wxPoint>(v, 3))
			self(v)->DrawText(getWxString(v, 2), *get<wxPoint>(v, 3)); 
		else
			self(v)->DrawText(getWxString(v, 2), getInt(v, 3), getInt(v, 4));
		return 0; 
	}

	NB_FUNC(drawBitmap)
	{ 
		if (is<wxPoint>(v, 3))
			self(v)->DrawBitmap(*get<wxBitmap>(v, 2), *get<wxPoint>(v, 3), optBool(v, 4, false)); 
		else
			self(v)->DrawBitmap(*get<wxBitmap>(v, 2), getInt(v, 3), getInt(v, 4), optBool(v, 5, false)); 

		return 0; 
	}

	NB_FUNC(drawArc)
	{
		if (is<wxPoint>(v, 2))
			self(v)->DrawArc(*get<wxPoint>(v, 2), *get<wxPoint>(v, 3), *get<wxPoint>(v, 4));
		else
			self(v)->DrawArc(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), getInt(v, 6), getInt(v, 7));
		return 0;
	}

	NB_FUNC(drawCircle)
	{
		if (is<wxPoint>(v, 2))
			self(v)->DrawCircle(*get<wxPoint>(v, 2), getInt(v, 3));
		else
			self(v)->DrawCircle(getInt(v, 2), getInt(v, 3), getInt(v, 4));
		return 0;
	}

	NB_FUNC(drawEllipse)
	{
		if (is<wxPoint>(v, 2))
			self(v)->DrawEllipse(*get<wxPoint>(v, 2), *get<wxSize>(v, 3));
		else
			self(v)->DrawEllipse(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5));
		return 0;
	}

	NB_FUNC(drawEllipticArc)
	{
		if (is<wxPoint>(v, 2))
			self(v)->DrawEllipticArc(*get<wxPoint>(v, 2), *get<wxSize>(v, 3), getFloat(v, 4), getFloat(v, 5));
		else
			self(v)->DrawEllipticArc(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), getFloat(v, 6), getFloat(v, 7));
		return 0;
	}

	NB_FUNC(drawLabel)
	{
		if (is<wxBitmap>(v, 3))
			self(v)->DrawLabel(getWxString(v, 2), *get<wxBitmap>(v, 3), *get<wxRect>(v, 4), optInt(v, 5, wxALIGN_LEFT | wxALIGN_TOP));
		else
			self(v)->DrawLabel(getWxString(v, 2), *get<wxRect>(v, 3), optInt(v, 4, wxALIGN_LEFT | wxALIGN_TOP));
		return 0;
	}

	NB_FUNC(drawPoint)
	{
		if (is<wxPoint>(v, 2))
			self(v)->DrawPoint(*get<wxPoint>(v, 2));
		else
			self(v)->DrawPoint(getInt(v, 2), getInt(v, 3));
		return 0;
	}

	NB_FUNC(drawRect)
	{
		if (is<wxPoint>(v, 2))
			self(v)->DrawRectangle(*get<wxPoint>(v, 2), *get<wxSize>(v, 3));
		else if (is<wxRect>(v, 3))
			self(v)->DrawRectangle(*get<wxRect>(v, 2));
		else
			self(v)->DrawRectangle(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5));
		return 0;
	}

	NB_FUNC(drawRoundRect)
	{
		if (is<wxPoint>(v, 2))
			self(v)->DrawRoundedRectangle(*get<wxPoint>(v, 2), *get<wxSize>(v, 3), getInt(v, 4));
		else if (is<wxRect>(v, 3))
			self(v)->DrawRoundedRectangle(*get<wxRect>(v, 2), getInt(v, 3));
		else
			self(v)->DrawRoundedRectangle(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), getInt(v, 6));
		return 0;
	}

	NB_FUNC(floodFill)
	{
		if (is<wxPoint>(v, 2))
			self(v)->FloodFill(*get<wxPoint>(v, 2), GetWxColor(v, 3), (wxFloodFillStyle)optInt(v, 4, wxFLOOD_SURFACE));
		else
			self(v)->FloodFill(getInt(v, 2), getInt(v, 3), GetWxColor(v, 4), (wxFloodFillStyle)optInt(v, 5, wxFLOOD_SURFACE));
		return 0;
	}

	NB_FUNC(crossHair)
	{
		if (is<wxPoint>(v, 2))
			self(v)->CrossHair(*get<wxPoint>(v, 2));
		else
			self(v)->CrossHair(getInt(v, 2), getInt(v, 3));
		return 0;
	}

	NB_FUNC(drawLines)
	{
		if (sq_gettype(v, 2) == OT_ARRAY)
		{
			wxPointArray pts;
			SQRESULT sr = pts.Append(v, 2);
			if (SQ_FAILED(sr)) return sr;
			self(v)->DrawLines(pts.GetCount(), pts.GetPoints(), optInt(v, 3, 0), optInt(v, 4, 0));
		}
		else
		{
			wxPointArray* pts = get<wxPointArray>(v, 2);
			self(v)->DrawLines(pts->GetCount(), pts->GetPoints(), optInt(v, 3, 0), optInt(v, 4, 0));
		}
		return 0;
	}

	NB_FUNC(drawSpline)
	{
		if (sq_gettype(v, 2) == OT_ARRAY)
		{
			wxPointArray pts;
			SQRESULT sr = pts.Append(v, 2);
			if (SQ_FAILED(sr)) return sr;
			self(v)->DrawSpline(pts.GetCount(), pts.GetPoints());
		}
		else if (isInt(v, 2))
		{
			self(v)->DrawSpline(getInt(v, 2), getInt(v, 3), getInt(v, 4), getInt(v, 5), getInt(v, 6), getInt(v, 7));
		}
		else
		{
			wxPointArray* pts = get<wxPointArray>(v, 2);
			self(v)->DrawSpline(pts->GetCount(), pts->GetPoints());
		}
		return 0;
	}

	NB_FUNC(drawPolygon)
	{
		if (sq_gettype(v, 2) == OT_ARRAY)
		{
			wxPointArray pts;
			SQRESULT sr = pts.Append(v, 2);
			if (SQ_FAILED(sr)) return sr;
			self(v)->DrawPolygon(pts.GetCount(), pts.GetPoints(), optInt(v, 3, 0), optInt(v, 4, 0), (wxPolygonFillMode)optInt(v, 5, wxODDEVEN_RULE));
		}
		else
		{
			wxPointArray* pts = get<wxPointArray>(v, 2);
			self(v)->DrawPolygon(pts->GetCount(), pts->GetPoints(), optInt(v, 3, 0), optInt(v, 4, 0), (wxPolygonFillMode)optInt(v, 5, wxODDEVEN_RULE));
		}
		return 0;
	}

	NB_FUNC(setClippingRegion)
	{
		if (is<wxRect>(v, 2))
			self(v)->SetClippingRegion(*get<wxRect>(v, 2));
		else
			self(v)->SetDeviceClippingRegion(*get<wxRegion>(v, 2));
		return 0; 
	}

	NB_FUNC(destroyClippingRegion)
	{
		self(v)->DestroyClippingRegion();
		return 0;
	}

	NB_FUNC(getTextExtent)
	{
		wxCoord w, h;
		self(v)->GetMultiLineTextExtent(
			getWxString(v, 2),
			&w, &h, NULL,
			opt<wxFont>(v, 3, NULL));
		return push(v, wxSize(w, h));
	}

	NB_FUNC(setAxisOrient)
	{
		self(v)->SetAxisOrientation(getBool(v, 2), getBool(v, 3));
		return 0;
	}

	NB_FUNC(toLogical)
	{
		type* o = self(v);
		wxPoint coord = *get<wxPoint>(v, 2);
		coord.x = o->DeviceToLogicalX(coord.x);
		coord.y = o->DeviceToLogicalY(coord.y);
		return push(v, coord);
	}

	NB_FUNC(toLogicalRel)
	{
		type* o = self(v);
		wxPoint coord = *get<wxPoint>(v, 2);
		coord.x = o->DeviceToLogicalXRel(coord.x);
		coord.y = o->DeviceToLogicalYRel(coord.y);
		return push(v, coord);
	}

	NB_FUNC(toDevice)
	{
		type* o = self(v);
		wxPoint coord = *get<wxPoint>(v, 2);
		coord.x = o->LogicalToDeviceX(coord.x);
		coord.y = o->LogicalToDeviceY(coord.y);
		return push(v, coord);
	}

	NB_FUNC(toDeviceRel)
	{
		type* o = self(v);
		wxPoint coord = *get<wxPoint>(v, 2);
		coord.x = o->LogicalToDeviceXRel(coord.x);
		coord.y = o->LogicalToDeviceYRel(coord.y);
		return push(v, coord);
	}

	NB_FUNC(resetTransform)
	{
		self(v)->ResetTransformMatrix();
		return 0;
	}

	NB_FUNC(blit)
	{
		wxRect& dstRect = *get<wxRect>(v, 2);
		wxDC* source = get<wxDC>(v, 3);
		const wxPoint& srcPt = *opt<wxPoint>(v, 4, wxPoint(0, 0));
		wxRasterOperationMode logicalOp = (wxRasterOperationMode)optInt(v, 5, wxCOPY);
		bool useMask = optBool(v, 6, false);
		const wxPoint& srcMask = *opt<wxPoint>(v, 7, wxPoint(wxDefaultCoord, wxDefaultCoord));

		return push(v, self(v)->Blit(
			dstRect.x, dstRect.y, dstRect.width, dstRect.height,
			source,
			srcPt.x, srcPt.y,
			logicalOp,
			useMask,
			srcMask.x, srcMask.y));
	}

	NB_FUNC(stretchBlit)
	{
		wxRect& dstRect = *get<wxRect>(v, 2);
		wxDC* source = get<wxDC>(v, 3);
		const wxRect& srcRect = *opt<wxRect>(v, 4, wxRect(0, 0, 0, 0));
		wxRasterOperationMode logicalOp = (wxRasterOperationMode)optInt(v, 5, wxCOPY);
		bool useMask = optBool(v, 6, false);
		const wxPoint& srcMask = *opt<wxPoint>(v, 7, wxPoint(wxDefaultCoord, wxDefaultCoord));

		return push(v, self(v)->StretchBlit(
			dstRect.x, dstRect.y, dstRect.width, dstRect.height,
			source,
			srcRect.x, srcRect.y, srcRect.width, srcRect.height,
			logicalOp,
			useMask,
			srcMask.x, srcMask.y));
	}

	NB_FUNC(startDoc)					{ return push(v, self(v)->StartDoc(getWxString(v, 2))); }
	NB_FUNC(startPage)					{ self(v)->StartPage(); return 0; }
	NB_FUNC(endDoc)						{ self(v)->EndDoc(); return 0; }
	NB_FUNC(endPage)					{ self(v)->EndPage(); return 0; }

	NB_FUNC(copyAttributes)				{ self(v)->CopyAttributes(*get<wxDC>(v, 2)); return 0; }

	NB_FUNC(getPixel)
	{
		wxColour color;
		bool ok = self(v)->GetPixel(getInt(v, 2), getInt(v, 3), &color);
		if (!ok) sq_throwerror(v, "can't get pixel");
		return PushWxColor(v, color);
	}

	NB_FUNC(gradientFillConcentric)
	{
		if (isNone(v, 5))
			self(v)->GradientFillConcentric(*get<wxRect>(v, 2), GetWxColor(v, 3), GetWxColor(v, 4));
		else
			self(v)->GradientFillConcentric(*get<wxRect>(v, 2), GetWxColor(v, 3), GetWxColor(v, 4), *get<wxPoint>(v, 5));
		return 0;
	}

	NB_FUNC(gradientFillLinear)
	{
		self(v)->GradientFillLinear(*get<wxRect>(v, 2), GetWxColor(v, 3), GetWxColor(v, 4), (wxDirection)optInt(v, 5, wxRIGHT));
		return 0;
	}

	NB_FUNC(resetBoundingBox)			{ self(v)->ResetBoundingBox(); return 0; }
	NB_FUNC(calcBoundingBox)			{ self(v)->CalcBoundingBox(getInt(v, 2), getInt(v, 3)); return 0; }

	NB_FUNC(setPalette)					{ self(v)->SetPalette(*opt<wxPalette>(v, 2, wxNullPalette)); return 0; } 
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, MemoryDC, wxDC, delete);

class NB_WxMemoryDC : TNitClass<wxMemoryDC>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(selectedBitmap),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()\n"
										"(wx.Bitmap)\n"
										"(wx.DC)"),
			FUNC_ENTRY_H(selectObject,	"(bmp: wx.Bitmap)"),
			FUNC_ENTRY_H(selectObjectAsSource, "(bmp: wx.Bitmap)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(selectedBitmap)			{ return push(v, self(v)->GetSelectedBitmap()); }

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new wxMemoryDC());
		else if (is<wxBitmap>(v, 2))
			setSelf(v, new wxMemoryDC(*get<wxBitmap>(v, 2)));
		else
			setSelf(v, new wxMemoryDC(get<wxDC>(v, 3)));
		return 0;
	}

	NB_FUNC(selectObject)				{ self(v)->SelectObject(*get<wxBitmap>(v, 2)); return 0; }
	NB_FUNC(selectObjectAsSource)		{ self(v)->SelectObjectAsSource(*get<wxBitmap>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, WindowDC, wxDC, delete);

class NB_WxWindowDC : TNitClass<wxWindowDC>
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
			CONS_ENTRY_H(				"(wx.Window)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxWindowDC(get<wxWindow>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, BufferedDC, wxMemoryDC, delete);

class NB_WxBufferedDC : TNitClass<wxBufferedDC>
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
			CONS_ENTRY_H(				"()\n"
										"(dc: wx.DC, area: wx.Size, style=BUFFER.CLIENT_AREA)\n"
										"(dc: wx.DC, buffer=null, style=BUFFER.CLIENT_AREA)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "BUFFER");
		newSlot(v, -1, "CLIENT_AREA",	(int)wxBUFFER_CLIENT_AREA);
		newSlot(v, -1, "VIRTUAL_AREA",	(int)wxBUFFER_VIRTUAL_AREA);
		sq_poptop(v);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, new wxBufferedDC());
		else if (!isNone(v, 3) && is<wxSize>(v, 3))
			setSelf(v, new wxBufferedDC(get<wxDC>(v, 2), *get<wxSize>(v, 3), optInt(v, 4, wxBUFFER_CLIENT_AREA)));
		else
			setSelf(v, new wxBufferedDC(get<wxDC>(v, 2), *opt<wxBitmap>(v, 3, &wxNullBitmap), optInt(v, 4, wxBUFFER_CLIENT_AREA)));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, AutoBufferedPaintDC, wxDC, delete);

class NB_WxAutoBufferedPaintDC : TNitClass<wxAutoBufferedPaintDC>
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
			CONS_ENTRY_H(				"(wx.Window)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxAutoBufferedPaintDC(get<wxWindow>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, ClientDC, wxWindowDC, delete);

class NB_WxClientDC : TNitClass<wxClientDC>
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
			CONS_ENTRY_H(				"(wx.Window)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxClientDC(get<wxWindow>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, GCDC, wxDC, delete);

class NB_WxGCDC : TNitClass<wxGCDC>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(graphicsContext),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(wx.WindowDC)\n"
										"(wx.MemoryDC)\n"
										"(wx.PrinterDC)\n"
										"(wx.MetafileDC)\n"
										"(wx.GraphicsContext)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(graphicsContext)		{ return push(v, self(v)->GetGraphicsContext()); }
	
	NB_PROP_SET(graphicsContext)		{ self(v)->SetGraphicsContext(get<wxGraphicsContext>(v, 2)); return 0; }

	NB_CONS()
	{
		if (is<wxWindowDC>(v, 2))
			setSelf(v, new wxGCDC(*get<wxWindowDC>(v, 2)));
		else if (is<wxMemoryDC>(v, 2))
			setSelf(v, new wxGCDC(*get<wxMemoryDC>(v, 2)));
		else if (is<wxPrinterDC>(v, 2))
			setSelf(v, new wxGCDC(*get<wxPrinterDC>(v, 2)));
#ifdef NIT_WIN32
		else if (is<wxEnhMetaFileDC>(v, 2))
			setSelf(v, new wxGCDC(*get<wxEnhMetaFileDC>(v, 2)));
#endif
		else
			setSelf(v, new wxGCDC(get<wxGraphicsContext>(v, 2)));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, ScreenDC, wxDC, delete);

class NB_WxScreenDC : TNitClass<wxScreenDC>
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
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, new wxScreenDC()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WX_AUTODELETE(NITWX_API, PrinterDC, wxDC, delete);

class NB_WxPrinterDC : TNitClass<wxPrinterDC>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(paperRect),
			PROP_ENTRY_R(resolution),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(paperRect)				{ return push(v, self(v)->GetPaperRect()); }
	NB_PROP_GET(resolution)				{ return push(v, self(v)->GetResolution()); }

	NB_CONS()							{ setSelf(v, new wxPrinterDC()); return 0; }
};


////////////////////////////////////////////////////////////////////////////////

#ifdef NIT_WIN32

NB_TYPE_WX_AUTODELETE(NITWX_API, EnhMetaFileDC, wxDC, delete);

class NB_WxEnhMetaFileDC : TNitClass<wxEnhMetaFileDC>
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
			CONS_ENTRY_H(				"(filename=\"\", width=0, height=0, desc=\"\")"),
			FUNC_ENTRY_H(close,			"(): wx.EnhMetaFile // purges this but caller should delete result"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							
	{ 
		setSelf(v, new wxEnhMetaFileDC(optWxString(v, 2, ""), optInt(v, 3, 0), optInt(v, 4, 0), optWxString(v, 5, ""))); return 0; 
	}
	
	NB_FUNC(close)
	{
		return sq_throwerror(v, "not implemented");
// 		wxEnhMetaFile* file = Self(v)->Close();
// 		sq_purgeinstance(v, 1);
// 		return Push(v, file);
	}
};

#endif

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WXOBJ(NITWX_API, GraphicsPath, wxObject);

class NB_WxGraphicsPath : TNitClass<wxGraphicsPath>
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

NB_TYPE_WXOBJ(NITWX_API, GraphicsContext, wxObject);

class NB_WxGraphicsContext : TNitClass<wxGraphicsContext>
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
			CONS_ENTRY_H(				"(wx.Window)\n(wx.Image)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (is<wxWindow>(v, 2))
			setSelf(v, wxGraphicsContext::Create(get<wxWindow>(v, 2)));
		else
			setSelf(v, wxGraphicsContext::Create(*get<wxImage>(v, 2)));

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

SQRESULT NitLibWxGraphics(HSQUIRRELVM v)
{
	NB_WxPen::Register(v);
	NB_WxBrush::Register(v);

	NB_WxPointArray::Register(v);
	NB_WxRegion::Register(v);
	NB_WxAffine::Register(v);

	NB_WxDC::Register(v);
	NB_WxMemoryDC::Register(v);
	NB_WxWindowDC::Register(v);

	NB_WxBufferedDC::Register(v);
	NB_WxAutoBufferedPaintDC::Register(v);
	NB_WxClientDC::Register(v);
	NB_WxGCDC::Register(v);
	NB_WxScreenDC::Register(v);
	NB_WxPrinterDC::Register(v);
#ifdef NIT_WIN32
	NB_WxEnhMetaFileDC::Register(v);
#endif

	sq_dostring(v, "wx.PaintDC := wx.AutoBufferedPaintDC");
//	sq_dostring(v, "wx.MetafileDC := wx.EnhMetaFileDC");

	NB_WxGraphicsPath::Register(v);
	NB_WxGraphicsContext::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
