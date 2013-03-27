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

#include "nit/script/NitLibData.h"

#include "CCGrid.h"
#include "CCGrabber.h"
#include "CCRibbon.h"
#include "CCConfiguration.h"
#include "CCKeypadDispatcher.h"
#include "CCIMEDispatcher.h"
#include "CCFont.h"

using namespace cocos2d;

NS_NIT_BEGIN;

#pragma warning (disable: 4345)			// behavior change: an object of POD type constructed with an initializer of the form() will be default-initialized

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCSize, Size);
NB_TYPE_VALUE(NIT2D_API, cc::Size, NULL);

class NB_CCSize : TNitClass<CCSize>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(width),
			PROP_ENTRY	(height),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(Width=0, Height=0: float): cc.Size"),

			FUNC_ENTRY	(_add),
			FUNC_ENTRY	(_sub),
			FUNC_ENTRY	(_div),
			FUNC_ENTRY	(_unm),
			FUNC_ENTRY	(_mul),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		new (self(v)) CCPoint(optFloat(v, 2, 0.0f), optFloat(v, 3, 0.0f));

		return 0;
	}

	NB_PROP_GET(width)					{ return push(v, self(v)->width); }
	NB_PROP_GET(height)					{ return push(v, self(v)->height); }

	NB_PROP_SET(width)					{ self(v)->width = getFloat(v, 2); return 0; }
	NB_PROP_SET(height)					{ self(v)->height = getFloat(v, 2); return 0; }

	NB_FUNC(_add)						
	{ 
		if (is<CCPoint>(v, 2))
		{
			CCSize* a = self(v);
			CCPoint* b = get<CCPoint>(v, 2);
			return push(v, CCSize(a->width + b->x, a->height + b->y));
		}
		else
		{
			CCSize* a = self(v);
			CCSize* b = get<CCSize>(v, 2);
			return push(v, CCSize(a->width + b->width, a->height + b->height));
		}
	}

	NB_FUNC(_sub)						
	{ 
		if (is<CCPoint>(v, 2))
		{
			CCSize* a = self(v);
			CCPoint* b = get<CCPoint>(v, 2);
			return push(v, CCSize(a->width - b->x, a->height - b->y));
		}
		else
		{
			CCSize* a = self(v);
			CCSize* b = get<CCSize>(v, 2);
			return push(v, CCSize(a->width - b->width, a->height - b->height));
		}
	}

	NB_FUNC(_div)						
	{ 
		CCSize* a = self(v);
		float b = getFloat(v, 2);
		return push(v, CCSize(a->width / b, a->height / b));
	}

	NB_FUNC(_mul)						
	{ 
		CCSize* a = self(v);
		float b = getFloat(v, 2);
		return push(v, CCSize(a->width * b, a->height * b));
	}

	NB_FUNC(_unm)						
	{ 
		CCSize* a = self(v);
		return push(v, CCSize(-a->width, -a->height));
	}

	NB_FUNC(_tostring)
	{
		CCSize* sz = self(v);
		pushFmt(v, "Size(%.3f, %.3f)", sz->width, sz->height);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCRect, Rect);
NB_TYPE_VALUE(NIT2D_API, cc::Rect, NULL);

class NB_CCRect : TNitClass<CCRect>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(origin),
			PROP_ENTRY	(size),
			PROP_ENTRY_R(minX),
			PROP_ENTRY_R(maxX),
			PROP_ENTRY_R(minY),
			PROP_ENTRY_R(maxY),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(x=0, y=0, width=0, height=0)\n"
										"(origin, size: Point)"),

			FUNC_ENTRY_H(contains,		"(pt: Point): bool"),
			FUNC_ENTRY_H(intersects,	"(rect: Rect): bool"),
			FUNC_ENTRY	(_add),
			FUNC_ENTRY	(_sub),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(origin)					{ return push(v, self(v)->origin); }
	NB_PROP_GET(size)					{ return push(v, self(v)->size); }
	NB_PROP_GET(minX)					{ return push(v, CCRect::CCRectGetMinX(*self(v))); }
	NB_PROP_GET(maxX)					{ return push(v, CCRect::CCRectGetMaxX(*self(v))); }
	NB_PROP_GET(minY)					{ return push(v, CCRect::CCRectGetMinY(*self(v))); }
	NB_PROP_GET(maxY)					{ return push(v, CCRect::CCRectGetMaxY(*self(v))); }

	NB_PROP_SET(origin)					{ self(v)->origin = *get<CCPoint>(v, 2); return 0; }
	NB_PROP_SET(size)					{ self(v)->size = *get<CCSize>(v, 2); return 0; }

	NB_CONS()
	{
		if (is<CCPoint>(v, 2))
		{
			const CCPoint& p = *get<CCPoint>(v, 2);
			const CCSize& s = *get<CCSize>(v, 3);
			new (self(v)) CCRect(p.x, p.y, s.width, s.height);
		}
		else
			new (self(v)) CCRect(optFloat(v, 2, 0.0f), optFloat(v, 3, 0.0f), optFloat(v, 4, 0.0f), optFloat(v, 5, 0.0f));

		return 0;
	}

	NB_FUNC(contains)					{ return push(v, CCRect::CCRectContainsPoint(*self(v), *get<CCPoint>(v, 2))); }
	NB_FUNC(intersects)					{ return push(v, CCRect::CCRectIntersectsRect(*self(v), *get<CCRect>(v, 2))); }

	NB_FUNC(_add)
	{
		if (is<CCPoint>(v, 2))
		{
			CCRect* a = self(v);
			CCPoint* b = get<CCPoint>(v, 2);
			return push(v, CCRect(a->origin.x + b->x, a->origin.y + b->y, a->size.width, a->size.height));
		}
		else
		{
			CCRect* a = self(v);
			CCSize* b = get<CCSize>(v, 2);
			return push(v, CCRect(a->origin.x, a->origin.y, a->size.width + b->width, a->size.height + b->height));
		}
	}

	NB_FUNC(_sub)
	{
		if (is<CCPoint>(v, 2))
		{
			CCRect* a = self(v);
			CCPoint* b = get<CCPoint>(v, 2);
			return push(v, CCRect(a->origin.x - b->x, a->origin.y - b->y, a->size.width, a->size.height));
		}
		else
		{
			CCRect* a = self(v);
			CCSize* b = get<CCSize>(v, 2);
			return push(v, CCRect(a->origin.x, a->origin.y, a->size.width - b->width, a->size.height - b->height));
		}
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCAffineTransform, Affine);
NB_TYPE_VALUE(NIT2D_API, cc::Affine, NULL);

class NB_CCAffine : TNitClass<CCAffineTransform>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(a),
			PROP_ENTRY_R(b),
			PROP_ENTRY_R(c),
			PROP_ENTRY_R(d),
			PROP_ENTRY_R(tx),
			PROP_ENTRY_R(ty),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(a=1.0, b=0.0, c=0.0, d=1.0, tx=0.0, ty=0.0)"),
			FUNC_ENTRY_H(translate,		"(tx, ty: float): cc.AffineTransform"),
			FUNC_ENTRY_H(rotate,		"(angle: float): cc.AffineTransform"),
			FUNC_ENTRY_H(scale,			"(sx, sy: float): cc.AffineTransform"),
			FUNC_ENTRY_H(invert,		"(): cc.AffineTransform"),
			FUNC_ENTRY	(_mul),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "IDENTITY",		CCAffineTransformMakeIdentity());
	}

	NB_PROP_GET(a)						{ return push(v, self(v)->a); }
	NB_PROP_GET(b)						{ return push(v, self(v)->b); }
	NB_PROP_GET(c)						{ return push(v, self(v)->c); }
	NB_PROP_GET(d)						{ return push(v, self(v)->d); }
	NB_PROP_GET(tx)						{ return push(v, self(v)->tx); }
	NB_PROP_GET(ty)						{ return push(v, self(v)->ty); }

	NB_PROP_SET(a)						{ self(v)->a = getFloat(v, 2); return 0; }
	NB_PROP_SET(b)						{ self(v)->b = getFloat(v, 2); return 0; }
	NB_PROP_SET(c)						{ self(v)->c = getFloat(v, 2); return 0; }
	NB_PROP_SET(d)						{ self(v)->d = getFloat(v, 2); return 0; }
	NB_PROP_SET(tx)						{ self(v)->tx = getFloat(v, 2); return 0; }
	NB_PROP_SET(ty)						{ self(v)->ty = getFloat(v, 2); return 0; }

	NB_CONS()
	{
		CCAffineTransform& at = *self(v);
		at.a = optFloat(v, 2, 1.0f);
		at.b = optFloat(v, 3, 0.0f);
		at.c = optFloat(v, 4, 0.0f);
		at.d = optFloat(v, 5, 1.0f);
		at.tx = optFloat(v, 6, 0.0f);
		at.ty = optFloat(v, 7, 0.0f);
		return 0;
	}

	NB_FUNC(translate)					{ return push(v, CCAffineTransformTranslate(*self(v), getFloat(v, 2), getFloat(v, 3))); }
	NB_FUNC(rotate)						{ return push(v, CCAffineTransformRotate(*self(v), getFloat(v, 2))); }
	NB_FUNC(scale)						{ return push(v, CCAffineTransformScale(*self(v), getFloat(v, 2), getFloat(v, 3))); }
	NB_FUNC(invert)						{ return push(v, CCAffineTransformInvert(*self(v))); }

	NB_FUNC(_add)
	{
		return push(v, CCAffineTransformTranslate(*self(v), getFloat(v, 2), getFloat(v, 3)));
	}

	NB_FUNC(_sub)
	{
		return push(v, CCAffineTransformTranslate(*self(v), -getFloat(v, 2), -getFloat(v, 3)));
	}

	NB_FUNC(_mul)
	{
		if (is<CCPoint>(v, 2))
			return push(v, CCPointApplyAffineTransform(*get<CCPoint>(v, 2), *self(v)));
		if (is<CCSize>(v, 2))
			return push(v, CCSizeApplyAffineTransform(*get<CCSize>(v, 2), *self(v)));
		if (is<CCRect>(v, 2))
			return push(v, CCRectApplyAffineTransform(*get<CCRect>(v, 2), *self(v)));
		if (is<CCAffineTransform>(v, 2))
			return push(v, CCAffineTransformConcat(*self(v), *get<CCAffineTransform>(v, 2)));

		float scale = getFloat(v, 2);
		return push(v, CCAffineTransformScale(*self(v), scale, scale));
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, ccGridSize, GridSize);
NB_TYPE_VALUE(NIT2D_API, cc::GridSize, NULL);

class NB_CCGridSize : TNitClass<ccGridSize>
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
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(x)						{ return push(v, self(v)->x); }
	NB_PROP_GET(y)						{ return push(v, self(v)->y); }

	NB_PROP_SET(x)						{ self(v)->x = getInt(v, 2); return 0; }
	NB_PROP_SET(y)						{ self(v)->y = getInt(v, 2); return 0; }

	NB_CONS()							{ type* o = self(v); o->x = optInt(v, 2, 0); o->y = optInt(v, 3, 0); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCObject, Object);
NB_TYPE_CC_REF(NIT2D_API, cc::Object, RefCounted);

class NB_CCObject : TNitClass<CCObject>
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

NS_IMPORT_NIT(cc, CCNode, Node);
NB_TYPE_CC_REF(NIT2D_API, cc::Node, CCObject);

class NB_CCNode : TNitClass<CCNode>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(description),
			PROP_ENTRY_R(zOrder),
			PROP_ENTRY	(vertexZ),
			PROP_ENTRY	(rotation),
			PROP_ENTRY	(scaleX),
			PROP_ENTRY	(scaleY),
			PROP_ENTRY	(position),
			PROP_ENTRY	(positionInPixels),
			PROP_ENTRY	(skewX),
			PROP_ENTRY	(skewY),
			PROP_ENTRY_R(camera),
			PROP_ENTRY	(grid),
			PROP_ENTRY	(visible),
			PROP_ENTRY	(anchorPoint),
			PROP_ENTRY_R(anchorPointInPixels),
			PROP_ENTRY	(contentSize),
			PROP_ENTRY	(contentSizeInPixels),
			PROP_ENTRY_R(running),
			PROP_ENTRY	(parent),
			PROP_ENTRY	(relativeAnchorPoint),
			PROP_ENTRY	(tag),
			PROP_ENTRY	(userValue),
			PROP_ENTRY_R(numRunningActions),
			PROP_ENTRY_R(children),
			PROP_ENTRY	(clipActive),
			PROP_ENTRY_R(scene),
			PROP_ENTRY	(name),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(					"()"),

			FUNC_ENTRY_H(addChild,			"(child: cc.Node)\n"
											"(child: cc.Node, zOrder: int)\n"
											"(child: cc.Node, zOrder: int, tag: int)"),
			FUNC_ENTRY_H(removeChild,		"(child: cc.Node, cleanup: bool)"),
			FUNC_ENTRY_H(removeChildByTag,	"(tag: int, cleanup: bool)"),
			FUNC_ENTRY_H(removeAllChildren, "(cleanup: bool)"),
			FUNC_ENTRY_H(removeFromParent,	"(cleanup: bool)"),
			FUNC_ENTRY_H(getChildByTag,		"(tag: int): cc.Node"),
			FUNC_ENTRY_H(reorderChild,		"(child: cc.Node, zOrder: int)"),
			FUNC_ENTRY_H(cleanup,			"()"),

			FUNC_ENTRY_H(findByName,		"(wildcard: string): cc.Node[]"),
			FUNC_ENTRY_H(findByClass,		"(filter=\"*\"): cc.Node[]"),

			FUNC_ENTRY_H(scale,				"(scale: float)"),
			FUNC_ENTRY_H(boundingBox,		"(): cc.Size"),
			FUNC_ENTRY_H(boundingBoxInPixels, "(): cc.Size"),

			FUNC_ENTRY_H(runAction,			"(action: cc.Action): cc.Action"),
			FUNC_ENTRY_H(stopAllActions,	"()"),
			FUNC_ENTRY_H(stopAction,		"(action: cc.Action)"),
			FUNC_ENTRY_H(stopActionByTag,	"(tag: int)"),
			FUNC_ENTRY_H(getActionByTag,	"(tag: int): cc.Action"),

			FUNC_ENTRY_H(nodeToParent,		"(): cc.AffineTransform"),
			FUNC_ENTRY_H(parentToNode,		"(): cc.AffineTransform"),
			FUNC_ENTRY_H(nodeToWorld,		"(): cc.AffineTransform"),
			FUNC_ENTRY_H(worldToNode,		"(): cc.AffineTransform"),

			FUNC_ENTRY_H(toNode,			"(pt: worldPoint): cc.Point"),
			FUNC_ENTRY_H(toWorld,			"(pt: nodePoint): cc.Point"),
			FUNC_ENTRY_H(toNodeAR,			"(pt: worldPoint): cc.Point // anchor relative"),
			FUNC_ENTRY_H(toWorldAR,			"(pt: nodePoint): cc.Point // anchor relative"),

			FUNC_ENTRY_H(touchToNode,		"(touch: cc.Touch): cc.Point"),
			FUNC_ENTRY_H(touchToNodeAR,		"(touch: cc.Touch): cc.Point // anchor relative"),

			FUNC_ENTRY_H(renderVisit,		"(ctx: RenderContext)"),

			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IEventSink>(v);
	}

	NB_PROP_GET(description)			{ return push(v, (const char*) self(v)->description()); }
	NB_PROP_GET(zOrder)					{ return push(v, self(v)->getZOrder()); }
	NB_PROP_GET(vertexZ)				{ return push(v, self(v)->getVertexZ()); }
	NB_PROP_GET(rotation)				{ return push(v, self(v)->getRotation()); }
	NB_PROP_GET(scaleX)					{ return push(v, self(v)->getScaleX()); }
	NB_PROP_GET(scaleY)					{ return push(v, self(v)->getScaleY()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->getPosition()); }
	NB_PROP_GET(positionInPixels)		{ return push(v, self(v)->getPositionInPixels()); }
	NB_PROP_GET(skewX)					{ return push(v, self(v)->getSkewX()); }
	NB_PROP_GET(skewY)					{ return push(v, self(v)->getSkewY()); }
	NB_PROP_GET(camera)					{ return push(v, self(v)->getCamera()); }
	NB_PROP_GET(grid)					{ return push(v, self(v)->getGrid()); }
	NB_PROP_GET(visible)				{ return push(v, self(v)->getIsVisible()); }
	NB_PROP_GET(anchorPoint)			{ return push(v, self(v)->getAnchorPoint()); }
	NB_PROP_GET(anchorPointInPixels)	{ return push(v, self(v)->getAnchorPointInPixels()); }
	NB_PROP_GET(contentSize)			{ return push(v, self(v)->getContentSize()); }
	NB_PROP_GET(contentSizeInPixels)	{ return push(v, self(v)->getContentSizeInPixels()); }
	NB_PROP_GET(running)				{ return push(v, self(v)->getIsRunning()); }
	NB_PROP_GET(parent)					{ return push(v, self(v)->getParent()); }
	NB_PROP_GET(relativeAnchorPoint)	{ return push(v, self(v)->getIsRelativeAnchorPoint()); }
	NB_PROP_GET(tag)					{ return push(v, self(v)->getTag()); }
	NB_PROP_GET(userValue)				{ return ScriptDataValue::push(v, self(v)->getUserValue()); }
	NB_PROP_GET(numRunningActions)		{ return push(v, self(v)->numberOfRunningActions()); }
	NB_PROP_GET(clipActive)				{ return push(v, self(v)->GetIsActiveClipArea()); }
	NB_PROP_GET(scene)					{ return push(v, self(v)->getScene()); }
	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }

	NB_PROP_GET(children)
	{
		CCArray* children = self(v)->getChildren();
		if (!children)
			return 0;

		sq_newarray(v, 0);
		for (uint i=0; i<children->count(); ++i)
		{
			CCObject* child = children->objectAtIndex(i);

			CCLayer* asLayer = dynamic_cast<CCLayer*>(child);
			if (asLayer) 
				arrayAppend(v, -1, asLayer);
			else
				arrayAppend(v, -1, static_cast<CCNode*>(child));
		}
		return 1;
	}

	NB_PROP_SET(vertexZ)				{ self(v)->setVertexZ(getFloat(v, 2)); return 0; }
	NB_PROP_SET(rotation)				{ self(v)->setRotation(getFloat(v, 2)); return 0; }
	NB_PROP_SET(scaleX)					{ self(v)->setScaleX(getFloat(v, 2)); return 0; }
	NB_PROP_SET(scaleY)					{ self(v)->setScaleY(getFloat(v, 2)); return 0; }
	NB_PROP_SET(position)				{ self(v)->setPosition(*get<CCPoint>(v, 2)); return 0; }
	NB_PROP_SET(positionInPixels)		{ self(v)->setPositionInPixels(*get<CCPoint>(v, 2)); return 0; }
	NB_PROP_SET(skewX)					{ self(v)->setSkewX(getFloat(v, 2)); return 0; }
	NB_PROP_SET(skewY)					{ self(v)->setSkewY(getFloat(v, 2)); return 0; }
	NB_PROP_SET(grid)					{ self(v)->setGrid(opt<CCGridBase>(v, 2, NULL)); return 0; }
	NB_PROP_SET(visible)				{ self(v)->setIsVisible(getBool(v, 2)); return 0; }
	NB_PROP_SET(anchorPoint)			{ self(v)->setAnchorPoint(*get<CCPoint>(v, 2)); return 0; }
	NB_PROP_SET(contentSize)			{ self(v)->setContentSize(*get<CCSize>(v, 2)); return 0; }
	NB_PROP_SET(contentSizeInPixels)	{ self(v)->setContentSizeInPixels(*get<CCSize>(v, 2)); return 0; }
	NB_PROP_SET(parent)					{ self(v)->setParent(opt<CCNode>(v, 2, NULL)); return 0; }
	NB_PROP_SET(relativeAnchorPoint)	{ self(v)->setIsRelativeAnchorPoint(getBool(v, 2)); return 0; }
	NB_PROP_SET(tag)					{ self(v)->setTag(getInt(v, 2)); return 0; }
	NB_PROP_SET(clipActive)				{ self(v)->SetActiveClipArea(getBool(v, 2)); return 0; }
	NB_PROP_SET(name)					{ self(v)->setName(getString(v, 2)); return 0; }
	NB_PROP_SET(userValue)				
	{ 
		DataValue value; 
		SQRESULT sr = ScriptDataValue::toValue(v, 2, value); 
		if (SQ_FAILED(sr)) return sr;
		self(v)->setUserValue(value); 
		return 0; 
	}

	NB_CONS()							{ setSelf(v, CCNode::node()); return 0; }

	NB_FUNC(addChild)
	{
		if (isNone(v, 3))
			self(v)->addChild(get<CCNode>(v, 2));
		else if (isNone(v, 4))
			self(v)->addChild(get<CCNode>(v, 2), getInt(v, 3));
		else
			self(v)->addChild(get<CCNode>(v, 2), getInt(v, 3), getInt(v, 4));
		return 0;
	}

	static SQRESULT DoFindByClass(HSQUIRRELVM v, const char* filter, CCNode* node, int outArrayIdx)
	{
		push(v, node);

		if (filter)
		{
			sq_pushstring(v, "_classname", -1);
			sq_get(v, -2);

			const char* str = "";
			sq_getstring(v, -1, &str);

			if (!Wildcard::match(filter, str))
				sq_pop(v, 2);
			else
			{
				sq_poptop(v);
				sq_arrayappend(v, outArrayIdx);
			}
		}
		else
		{
			sq_arrayappend(v, outArrayIdx);
		}

		CCArray* children = node->getChildren();
		if (!children)
			return SQ_OK;

		for (uint i=0; i<children->count(); ++i)
		{
			CCNode* child = static_cast<CCNode*>(children->objectAtIndex(i));

			DoFindByClass(v, filter, child, outArrayIdx);
		}

		return SQ_OK;
	}

	NB_FUNC(findByClass)
	{
		const char* filter = optString(v, 2, NULL);

		sq_newarray(v, 0);
		DoFindByClass(v, filter, self(v), toAbsIdx(v, -1));

		return 1;
	}

	static SQRESULT DoFindByName(HSQUIRRELVM v, const char* filter, CCNode* node, int outArrayIdx)
	{
		if (filter == NULL || Wildcard::match(filter, node->getName()))
		{
			push(v, node);
			sq_arrayappend(v, outArrayIdx);
		}

		CCArray* children = node->getChildren();
		if (!children)
			return SQ_OK;

		for (uint i=0; i<children->count(); ++i)
		{
			CCNode* child = static_cast<CCNode*>(children->objectAtIndex(i));

			DoFindByName(v, filter, child, outArrayIdx);
		}

		return SQ_OK;
	}

	NB_FUNC(findByName)
	{
		const char* filter = getString(v, 2);

		if (strcmp(filter, "*") == 0) filter = NULL;

		sq_newarray(v, 0);
		DoFindByName(v, filter, self(v), toAbsIdx(v, -1));

		return 1;
	}

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		SQInteger top = sq_gettop(v);
		const char* clsname = "???";
		push(v, "_classname");
		sq_get(v, 1);
		sq_getstring(v, -1, &clsname);
		sq_settop(v, top);
		const String& name = o->getName();
		if (!name.empty())
			return pushFmt(v, "(%s: %s -> %08X)", name.c_str(), clsname, o);
		else
			return pushFmt(v, "(%s -> %08X)", clsname, o);
	}

	NB_FUNC(removeChild)				{ self(v)->removeChild(get<CCNode>(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(removeChildByTag)			{ self(v)->removeChildByTag(getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC(removeAllChildren)			{ self(v)->removeAllChildrenWithCleanup(getBool(v, 2)); return 0; }
	NB_FUNC(removeFromParent)			{ self(v)->removeFromParentAndCleanup(getBool(v, 2)); return 0; }
	NB_FUNC(getChildByTag)				{ return push(v, self(v)->getChildByTag(getInt(v, 2))); return 0; }
	NB_FUNC(reorderChild)				{ self(v)->reorderChild(get<CCNode>(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC(cleanup)					{ self(v)->cleanup(); return 0; }
	NB_FUNC(scale)						{ self(v)->setScale(getFloat(v, 2)); return 0; }

	NB_FUNC(boundingBox)				{ return push(v, self(v)->boundingBox()); }
	NB_FUNC(boundingBoxInPixels)		{ return push(v, self(v)->boundingBoxInPixels()); }
	NB_FUNC(runAction)					{ return push(v, self(v)->runAction(get<CCAction>(v, 2))); }
	NB_FUNC(stopAllActions)				{ self(v)->stopAllActions(); return 0; }
	NB_FUNC(stopAction)					{ self(v)->stopAction(get<CCAction>(v, 2)); return 0; }
	NB_FUNC(stopActionByTag)			{ self(v)->stopActionByTag(getInt(v, 2)); return 0; }
	NB_FUNC(getActionByTag)				{ return push(v, self(v)->getActionByTag(getInt(v, 2))); }

	NB_FUNC(nodeToParent)				{ return push(v, self(v)->nodeToParentTransform()); }
	NB_FUNC(parentToNode)				{ return push(v, self(v)->parentToNodeTransform()); }
	NB_FUNC(nodeToWorld)				{ return push(v, self(v)->nodeToWorldTransform()); }
	NB_FUNC(worldToNode)				{ return push(v, self(v)->worldToNodeTransform()); }

	NB_FUNC(toNode)						{ return push(v, self(v)->convertToNodeSpace(*get<CCPoint>(v, 2))); }
	NB_FUNC(toWorld)					{ return push(v, self(v)->convertToWorldSpace(*get<CCPoint>(v, 2))); }
	NB_FUNC(toNodeAR)					{ return push(v, self(v)->convertToNodeSpaceAR(*get<CCPoint>(v, 2))); }
	NB_FUNC(toWorldAR)					{ return push(v, self(v)->convertToWorldSpaceAR(*get<CCPoint>(v, 2))); }

	NB_FUNC(touchToNode)				{ return push(v, self(v)->convertTouchToNodeSpace(get<CCTouch>(v, 2))); }
	NB_FUNC(touchToNodeAR)				{ return push(v, self(v)->convertTouchToNodeSpaceAR(get<CCTouch>(v, 2))); }

	NB_FUNC(renderVisit)				{ self(v)->visit(get<RenderContext>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCGridBase, GridBase);
NB_TYPE_CC_REF(NIT2D_API, cc::GridBase, CCObject);

class NB_CCGridBase : TNitClass<CCGridBase>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(active),
			PROP_ENTRY	(resultGrid),
			PROP_ENTRY	(gridSize),
			PROP_ENTRY	(step),
			PROP_ENTRY	(textureFlipped),

			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(active)					{ return push(v, self(v)->isActive()); }
	NB_PROP_GET(resultGrid)				{ return push(v, self(v)->getReuseGrid()); }
	NB_PROP_GET(gridSize)				{ return push(v, self(v)->getGridSize()); }
	NB_PROP_GET(step)					{ return push(v, self(v)->getStep()); }
	NB_PROP_GET(textureFlipped)			{ return push(v, self(v)->isTextureFlipped()); }

	NB_PROP_SET(active)					{ self(v)->setActive(getBool(v, 2)); return 0; }
	NB_PROP_SET(resultGrid)				{ self(v)->setReuseGrid(getInt(v, 2)); return 0; }
	NB_PROP_SET(gridSize)				{ self(v)->setGridSize(*get<ccGridSize>(v, 2)); return 0; }
	NB_PROP_SET(step)					{ self(v)->setStep(*get<CCPoint>(v, 2)); return 0; }
	NB_PROP_SET(textureFlipped)			{ self(v)->setIsTextureFlipped(getBool(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCGrid3D, Grid3D);
NB_TYPE_CC_REF(NIT2D_API, cc::Grid3D, CCGridBase);

class NB_CCGrid3D : TNitClass<CCGrid3D>
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
			CONS_ENTRY_H(				"(gridSize: cc.GridSize)\n"
										"(gridSize: cc.GridSize, tex: cc.Texture, flip: bool)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 3))
			setSelf(v, CCGrid3D::gridWithSize(*get<ccGridSize>(v, 2)));
		else
			setSelf(v, CCGrid3D::gridWithSize(*get<ccGridSize>(v, 2), get<CCTexture2D>(v, 3), getBool(v, 4)));

		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTiledGrid3D, TiledGrid3D);
NB_TYPE_CC_REF(NIT2D_API, cc::TiledGrid3D, CCGridBase);

class NB_CCTiledGrid3D : TNitClass<CCTiledGrid3D>
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
			CONS_ENTRY_H(				"(gridSize: cc.GridSize)\n"
										"(gridSize: cc.GridSize, tex: cc.Texture, flip: bool)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 3))
			setSelf(v, CCTiledGrid3D::gridWithSize(*get<ccGridSize>(v, 2)));
		else
			setSelf(v, CCTiledGrid3D::gridWithSize(*get<ccGridSize>(v, 2), get<CCTexture2D>(v, 3), getBool(v, 4)));

		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCCamera, Camera);
NB_TYPE_CC_REF(NIT2D_API, cc::Camera, CCObject);

class NB_CCCamera : TNitClass<CCCamera>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(dirty),
			PROP_ENTRY	(eye),
			PROP_ENTRY	(center),
			PROP_ENTRY	(up),
			PROP_ENTRY_R(description),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(restore,		"()"),
			FUNC_ENTRY_H(locate,		"(ctx: RenderContext)"),
			FUNC_ENTRY_H(getZEye,		"[class] (): nit.Vector3"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(dirty)					{ return push(v, self(v)->getDirty()); }
	NB_PROP_GET(eye)					{ Vector3 vec; self(v)->getEyeXYZ(&vec.x, &vec.y, &vec.z); return push(v, vec); }
	NB_PROP_GET(center)					{ Vector3 vec; self(v)->getCenterXYZ(&vec.x, &vec.y, &vec.z); return push(v, vec); }
	NB_PROP_GET(up)						{ Vector3 vec; self(v)->getUpXYZ(&vec.x, &vec.y, &vec.z); return push(v, vec); }
	NB_PROP_GET(description)			{ return push(v, (const char*)self(v)->description()); }

	NB_PROP_SET(dirty)					{ self(v)->setDirty(getBool(v, 2)); return 0; }
	NB_PROP_SET(eye)					{ Vector3* vec = get<Vector3>(v, 2); self(v)->setEyeXYZ(vec->x, vec->y, vec->z); return 0; }
	NB_PROP_SET(center)					{ Vector3* vec = get<Vector3>(v, 2); self(v)->setCenterXYZ(vec->x, vec->y, vec->z); return 0; }
	NB_PROP_SET(up)						{ Vector3* vec = get<Vector3>(v, 2); self(v)->setUpXYZ(vec->x, vec->y, vec->z); return 0; }

	NB_FUNC(restore)					{ self(v)->restore(); return 0; }
	NB_FUNC(locate)						{ self(v)->locate(get<RenderContext>(v, 2)); return 0; }
	NB_FUNC(getZEye)					{ return push(v, CCCamera::getZEye()); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTouch, Touch);
NB_TYPE_CC_REF(NIT2D_API, cc::Touch, CCObject);

class NB_CCTouch : TNitClass<CCTouch>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(view),
			PROP_ENTRY_R(point),
			PROP_ENTRY_R(prevPoint),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(							"(viewID: int, x, y: float)"),
			FUNC_ENTRY_H(locationInView,			"(viewID: int): cc.Point"),
			FUNC_ENTRY_H(previousLocationInView,	"(viewID: int): cc.Point"),
			FUNC_ENTRY_H(setTouchInfo,				"(viewID: int, x, y: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(view)					{ return push(v, self(v)->view()); }
	NB_PROP_GET(point)					{ return push(v, self(v)->locationInView(self(v)->view())); }
	NB_PROP_GET(prevPoint)				{ return push(v, self(v)->previousLocationInView(self(v)->view())); }
	
	NB_CONS()							{ setSelf(v, new CCTouch(getInt(v, 2), getFloat(v, 3), getFloat(v, 4))); return 0; }

	NB_FUNC(locationInView)				{ return push(v, self(v)->locationInView(getInt(v, 2))); }
	NB_FUNC(previousLocationInView)		{ return push(v, self(v)->previousLocationInView(getInt(v, 2))); }
	NB_FUNC(setTouchInfo)				{ self(v)->SetTouchInfo(getInt(v, 2), getFloat(v, 3), getFloat(v, 4)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCDirector, Director);
NB_TYPE_WEAK(NIT2D_API, cc::Director, NULL);

class NB_CCDirector : TNitClass<CCDirector>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(runningScene),
			PROP_ENTRY	(displayFPS),
			PROP_ENTRY_R(renderView),
			PROP_ENTRY	(nextDeltaTimeZero),
			PROP_ENTRY_R(paused),
			PROP_ENTRY_R(frames),
			PROP_ENTRY	(projection),
			PROP_ENTRY_R(sendCleanupToScene),
			PROP_ENTRY_R(winSize),
			PROP_ENTRY_R(winSizeInPixels),
			PROP_ENTRY_R(displaySizeInPixels),
			PROP_ENTRY_R(zEye),
			PROP_ENTRY	(deviceOrientation),
			PROP_ENTRY	(contentScaleFactor),
			PROP_ENTRY_R(timer),
			PROP_ENTRY_R(scheduler),
			PROP_ENTRY	(debugBound),
			PROP_ENTRY	(debugSprite),
			PROP_ENTRY	(debugSpriteBatch),
			PROP_ENTRY	(debugLabel),
			PROP_ENTRY	(debugClip),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(toGl,			"(uiPoint: cc.Point): cc.Point"),
			FUNC_ENTRY_H(toUi,			"(glPoint: cc.Point): cc.Point"),
			FUNC_ENTRY_H(runWithScene,	"(scene: cc.Scene)"),
			FUNC_ENTRY_H(pushScene,		"(scene: cc.Scene)"),
			FUNC_ENTRY_H(popScene,		"()"),
			FUNC_ENTRY_H(replaceScene,	"(scene: cc.Scene)"),
			FUNC_ENTRY_H(pause,			"()"),
			FUNC_ENTRY_H(resume,		"()"),
			FUNC_ENTRY_H(stopAnimation,	"()"),
			FUNC_ENTRY_H(startAnimation, "()"),
			FUNC_ENTRY_H(purgeCachedData, "()"),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "PROJ_2D",				(int)kCCDirectorProjection2D);
		addStatic(v, "PROJ_3D",				(int)kCCDirectorProjection3D);
		addStatic(v, "PROJ_CUSTOM",			(int)kCCDirectorProjectionCustom);
		addStatic(v, "PROJ_DEFAULT",		(int)kCCDirectorProjectionDefault);

		addStatic(v, "ORIENT_PORTRAIT_UP",		(int)kCCDeviceOrientationPortrait);
		addStatic(v, "ORIENT_PORTRAIT_DOWN",	(int)kCCDeviceOrientationPortraitUpsideDown);
		addStatic(v, "ORIENT_LANDSCAPE_LEFT",	(int)kCCDeviceOrientationLandscapeLeft);
		addStatic(v, "ORIENT_LANDSCAPE_RIGHT",	(int)kCCDeviceOrientationLandscapeRight);
	}

	NB_PROP_GET(runningScene)			{ return push(v, self(v)->getRunningScene()); }
	NB_PROP_GET(displayFPS)				{ return push(v, self(v)->isDisplayFPS()); }
	NB_PROP_GET(renderView)				{ return push(v, self(v)->GetRenderView()); }
	NB_PROP_GET(nextDeltaTimeZero)		{ return push(v, self(v)->isNextDeltaTimeZero()); }
	NB_PROP_GET(paused)					{ return push(v, self(v)->isPaused()); }
	NB_PROP_GET(frames)					{ return push(v, self(v)->getFrames()); }
	NB_PROP_GET(projection)				{ return push(v, (int)self(v)->getProjection()); }
	NB_PROP_GET(sendCleanupToScene)		{ return push(v, self(v)->isSendCleanupToScene()); }
	NB_PROP_GET(winSize)				{ return push(v, self(v)->getWinSize()); }
	NB_PROP_GET(winSizeInPixels)		{ return push(v, self(v)->getWinSizeInPixels()); }
	NB_PROP_GET(displaySizeInPixels)	{ return push(v, self(v)->getDisplaySizeInPixels()); }
	NB_PROP_GET(zEye)					{ return push(v, self(v)->getZEye()); }
	NB_PROP_GET(deviceOrientation)		{ return push(v, (int)self(v)->getDeviceOrientation()); }
	NB_PROP_GET(contentScaleFactor)		{ return push(v, self(v)->getContentScaleFactor()); }
	NB_PROP_GET(timer)					{ return push(v, self(v)->getTimer()); }
	NB_PROP_GET(scheduler)				{ return push(v, self(v)->getScheduler()); }
	NB_PROP_GET(debugBound)				{ return push(v, self(v)->GetDebugBound()); }
	NB_PROP_GET(debugSprite)			{ return push(v, self(v)->GetDebugSprite()); }
	NB_PROP_GET(debugSpriteBatch)		{ return push(v, self(v)->GetDebugSpriteBatch()); }
	NB_PROP_GET(debugLabel)				{ return push(v, self(v)->GetDebugLabel()); }
	NB_PROP_GET(debugClip)				{ return push(v, self(v)->GetDebugClip()); }

	NB_PROP_SET(displayFPS)				{ self(v)->setDisplayFPS(getBool(v, 2)); return 0; }
	NB_PROP_SET(nextDeltaTimeZero)		{ self(v)->setNextDeltaTimeZero(getBool(v, 2)); return 0; }
	NB_PROP_SET(projection)				{ self(v)->setProjection((ccDirectorProjection)getInt(v, 2)); return 0; }
	NB_PROP_SET(deviceOrientation)		{ self(v)->setDeviceOrientation((ccDeviceOrientation)getInt(v, 2)); return 0; }
	NB_PROP_SET(contentScaleFactor)		{ self(v)->setContentScaleFactor(getFloat(v, 2)); return 0; }
	NB_PROP_SET(debugBound)				{ self(v)->SetDebugBound(getBool(v, 2)); return 0; }
	NB_PROP_SET(debugSprite)			{ self(v)->SetDebugSprite(getBool(v, 2)); return 0; }
	NB_PROP_SET(debugSpriteBatch)		{ self(v)->SetDebugSpriteBatch(getBool(v, 2)); return 0; }
	NB_PROP_SET(debugLabel)				{ self(v)->SetDebugLabel(getBool(v, 2)); return 0; }
	NB_PROP_SET(debugClip)				{ self(v)->SetDebugClip(getBool(v, 2)); return 0; }

	NB_FUNC(toGl)						{ return push(v, self(v)->convertToGL(*get<CCPoint>(v, 2)));}
	NB_FUNC(toUi)						{ return push(v, self(v)->convertToUI(*get<CCPoint>(v, 2))); }
	NB_FUNC(runWithScene)				{ self(v)->runWithScene(get<CCScene>(v, 2)); return 0; }
	NB_FUNC(pushScene)					{ self(v)->pushScene(get<CCScene>(v, 2)); return 0; }
	NB_FUNC(popScene)					{ self(v)->popScene(); return 0; }
	NB_FUNC(replaceScene)				{ self(v)->replaceScene(get<CCScene>(v, 2)); return 0; }
	NB_FUNC(pause)						{ self(v)->pause(); return 0; }
	NB_FUNC(resume)						{ self(v)->resume(); return 0; }
	NB_FUNC(stopAnimation)				{ self(v)->stopAnimation(); return 0; }
	NB_FUNC(startAnimation)				{ self(v)->startAnimation(); return 0; }
	NB_FUNC(purgeCachedData)			{ self(v)->purgeCachedData(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCScene, Scene);
NB_TYPE_CC_REF(NIT2D_API, cc::Scene, CCNode);

class NB_CCScene : TNitClass<CCScene>
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

	NB_CONS()							{ setSelf(v, CCScene::node()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCLayer, Layer);
NB_TYPE_CC_REF(NIT2D_API, cc::Layer, CCNode);

class NB_CCLayer : TNitClass<CCLayer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(touchEnabled),
			PROP_ENTRY	(keypadEnabled),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			FUNC_ENTRY_H(setModal,		"(flag: bool, recursive=true)"),
			NULL
		};

		bind(v, props, funcs);

		addInterface<CCTouchDelegate>(v);
	}

	NB_PROP_GET(touchEnabled)			{ return push(v, self(v)->getIsTouchEnabled()); }
	NB_PROP_GET(keypadEnabled)			{ return push(v, self(v)->getIsKeypadEnabled()); }

	NB_PROP_SET(touchEnabled)			{ self(v)->setIsTouchEnabled(getBool(v, 2)); return 0; }
	NB_PROP_SET(keypadEnabled)			{ self(v)->setIsKeypadEnabled(getBool(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCLayer::node()); return 0; }

	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }

	NB_FUNC(setModal)					{ self(v)->setModal(getBool(v, 2), optBool(v, 3, true)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCLayerColor, LayerColor);
NB_TYPE_CC_REF(NIT2D_API, cc::LayerColor, CCLayer);

class NB_CCLayerColor : TNitClass<CCLayerColor>
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
										"(color: Color)\n"
										"(color: Color, width, height: float)\n"),
			FUNC_ENTRY_H(changeWidth,	"(w: float)"),
			FUNC_ENTRY_H(changeHeight,	"(h: float)"),
			FUNC_ENTRY_H(changeWidthAndHeight, "(w, h: float)"),
			NULL
		};

		bind(v, props, funcs);

		NB_CCRGBAProtocol::Register<type>(v);
		NB_CCBlendProtocol::Register<type>(v);
	}

	NB_CONS()
	{
		if (isNone(v, 2))
			setSelf(v, CCLayerColor::node()); 
		else if (isNone(v, 3))
			setSelf(v, CCLayerColor::layerWithColor(ToColor4B(*get<Color>(v, 2))));
		else
			setSelf(v, CCLayerColor::layerWithColorWidthHeight(ToColor4B(*get<Color>(v, 2)), getFloat(v, 3), getFloat(v, 4)));
		return 0; 
	}

	NB_FUNC(changeWidth)				{ self(v)->changeWidth(getFloat(v, 2)); return 0; }
	NB_FUNC(changeHeight)				{ self(v)->changeHeight(getFloat(v, 2)); return 0; }
	NB_FUNC(changeWidthAndHeight)		{ self(v)->changeWidthAndHeight(getFloat(v, 2), getFloat(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCLayerGradient, LayerGradient);
NB_TYPE_CC_REF(NIT2D_API, cc::LayerGradient, CCLayerColor);

class NB_CCLayerGradient : TNitClass<CCLayerGradient>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(startColor),
			PROP_ENTRY	(endColor),
			PROP_ENTRY	(vector),
			PROP_ENTRY	(compressedInterpolation),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()\n"
										"(start, end: Color)\n"
										"(start, end: Color, vector: cc.Point)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(startColor)				{ return push(v, FromColor3B(self(v)->getStartColor(), self(v)->getStartOpacity())); }
	NB_PROP_GET(endColor)				{ return push(v, FromColor3B(self(v)->getEndColor(), self(v)->getEndOpacity())); }
	NB_PROP_GET(vector)					{ return push(v, self(v)->getVector()); }
	NB_PROP_GET(compressedInterpolation){ return push(v, self(v)->getIsCompressedInterpolation()); }

	NB_PROP_SET(startColor)				{ Color& c = *get<Color>(v, 2); self(v)->setStartColor(ToColor3B(c)); self(v)->setStartOpacity(GLubyte(c.a * 255)); return 0; }
	NB_PROP_SET(endColor)				{ Color& c = *get<Color>(v, 2); self(v)->setEndColor(ToColor3B(c)); self(v)->setEndOpacity(GLubyte(c.a * 255)); return 0; }
	NB_PROP_SET(vector)					{ self(v)->setVector(*get<CCPoint>(v, 2)); return 0; }
	NB_PROP_SET(compressedInterpolation){ self(v)->setIsCompressedInterpolation(getBool(v, 2)); return 0; }

	NB_CONS()							
	{ 
		if (isNone(v, 2))
			setSelf(v, CCLayerGradient::node()); 
		else if (isNone(v, 4))
			setSelf(v, CCLayerGradient::layerWithColor(ToColor4B(*get<Color>(v, 2)), ToColor4B(*get<Color>(v, 3))));
		else
			setSelf(v, CCLayerGradient::layerWithColor(ToColor4B(*get<Color>(v, 2)), ToColor4B(*get<Color>(v, 3)), *get<CCPoint>(v, 4)));
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCLayerMultiplex, LayerMultiplex);
NB_TYPE_CC_REF(NIT2D_API, cc::LayerMultiplex, CCLayer);

class NB_CCLayerMultiplex : TNitClass<CCLayerMultiplex>
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
			CONS_ENTRY_H(				"(layer1, ...: cc.Layer)"),
			FUNC_ENTRY_H(addLayer,		"(layer: cc.Layer)"),
			FUNC_ENTRY_H(switchTo,		"(n: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(addLayer)					{ self(v)->addLayer(get<CCLayer>(v, 2)); return 0; }
	NB_FUNC(switchTo)					{ self(v)->switchTo(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		int top = sq_gettop(v);

		// Check all parameters in advance
		for (int i = 2; i <= top; ++i)
			if (!is<CCLayer>(v, i))
				return sq_throwerror(v, "cc.Layer expected");

		CCLayer* l1 = get<CCLayer>(v, 2);

		CCLayerMultiplex* self = CCLayerMultiplex::layerWithLayer(l1);

		for (int i = 3; i <= top; ++i)
		{
			CCLayer* l = get<CCLayer>(v, i);
			self->addLayer(l);
		}

		setSelf(v, self);
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCSprite, Sprite);
NB_TYPE_CC_REF(NIT2D_API, cc::Sprite, CCNode);

class NB_CCSprite : TNitClass<CCSprite>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(dirty),
			PROP_ENTRY_R(textureRectRotated),
			PROP_ENTRY	(atlasIndex),
			PROP_ENTRY	(textureRect),
			PROP_ENTRY	(usesBatchNode),
			PROP_ENTRY	(textureAtlas),
			PROP_ENTRY	(spriteBatchNode),
			PROP_ENTRY	(honorParentTransform),
			PROP_ENTRY  (flipX),
			PROP_ENTRY  (flipY),
			PROP_ENTRY_R(offsetPositionPixels),
			PROP_ENTRY	(displayedFrame),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"
										"(tex: cc.Texture2D)\n"
										"(frame: cc.SpriteFrame)\n"
										"(filename: string)\n"
										"(source: StreamSource)\n"
										"(tex: cc.Texture2D, rect: cc.Rect)\n"
										"(source: StreamSource, rect: cc.Rect)\n"
										"(batch: cc.SpriteBatchNode, rect: cc.Rect)\n"
										"(tex: cc.Texture2D, rect: cc.Rect, offset: cc.Point)\n"),
			FUNC_ENTRY_H(isFrameDisplayed, "(frame: cc.SpriteFrame): bool"),
			FUNC_ENTRY_H(setDisplayFrameWithAnimName, "(animname: string, idx: int)"),
			FUNC_ENTRY_H(useBatchNode,	"(batch: SpriteBatchNode)"),
			FUNC_ENTRY_H(setTextureRectInPixels, "(rect: cc.Rect, rotated: bool, size: cc.Size)"),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "HONOR_TRANSLATE", (int)CC_HONOR_PARENT_TRANSFORM_TRANSLATE);
		addStatic(v, "HONOR_ROTATE",	(int)CC_HONOR_PARENT_TRANSFORM_ROTATE);
		addStatic(v, "HONOR_SCALE",		(int)CC_HONOR_PARENT_TRANSFORM_SCALE);
		addStatic(v, "HONOR_SKEW",		(int)CC_HONOR_PARENT_TRANSFORM_SKEW);
		addStatic(v, "HONOR_ALL",		(int)CC_HONOR_PARENT_TRANSFORM_ALL);

		NB_CCRGBAProtocol::Register<type>(v);
		NB_CCTextureProtocol::Register<type>(v);
		NB_CCBlendProtocol::Register<type>(v);
	}

	NB_PROP_GET(dirty)					{ return push(v, self(v)->isDirty()); }
	NB_PROP_GET(textureRectRotated)		{ return push(v, self(v)->isTextureRectRotated()); }
	NB_PROP_GET(atlasIndex)				{ return push(v, self(v)->getAtlasIndex()); }
	NB_PROP_GET(textureRect)			{ return push(v, self(v)->getTextureRect()); }
	NB_PROP_GET(usesBatchNode)			{ return push(v, self(v)->isUsesBatchNode()); }
	NB_PROP_GET(textureAtlas)			{ return push(v, self(v)->getTextureAtlas()); }
	NB_PROP_GET(spriteBatchNode)		{ return push(v, self(v)->getSpriteBatchNode()); }
	NB_PROP_GET(honorParentTransform)	{ return push(v, (int)self(v)->getHornorParentTransform()); }
	NB_PROP_GET(offsetPositionPixels)	{ return push(v, self(v)->getOffsetPositionInPixels()); }
	NB_PROP_GET(flipX)					{ return push(v, self(v)->isFlipX()); }
	NB_PROP_GET(flipY)					{ return push(v, self(v)->isFlipY()); }
	NB_PROP_GET(displayedFrame)			{ return push(v, self(v)->displayedFrame()); }

	NB_PROP_SET(dirty)					{ self(v)->setDirty(getBool(v, 2)); return 0; }
	NB_PROP_SET(atlasIndex)				{ self(v)->setAtlasIndex((uint)getInt(v, 2)); return 0; }
	NB_PROP_SET(textureRect)			{ self(v)->setTextureRect(*get<CCRect>(v, 2)); return 0; }
	NB_PROP_SET(usesBatchNode)			{ self(v)->setUsesSpriteBatchNode(getBool(v, 2)); return 0; }
	NB_PROP_SET(textureAtlas)			{ self(v)->setTextureAtlas(get<CCTextureAtlas>(v, 2)); return 0; }
	NB_PROP_SET(spriteBatchNode)		{ self(v)->setSpriteBatchNode(get<CCSpriteBatchNode>(v, 2)); return 0; }
	NB_PROP_SET(flipX)					{ self(v)->setFlipX(getBool(v, 2)); return 0; }
	NB_PROP_SET(flipY)					{ self(v)->setFlipY(getBool(v, 2)); return 0; }
	NB_PROP_SET(honorParentTransform)	{ self(v)->setHornorParentTransform((ccHonorParentTransform)getInt(v, 2)); return 0; }
	NB_PROP_SET(displayedFrame)			{ self(v)->setDisplayFrame(get<CCSpriteFrame>(v, 2)); return 0; }

	NB_CONS()
	{
		SQInteger nargs = sq_gettop(v);

		CCSprite* sprite = NULL;
		switch (nargs)
		{
		case 1:
			setSelf(v, CCSprite::sprite());
			break;

		case 2:
			if (is<CCTexture2D>(v, 2))
				setSelf(v, CCSprite::spriteWithTexture(get<CCTexture2D>(v, 2)));
			else if (is<CCSpriteFrame>(v, 2))
				setSelf(v, CCSprite::spriteWithSpriteFrame(get<CCSpriteFrame>(v, 2)));
			else if (is<StreamSource>(v, 2))
				setSelf(v, CCSprite::spriteWithFile(get<StreamSource>(v, 2)));	
			else
				return sq_throwerror(v, "invalid argument");
			break;

		case 3:
			if (is<CCTexture2D>(v, 2))
				setSelf(v, CCSprite::spriteWithTexture(get<CCTexture2D>(v, 2), *get<CCRect>(v, 3)));
			else if (is<StreamSource>(v, 2))
				setSelf(v, CCSprite::spriteWithFile(get<StreamSource>(v, 2), *get<CCRect>(v, 3)));
			else if (is<CCSpriteBatchNode>(v, 2))
				setSelf(v, CCSprite::spriteWithBatchNode(get<CCSpriteBatchNode>(v, 2), *get<CCRect>(v, 3)));
			else
				return sq_throwerror(v, "invalid argument");
			break;

		case 4:
			setSelf(v, CCSprite::spriteWithTexture(get<CCTexture2D>(v, 2), *get<CCRect>(v, 3), *get<CCPoint>(v, 4)));
			break;

		default:
			return sq_throwerror(v, "invalid argument");
		}

		return 0;
	}
		
	NB_FUNC	(isFrameDisplayed)			{ return push(v, self(v)->isFrameDisplayed(get<CCSpriteFrame>(v, 2))); }
	NB_FUNC	(setDisplayFrameWithAnimName) { self(v)->setDisplayFrameWithAnimationName(getString(v, 2), getInt(v, 3)); return 0; }
	NB_FUNC	(useBatchNode)				{ self(v)->useBatchNode(get<CCSpriteBatchNode>(v, 2)); return 0; }
	NB_FUNC	(setTextureRectInPixels)	{ self(v)->setTextureRectInPixels(*get<CCRect>(v, 2), getBool(v, 3), *get<CCSize>(v, 4)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCSpriteBatchNode, SpriteBatchNode);
NB_TYPE_CC_REF(NIT2D_API, cc::SpriteBatchNode, CCNode);

class NB_CCSpriteBatchNode : TNitClass<CCSpriteBatchNode>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(textureAtlas),	
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(tex: cc.Texture2D)\n"
										"(source: StreamSource)\n"
										"(tex: cc.Texture2D, capacity: uint)\n"
										"(source: StreamSource, capacity: uint)"),

			FUNC_ENTRY_H(getDescendants,"(): array"),
			FUNC_ENTRY_H(increaseAtlasCapacity,"()"),
			FUNC_ENTRY_H(removeChildAtIndex, "(idx: uint, clean: bool)"),
			FUNC_ENTRY_H(insertChild,	"(child: cc.Sprite, idx: uint)"),
			FUNC_ENTRY_H(removeSpriteFromAtlas, "(sprite: cc.Sprite)"),
			FUNC_ENTRY_H(rebuildIndexInOrder, "(parent: cc.Sprite, idx: uint): uint"),
			FUNC_ENTRY_H(highestAtlasIndexInChild, "(parent: cc.Sprite): uint"),
			FUNC_ENTRY_H(lowestAtlasIndexInChild, "(parent: cc.Sprite): uint"),
			FUNC_ENTRY_H(atlasIndexForChild, "(parent: cc.Sprite, z: int): uint"),
			NULL
		};

		bind(v, props, funcs);

		NB_CCTextureProtocol::Register<type>(v);
	}

	NB_PROP_GET(textureAtlas)			{ return push(v, self(v)->getTextureAtlas()); }
	NB_PROP_SET(textureAtlas)			{ self(v)->setTextureAtlas(get<CCTextureAtlas>(v, 2)); return 0; }

	NB_CONS()
	{
		SQInteger nargs = sq_gettop(v);

		switch (nargs)
		{
		case 2:
			if (is<CCTexture2D>(v, 2))
				setSelf(v, CCSpriteBatchNode::batchNodeWithTexture(get<CCTexture2D>(v, 2)));	
			else if (is<StreamSource>(v, 2))
				setSelf(v, CCSpriteBatchNode::batchNodeWithFile(get<StreamSource>(v, 2)));
			else	
				return sq_throwerror(v, "invalid argument");
			break;
		
		case 3:
			if (is<CCTexture2D>(v, 2))
				setSelf(v, CCSpriteBatchNode::batchNodeWithTexture(get<CCTexture2D>(v, 2), (uint)getInt(v, 3)));	
			else if (is<StreamSource>(v, 2))
				setSelf(v, CCSpriteBatchNode::batchNodeWithFile(get<StreamSource>(v, 2), (uint)getInt(v, 3)));	
			else
				return sq_throwerror(v, "invalid argument");
			break;

		default:
			return sq_throwerror(v, "invalid argument");
		}

		return 0;
	}

	NB_FUNC	(getDescendants)
	{
		CCArray* list = self(v)->getDescendants();

		sq_newarray(v, 0);
		for (uint i = 0; i < list->count(); ++i)
			arrayAppend(v, -1, list->objectAtIndex(i));

		return 1;
	}

	NB_FUNC	(increaseAtlasCapacity)		{ self(v)->increaseAtlasCapacity(); return 0; }
	NB_FUNC	(removeChildAtIndex)		{ self(v)->removeChildAtIndex((uint)getInt(v, 2), getBool(v, 3)); return 0; }
	NB_FUNC	(insertChild)				{ self(v)->insertChild(get<CCSprite>(v, 2), (uint)getInt(v, 3)); return 0; }
	NB_FUNC	(removeSpriteFromAtlas)		{ self(v)->removeSpriteFromAtlas(get<CCSprite>(v, 2)); return 0; }
	NB_FUNC	(rebuildIndexInOrder)		{ return push(v, self(v)->rebuildIndexInOrder(get<CCSprite>(v, 2), (uint)getInt(v, 3))); }
	NB_FUNC	(highestAtlasIndexInChild)	{ return push(v, self(v)->highestAtlasIndexInChild(get<CCSprite>(v, 2))); }
	NB_FUNC	(lowestAtlasIndexInChild)	{ return push(v, self(v)->lowestAtlasIndexInChild(get<CCSprite>(v, 2))); }
	NB_FUNC	(atlasIndexForChild)		{ return push(v, self(v)->atlasIndexForChild(get<CCSprite>(v, 2), getInt(v, 3))); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCSpriteFrame, SpriteFrame);
NB_TYPE_CC_REF(NIT2D_API, cc::SpriteFrame, CCObject);

class NB_CCSpriteFrame : TNitClass<CCSpriteFrame>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(rect),
			PROP_ENTRY	(rotated),
			PROP_ENTRY	(offset),
			PROP_ENTRY	(originalSize),
			PROP_ENTRY	(texture),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(pobtex: cc.Texture2D, rect: cc.Rect)\n"
										"(pobtex: cc.Texture2D, rect: cc.Rect, rotated: bool, offset: cc.Point, origSize: cc.Size)\n"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(rotated)				{ return push(v, self(v)->isRotated()); }
	NB_PROP_GET(rect)					{ return push(v, self(v)->getRect()); }
	NB_PROP_GET(offset)					{ return push(v, self(v)->getOffset()); }
	NB_PROP_GET(originalSize)			{ return push(v, self(v)->getOriginalSize()); }
	NB_PROP_GET(texture)				{ return push(v, self(v)->getTexture()); }

	NB_PROP_SET(rotated)				{ self(v)->setRotated(getBool(v, 2)); return 0; }
	NB_PROP_SET(rect)					{ self(v)->setRect(*get<CCRect>(v, 2)); return 0; }
	NB_PROP_SET(offset)					{ self(v)->setOffset(*get<CCPoint>(v, 2)); return 0; }
	NB_PROP_SET(originalSize)			{ self(v)->setOriginalSize(*get<CCSize>(v, 2)); return 0; }
	NB_PROP_SET(texture)				{ self(v)->setTexture(get<CCTexture2D>(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 4))
			setSelf(v, CCSpriteFrame::frameWithTexture(get<CCTexture2D>(v, 2), *get<CCRect>(v, 3)));
		else
			setSelf(v, CCSpriteFrame::frameWithTexture(get<CCTexture2D>(v, 2), *get<CCRect>(v, 3), getBool(v, 4), 
														*get<CCPoint>(v, 5), *get<CCSize>(v, 6)));

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCSpriteFrameCache, SpriteFrameCache);
NB_TYPE_WEAK(NIT2D_API, cc::SpriteFrameCache, NULL);

class NB_CCSpriteFrameCache : TNitClass<CCSpriteFrameCache>
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
			FUNC_ENTRY_H(addSpriteFramesWithFile,	"(source: StreamSource)\n"
													"(source: StreamSource, texfile: string)\n"
													"(source: StreamSource, tex: cc.Texture2D)"),
			FUNC_ENTRY_H(addSpriteFrame, "(frame: CCSpriteFrame, texfile: string)"),
			FUNC_ENTRY_H(removeSpriteFrames, "()"),
			FUNC_ENTRY_H(removeUnusedSpriteFrames, "()"),
			FUNC_ENTRY_H(removeSpriteFramesFromName, "(name: string)"),
			FUNC_ENTRY_H(removeSpriteFramesFromFile, "(source: StreamSource)"),
			FUNC_ENTRY_H(removeSpriteFramesFromTexture, "(tex: cc.Texture2D)"),
			FUNC_ENTRY_H(spriteFrameByName, "(name: string): cc.SpriteFrame"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC	(addSpriteFramesWithFile)
	{
		uint nargs = sq_gettop(v);

		switch (nargs)
		{
		case 2:
			self(v)->addSpriteFramesWithFile(get<StreamSource>(v, 2));
			break;

		case 3:
			if (isString(v, 3))
				self(v)->addSpriteFramesWithFile(get<StreamSource>(v, 2), getString(v, 3));
			else if(is<CCTexture2D>(v, 3))
				self(v)->addSpriteFramesWithFile(get<StreamSource>(v, 2), get<CCTexture2D>(v, 3));
			else
				return sq_throwerror(v, "invalid argument");

			break;

		default:
			return sq_throwerror(v, "invalid argument");
		}

		return 0; 
	}

	NB_FUNC(addSpriteFrame)				{ self(v)->addSpriteFrame(get<CCSpriteFrame>(v, 2), getString(v, 3)); return 0; }
	NB_FUNC(removeSpriteFrames)			{ self(v)->removeSpriteFrames(); return 0; }
	NB_FUNC(removeUnusedSpriteFrames)	{ self(v)->removeUnusedSpriteFrames(); return 0; }
	NB_FUNC(removeSpriteFramesFromName) { self(v)->removeSpriteFrameByName(getString(v, 2)); return 0; }
	NB_FUNC(removeSpriteFramesFromFile) { self(v)->removeSpriteFramesFromFile(get<StreamSource>(v, 2)); return 0; }
	NB_FUNC(removeSpriteFramesFromTexture) { self(v)->removeSpriteFramesFromTexture(get<CCTexture2D>(v, 2)); return 0; }
	NB_FUNC(spriteFrameByName)			{ return push(v, self(v)->spriteFrameByName(getString(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCAnimation, Animation);
NB_TYPE_CC_REF(NIT2D_API, cc::Animation, CCObject);

class NB_CCAnimation : TNitClass<CCAnimation>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(name),
			PROP_ENTRY	(delay),
			PROP_ENTRY_R(frames),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY_H(addFrame,		"(frame: cc.SpriteFrame)"),
			FUNC_ENTRY_H(addFrameWithFileName, "(frame: cc.StreamSource)"),
			FUNC_ENTRY_H(addFrameWithTexture, "(tex: cc.Texture2D, rect: cc.Rect)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(delay)					{ return push(v, self(v)->getDelay()); }
	NB_PROP_GET(frames)				
	{
		CCMutableArray<CCSpriteFrame*>* list = self(v)->getFrames();

		sq_newarray(v, 0);

		for (uint i = 0; i < list->count(); ++i)
			arrayAppend(v, -1, list->getObjectAtIndex(i));

		return 1;
	}

	NB_PROP_SET(name)					{ self(v)->setName(getString(v, 2)); return 0; }
	NB_PROP_SET(delay)					{ self(v)->setDelay(getFloat(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCAnimation::animation()); return 0; }

	NB_FUNC(addFrame)					{ self(v)->addFrame(get<CCSpriteFrame>(v, 2)); return 0; }
	NB_FUNC(addFrameWithFileName)		{ self(v)->addFrameWithFileName(get<StreamSource>(v, 2)); return 0; }
	NB_FUNC(addFrameWithTexture)		{ self(v)->addFrameWithTexture(get<CCTexture2D>(v, 2), *get<CCRect>(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCAnimationCache, AnimationCache);
NB_TYPE_WEAK(NIT2D_API, cc::AnimationCache, NULL);

class NB_CCAnimationCache : TNitClass<CCAnimationCache>
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
			FUNC_ENTRY_H(add,			"(anim: cc.Animation, name: string)"),
			FUNC_ENTRY_H(get,			"(name: string): cc.Animation"),
			FUNC_ENTRY_H(remove,		"(name: string)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(add)						{ self(v)->addAnimation(get<CCAnimation>(v, 2), getString(v, 3)); return 0; }
	NB_FUNC(get)						{ return push(v, self(v)->animationByName(getString(v, 2))); }
	NB_FUNC(remove)						{ self(v)->removeAnimationByName(getString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCAtlasNode, AtlasNode);
NB_TYPE_CC_REF(NIT2D_API, cc::AtlasNode, CCNode);

class NB_CCAtlasNode : TNitClass<CCAtlasNode>
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
			FUNC_ENTRY (updateAtlasValues),
			NULL
		};

		bind(v, props, funcs);

		NB_CCRGBAProtocol::Register<type>(v);
		NB_CCTextureProtocol::Register<type>(v);
	}

	
	NB_FUNC(updateAtlasValues)			{ self(v)->updateAtlasValues(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCLabelAtlas, LabelAtlas);
NB_TYPE_CC_REF(NIT2D_API, cc::LabelAtlas, CCAtlasNode);

class NB_CCLabelAtlas : TNitClass<CCLabelAtlas>
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
			CONS_ENTRY_H("(label: string, charMapSource: StreamSource*,itemWidth,itemHeight: int, startCharMap:char) : cc.CCLabelAtlas)"),

			FUNC_ENTRY	(updateAtlasValues),
			NULL
		};

		bind(v, props, funcs);

		NB_CCLabelProtocol::Register<type>(v);
	}
	NB_CONS()
	{
		setSelf(v, CCLabelAtlas::labelWithString(getString(v, 2), get<StreamSource>(v, 3), getInt(v, 4), getInt(v, 5), (u8)getInt(v, 6)) );
		return 0;
	}
	NB_FUNC(updateAtlasValues)			{ self(v)->updateAtlasValues(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCLabelBMFont, LabelBMFont);
NB_TYPE_CC_REF(NIT2D_API, cc::LabelBMFont, CCSpriteBatchNode);

class NB_CCLabelBMFont : TNitClass<CCLabelBMFont>
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
			CONS_ENTRY_H(					"(label: string, charMapSource: StreamSource)"),
			FUNC_ENTRY	(createFontChars),
			NULL
		};

		bind(v, props, funcs);

		NB_CCLabelProtocol::Register<type>(v);
		NB_CCRGBAProtocol::Register<type>(v);
	}

	NB_CONS()
	{
		setSelf(v, CCLabelBMFont::labelWithString(getString(v, 2), get<StreamSource>(v, 3)) );
		return 0;
	}
	
	NB_FUNC(createFontChars)			{ self(v)->createFontChars(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCLabelTTF, LabelTTF);
NB_TYPE_CC_REF(NIT2D_API, cc::LabelTTF, CCSprite);

class NB_CCLabelTTF : TNitClass<CCLabelTTF>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(font),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				
				"[deprecated] (label: string, dimensions: cc.Size, align: ALIGN, fontName: string, fontSize: float)\n"
				"[deprecated] (label: string, fontName: string, fontSize: float)\n"
				"(label: string, font: cc.Font=null, fontSize=0.0)\n"
				"(label: string, dimensions: cc.Size, align: ALIGN, font: cc.Font=null, fontSize=0.0)"),
			NULL
		};

		bind(v, props, funcs);

		NB_CCLabelProtocol::Register<type>(v);
	}

	NB_PROP_GET(font)					{ return push(v, self(v)->getFont()); }

	NB_PROP_SET(font)					{ self(v)->setFont(get<CCFont>(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 3))
			setSelf(v, CCLabelTTF::labelWithString(getString(v, 2)));
		else if (isString(v, 3))
			setSelf(v, CCLabelTTF::labelWithString(getString(v, 2), getString(v, 3), optFloat(v, 4, 0.0f)));
		else if (is<CCFont>(v, 3))
			setSelf(v, CCLabelTTF::labelWithString(getString(v, 2), get<CCFont>(v, 3), optFloat(v, 4, 0.0f)));
		else if (!isNone(v, 5) && isString(v, 5))
			setSelf(v, CCLabelTTF::labelWithString(getString(v, 2), *get<CCSize>(v, 3), CCTextAlignment(getInt(v, 4)), getString(v, 5), getFloat(v, 6)));
		else
			setSelf(v, CCLabelTTF::labelWithString(getString(v, 2), *get<CCSize>(v, 3), CCTextAlignment(getInt(v, 4)), opt<CCFont>(v, 5, NULL), optFloat(v, 6, 0.0f)));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCMenu, Menu);
NB_TYPE_CC_REF(NIT2D_API, cc::Menu, CCLayer);

class NB_CCMenu : TNitClass<CCMenu>
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
			CONS_ENTRY_H					(				"()"
			"\n"											"(item1, ...: cc.MenuItem)"),

			FUNC_ENTRY_H(alignItemsInColumns,				"(column: int, ... )"), 
			FUNC_ENTRY_H(alignItemsVertically,				"()"),
			FUNC_ENTRY_H(alignItemsHorizontally,			"()"),
			FUNC_ENTRY_H(alignItemsVerticallyWithPadding,   "(padding: float)"),
			FUNC_ENTRY_H(alignItemsHorizontallyWithPadding, "(padding: float)"),
			NULL
		};

		bind(v, props, funcs);

		NB_CCRGBAProtocol::Register<type>(v);
	}

	NB_CONS()
	{
		int top = sq_gettop(v);
		if (top == 1)
		{
			setSelf(v, CCMenu::node());
			return 0;
		}

		// Check all parameters in advance
		for (int i = 2; i <= top; ++i)
			if (!is<CCMenuItem>(v, i))
				return sq_throwerror(v, "cc.MenuItem expected");

		CCMenuItem* item0 = get<CCMenuItem>(v, 2);

		CCMenu* self = CCMenu::menuWithItem(item0);

		int z = 1;
		for (int i=3; i <= top; ++i)
		{
			CCMenuItem* item = get<CCMenuItem>(v, i);
			self->addChild(item, z++);
		}

		setSelf(v, self);
		return 0;
	}

	NB_FUNC(alignItemsInColumns)	    
	{
		int top = sq_gettop(v);
		if (top == 1)
		{
			return 0;
		}

		vector<unsigned int>::type columns;
		for (int i=2; i <= top; ++i)
		{
			columns.push_back(getInt(v, i));
		}
		self(v)->alignItemsInColumns(columns);

		return 0;
	}
	NB_FUNC(alignItemsVertically)	    { self(v)->alignItemsVertically(); return 0; }
	NB_FUNC(alignItemsHorizontally)		{ self(v)->alignItemsHorizontally(); return 0; }
	NB_FUNC(alignItemsVerticallyWithPadding)	{self(v)->alignItemsVerticallyWithPadding(getFloat(v, 2)); return 0; }
	NB_FUNC(alignItemsHorizontallyWithPadding)	{self(v)->alignItemsHorizontallyWithPadding(getFloat(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCMenuItem, MenuItem);
NB_TYPE_CC_REF(NIT2D_API, cc::MenuItem, CCNode);

class NB_CCMenuItem : TNitClass<CCMenuItem>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(selected),
			PROP_ENTRY	(enabled),
			PROP_ENTRY_R(rect),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(target: EventHandler)\n"
										"(sink, closure)"),
			FUNC_ENTRY_H(activate,		"()"),
			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(selected)				{ return push(v, self(v)->getIsSelected()); }
	NB_PROP_GET(enabled)				{ return push(v, self(v)->getIsEnabled()); }
	NB_PROP_GET(rect)					{ return push(v, self(v)->rect()); }

	NB_PROP_SET(selected)				{ if (getBool(v, 2)) self(v)->selected(); else self(v)->unselected(); return 0; }
	NB_PROP_SET(enabled)				{ self(v)->setIsEnabled(getBool(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCMenuItem::itemWithTarget(ScriptEventHandler::get(v, 2, 3))); return 0; }

	NB_FUNC(activate)					{ self(v)->activate(); return 0; }
	NB_FUNC(channel)					{ return push(v, self(v)->Channel()); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCMenuItemLabel, MenuItemLabel);
NB_TYPE_CC_REF(NIT2D_API, cc::MenuItemLabel, CCMenuItem);

class NB_CCMenuItemLabel : TNitClass<CCMenuItemLabel>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(disabledColor),
			PROP_ENTRY	(label),
			PROP_ENTRY	(string),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(label: CCNode, handler: EventHandler)\n"
										"(label: CCNode)"),
			NULL
		};

		bind(v, props, funcs);

		NB_CCRGBAProtocol::Register<type>(v);
	}

	NB_PROP_GET(disabledColor)			{ return push(v, FromColor3B(self(v)->getDisabledColor())); }
	NB_PROP_GET(label)					{ return push(v, self(v)->getLabel()); }
	NB_PROP_GET(string)
	{
		CCNode* label = self(v)->getLabel();
		CCLabelProtocol* lp = label->convertToLabelProtocol();
		if (lp) 
			return push(v, lp->getString());
		else
			return 0;
	}

	NB_PROP_SET(disabledColor)			{ self(v)->setDisabledColor(ToColor3B(*get<Color>(v, 2))); return 0; }
	NB_PROP_SET(label)					{ self(v)->setLabel(get<CCNode>(v, 2)); return 0; }
	NB_PROP_SET(string)					{ self(v)->setString(NitBind::getString(v, 2)); return 0; }

	NB_CONS()
	{
		CCNode* label = get<CCNode>(v, 2);
		if (label->convertToLabelProtocol() == NULL)
			return sq_throwerror(v, "node with CCLabelProtocol expected");

		if (isNone(v, 3))
			setSelf(v, CCMenuItemLabel::itemWithLabel(label));
		else
			setSelf(v, CCMenuItemLabel::itemWithLabel(label, ScriptEventHandler::get(v, 3, 4)));

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCMenuItemAtlasFont, MenuItemAtlasFont);
NB_TYPE_CC_REF(NIT2D_API, cc::MenuItemAtlasFont, CCMenuItemLabel);

class NB_CCMenuItemAtlasFont : TNitClass<CCMenuItemAtlasFont>
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
			CONS_ENTRY_H(				"(value: string, charMapSource: StreamSource, itemWidth, itemHeight: int, startCharMap: int)\n"
										"(value: string, charMapSource: StreamSource, itemWidth, itemHeight: int, startCharMap: int, handler: EventHandler)\n"),

			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 7))
			setSelf(v, CCMenuItemAtlasFont::itemFromString(getString(v, 2), get<StreamSource>(v, 3), getInt(v, 4), getInt(v, 5), getInt(v, 6)));
		else
			setSelf(v, CCMenuItemAtlasFont::itemFromString(getString(v, 2), get<StreamSource>(v, 3), getInt(v, 4), getInt(v, 5), getInt(v, 6), ScriptEventHandler::get(v, 7, 8)));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCMenuItemFont, MenuItemFont);
NB_TYPE_CC_REF(NIT2D_API, cc::MenuItemFont, CCMenuItemLabel);

class NB_CCMenuItemFont : TNitClass<CCMenuItemFont>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(fontSize),
			PROP_ENTRY	(fontName),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(value: string)\n"
										"(value: string, handler: EventHandler)"),
			FUNC_ENTRY_H(setDefaultFontSize, "[class] (size: int)"),
			FUNC_ENTRY_H(setDefaultFontName, "[class] (name: string)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(fontSize)				{ return push(v, self(v)->fontSizeObj()); }
	NB_PROP_GET(fontName)				{ return push(v, self(v)->fontNameObj()); }

	NB_PROP_SET(fontSize)				{ self(v)->setFontSizeObj(getInt(v, 2)); return 0; }
	NB_PROP_SET(fontName)				{ self(v)->setFontNameObj(getString(v, 2)); return 0; }

	NB_CONS()
	{
		if (isNone(v, 3))
			setSelf(v, CCMenuItemFont::itemFromString(getString(v, 2)));
		else
			setSelf(v, CCMenuItemFont::itemFromString(getString(v, 2), ScriptEventHandler::get(v, 3, 4)));
		return 0;
	}

	NB_FUNC(setDefaultFontSize)			{ CCMenuItemFont::setFontSize(getInt(v, 2)); return 0; }
	NB_FUNC(setDefaultFontName)			{ CCMenuItemFont::setFontName(getString(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCMenuItemSprite, MenuItemSprite);
NB_TYPE_CC_REF(NIT2D_API, cc::MenuItemSprite, CCMenuItem);

class NB_CCMenuItemSprite : TNitClass<CCMenuItemSprite>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(normalImage),
			PROP_ENTRY	(selectedImage),
			PROP_ENTRY	(disabledImage),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(
				"(normalSprite, selectedSprite: cc.Node)\n"
				"(normalSprite, selectedSprite: cc.Node, handler: EventHandler)\n"
				"(normalSprite, selectedSprite: cc.Node, sink, closure)\n"
				"(normalSprite, selectedSprite, disabledSprite: cc.Node)\n"
				"(normalSprite, selectedSprite, disabledSprite: cc.Node, handler: EventHandler)\n"
				"(normalSprite, selectedSprite, disabledSprite: cc.Node, sink, closure)\n"),
			NULL
		};

		bind(v, props, funcs);

		NB_CCRGBAProtocol::Register<type>(v);
	}

	NB_PROP_GET(normalImage)			{ return push(v, self(v)->getNormalImage()); }
	NB_PROP_GET(selectedImage)			{ return push(v, self(v)->getSelectedImage()); }
	NB_PROP_GET(disabledImage)			{ return push(v, self(v)->getDisabledImage()); }

	NB_PROP_SET(normalImage)			{ self(v)->setNormalImage(get<CCNode>(v, 2)); return 0; }
	NB_PROP_SET(selectedImage)			{ self(v)->setSelectedImage(get<CCNode>(v, 2)); return 0; }
	NB_PROP_SET(disabledImage)			{ self(v)->setDisabledImage(get<CCNode>(v, 2)); return 0; }

	NB_CONS()
	{
		if (!isNone(v, 4) && is<CCNode>(v, 4))
		{
			setSelf(v, CCMenuItemSprite::itemFromNormalSprite(get<CCNode>(v, 2), get<CCNode>(v, 3), get<CCNode>(v, 4), ScriptEventHandler::get(v, 5, 6)));
		}
		else
		{
			if (isNone(v, 4))
				setSelf(v, CCMenuItemSprite::itemFromNormalSprite(get<CCNode>(v, 2), get<CCNode>(v, 3)));
			else
				setSelf(v, CCMenuItemSprite::itemFromNormalSprite(get<CCNode>(v, 2), get<CCNode>(v, 3), ScriptEventHandler::get(v, 4, 5)));
		}
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCMenuItemImage, MenuItemImage);
NB_TYPE_CC_REF(NIT2D_API, cc::MenuItemImage, CCMenuItemSprite);

class NB_CCMenuItemImage : TNitClass<CCMenuItemImage>
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
				"(normal, selected: StreamSource)\n"
				"(normal, selected: StreamSource, handler: EventHandler)\n"
				"(normal, selected: StreamSource, sink, closure)\n"
				"(normal, selected, disabled: StreamSource)\n"
				"(normal, selected, disabled: StreamSource, handler: EventHandler)\n"
				"(normal, selected, disabled: StreamSource, sink, closure)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (!isNone(v, 4) && is<StreamSource>(v, 4))
		{
			if (isNone(v, 5))
				setSelf(v, CCMenuItemImage::itemFromNormalImage(get<StreamSource>(v, 2), get<StreamSource>(v, 3), get<StreamSource>(v, 4)));
			else
				setSelf(v, CCMenuItemImage::itemFromNormalImage(get<StreamSource>(v, 2), get<StreamSource>(v, 3), get<StreamSource>(v, 4), ScriptEventHandler::get(v, 5, 6)));
		}
		else
		{
			if (isNone(v, 4))
				setSelf(v, CCMenuItemImage::itemFromNormalImage(get<StreamSource>(v, 2), get<StreamSource>(v, 3)));
			else
				setSelf(v, CCMenuItemImage::itemFromNormalImage(get<StreamSource>(v, 2), get<StreamSource>(v, 3), ScriptEventHandler::get(v, 4, 5)));
		}
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCMenuItemToggle, MenuItemToggle);
NB_TYPE_CC_REF(NIT2D_API, cc::MenuItemToggle, CCMenuItem);

class NB_CCMenuItemToggle : TNitClass<CCMenuItemToggle>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(selectedIndex),
			PROP_ENTRY_R(subItems),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(handler: EventHandler, item1: cc.MenuItem, ...)\n"
										"(sink, closure, item1: cc.MenuItem, ...)"),
			FUNC_ENTRY_H(addSubItem,	"(item: cc.MenuItem)"),
			NULL
		};

		bind(v, props, funcs);

		NB_CCRGBAProtocol::Register<type>(v);
	}

	NB_PROP_GET(selectedIndex)			{ return push(v, self(v)->getSelectedIndex()); }
	NB_PROP_GET(subItems)
	{
		CCMutableArray<CCMenuItem*>* list = self(v)->getSubItems();

		sq_newarray(v, 0);

		for (uint i = 0; i < list->count(); ++i)
			arrayAppend(v, -1, list->getObjectAtIndex(i));

		return 1;
	}
	
	NB_PROP_SET(selectedIndex)			{ self(v)->setSelectedIndex(getInt(v, 2)); return 0; }

	NB_CONS()
	{
		SQInteger top = sq_gettop(v);

		SQInteger itemIdx;
		EventHandler* handler;
		if (is<EventHandler>(v, 2))
		{
			handler = get<EventHandler>(v, 2);
			itemIdx = 3;
		}
		else
		{
			handler = ScriptEventHandler::create(v, 2, 3);
			itemIdx = 4;
		}
			
		CCMenuItem* item1 = get<CCMenuItem>(v, itemIdx++);
		CCMenuItemToggle* self = CCMenuItemToggle::itemWithTarget(handler, item1, NULL);

		setSelf(v, self);

		for (; itemIdx <= top; ++itemIdx)
		{
			self->addSubItem(get<CCMenuItem>(v, itemIdx));
		}

		return 0;
	}

	NB_FUNC(addSubItem)					{ self(v)->addSubItem(get<CCMenuItem>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTextFieldTTF, TextFieldTTF);
NB_TYPE_CC_REF(NIT2D_API, cc::TextFieldTTF, CCLabelTTF);

class NB_CCTextFieldTTF : TNitClass<CCTextFieldTTF>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(charCount),
			PROP_ENTRY	(colorSpaceHolder),
			PROP_ENTRY	(fieldString),
			PROP_ENTRY	(placeHolder),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(holder: string, dimensions: cc.Size, align: int, fontname: string, fontsize: float)\n"
										"(holder: string, fontname: string, fontsize: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(charCount)				{ return push(v, self(v)->getCharCount()); }
	NB_PROP_GET(fieldString)			{ return push(v, (const char*)self(v)->getString()); }
	NB_PROP_GET(placeHolder)			{ return push(v, (const char*)self(v)->getPlaceHolder()); }
	NB_PROP_GET(colorSpaceHolder)		{ return push(v, FromColor3B(self(v)->getColorSpaceHolder(), self(v)->getOpacity()));  }

	NB_PROP_SET(fieldString)			{ self(v)->setString((const char*)getString(v, 2)); return 0; }
	NB_PROP_SET(placeHolder)			{ self(v)->setPlaceHolder(getString(v, 2)); return 0; }
	NB_PROP_SET(colorSpaceHolder)
	{
		Color& c = *get<Color>(v, 2); 
		self(v)->setColorSpaceHolder(ToColor3B(c)); 
		if (!self(v)->getIsOpacityModifyRGB())
			self(v)->setOpacity(GLubyte(c.a * 255)); 

		return 0;
	}

	NB_CONS()
	{
		if (isNone(v, 5))
			setSelf(v, CCTextFieldTTF::textFieldWithPlaceHolder(getString(v, 2), getString(v, 3), getFloat(v, 4)));
		else
			setSelf(v, CCTextFieldTTF::textFieldWithPlaceHolder(getString(v, 2), *get<CCSize>(v, 3), (CCTextAlignment)getInt(v, 4),
																getString(v, 5), getFloat(v, 6)));

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCRenderTexture, RenderTexture);
NB_TYPE_CC_REF(NIT2D_API, cc::RenderTexture, CCNode);

class NB_CCRenderTexture : TNitClass<CCRenderTexture>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(sprite),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(w,h: int)\n"
										"(w,h: int,  eFormat:CCTexture2DPixelFormat)"),
			FUNC_ENTRY_H(begin,			"(ctx: RenderContext)"),
			FUNC_ENTRY_H(beginWithClear,"(ctx: RenderContext, r=0.0, g=0.0, b=0.0, a=0.0)"),
			FUNC_ENTRY_H(end,			"(ctx: RenderContext): bool"),
			FUNC_ENTRY_H(clear,			"(ctx: RenderContext, r=0.0, g=0.0, b=0.0, a=0.0)"),
			FUNC_ENTRY_H(newImageCopy,	"(ctx: RenderContext, x=0, y=0, w=0, h=0): Image"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(sprite)					{ return push(v, self(v)->getSprite()); }

	NB_CONS()
	{
		if (isNone(v, 4))
			setSelf(v, CCRenderTexture::renderTextureWithWidthAndHeight(getInt(v, 2), getInt(v, 3)) );
		else
			setSelf(v, CCRenderTexture::renderTextureWithWidthAndHeight(getInt(v, 2), getInt(v, 3), (CCTexture2DPixelFormat)getInt(v, 4)) );
		return 0;
	}

	NB_FUNC(begin)						{ self(v)->begin(get<RenderContext>(v, 2)); return 0; }
	NB_FUNC(beginWithClear)				{ self(v)->beginWithClear(get<RenderContext>(v, 2), optFloat(v, 3, 0.0f), optFloat(v, 4, 0.0f), optFloat(v, 5, 0.0f), optFloat(v, 6, 0.0f)); return 0; }
	NB_FUNC(end)						{ self(v)->end(get<RenderContext>(v, 2)); return 0; }
	NB_FUNC(clear)						{ self(v)->clear(get<RenderContext>(v, 2), optFloat(v, 3, 0.0f), optFloat(v, 4, 0.0f), optFloat(v, 5, 0.0f), optFloat(v, 6, 0.0f)); return 0; }
	NB_FUNC(newImageCopy)				{ return push(v, self(v)->newImageCopy(get<RenderContext>(v, 2), optInt(v, 3, 0), optInt(v, 4, 0), optInt(v, 5, 0), optInt(v, 6, 0))); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTexture2D, Texture2D);
NB_TYPE_CC_REF(NIT2D_API, cc::Texture2D, CCObject);

class NB_CCTexture2D : TNitClass<CCTexture2D>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(pixelFormat),
			PROP_ENTRY_R(pixelWide),
			PROP_ENTRY_R(pixelHigh),
			PROP_ENTRY_R(contentSizeInPixels),
			PROP_ENTRY_R(hasPremultipliedAlpha),

			PROP_ENTRY_R(description),
			PROP_ENTRY_R(contentSize),
			PROP_ENTRY_R(bitsPerPixelForFormat),

			PROP_ENTRY_R(nitTexture),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(setAntiAliasTexParameters,				"()"),
			FUNC_ENTRY_H(setAliasTexParameters,					"()"),
			FUNC_ENTRY_H(generateMipmap,						"()"),
			FUNC_ENTRY_H(setDefaultAlphaPixelFormat,			"[class] (format : int)"),
			FUNC_ENTRY_H(defaultAlphaPixelFormat,				"[class] (): int"),
			NULL
		};
	
		bind(v, props, funcs);

		addStatic(v, "FORMAT_AUTOMATIC",(int)kCCTexture2DPixelFormat_Automatic);
		addStatic(v, "FORMAT_RGBA8888",	(int)kCCTexture2DPixelFormat_RGBA8888);
		addStatic(v, "FORMAT_RGB888",	(int)kCCTexture2DPixelFormat_RGB888);
		addStatic(v, "FORMAT_RGB565",	(int)kCCTexture2DPixelFormat_RGB565);
		addStatic(v, "FORMAT_A8",		(int)kCCTexture2DPixelFormat_A8);
		addStatic(v, "FORMAT_I8",		(int)kCCTexture2DPixelFormat_I8);
		addStatic(v, "FORMAT_AI88",		(int)kCCTexture2DPixelFormat_AI88);
		addStatic(v, "FORMAT_RGBA4444", (int)kCCTexture2DPixelFormat_RGBA4444);
		addStatic(v, "FORMAT_RGB5A1",	(int)kCCTexture2DPixelFormat_RGB5A1);
		addStatic(v, "FORMAT_PVRTC4",	(int)kCCTexture2DPixelFormat_PVRTC4);
		addStatic(v, "FORMAT_PVRTC2",	(int)kCCTexture2DPixelFormat_PVRTC2);
		addStatic(v, "FORMAT_DEFAULT",	(int)kCCTexture2DPixelFormat_Default);
	}

	NB_PROP_GET(pixelFormat)			{ return push(v, (int)(self(v)->getPixelFormat())); }
	NB_PROP_GET(pixelWide)				{ return push(v, (int)self(v)->getPixelsWide()); }
	NB_PROP_GET(pixelHigh)				{ return push(v, (int)self(v)->getPixelsHigh()); }
	NB_PROP_GET(contentSizeInPixels)	{ return push(v, self(v)->getContentSizeInPixels()); }
	NB_PROP_GET(hasPremultipliedAlpha)	{ return push(v, self(v)->getHasPremultipliedAlpha()); }
	NB_PROP_GET(description)			{ return push(v, (const char*)self(v)->description()); }
	NB_PROP_GET(contentSize)			{ return push(v, self(v)->getContentSize()); }
	NB_PROP_GET(bitsPerPixelForFormat)	{ return push(v, self(v)->bitsPerPixelForFormat()); }
	NB_PROP_GET(nitTexture)				{ return push(v, self(v)->getNitTexture()); }

	NB_FUNC	(setAntiAliasTexParameters)	{ self(v)->setAntiAliasTexParameters(); return 0;}
	NB_FUNC	(setAliasTexParameters)		{ self(v)->setAliasTexParameters(); return 0;}
	NB_FUNC	(generateMipmap)			{ self(v)->generateMipmap(); return 0;}
//	NB_FUNC	(setPVRImagesHavePremultipliedAlpha) { Self(v)->setPVRImagesHavePremultipliedAlpha(GetBool(v, 2)); return 0;}
	NB_FUNC	(setDefaultAlphaPixelFormat) { CCTexture2D::setDefaultAlphaPixelFormat((CCTexture2DPixelFormat)getInt(v, 2)); return 0;}
	NB_FUNC	(defaultAlphaPixelFormat)	{ return push(v, (int)CCTexture2D::defaultAlphaPixelFormat());}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTextureAtlas, TextureAtlas);
NB_TYPE_CC_REF(NIT2D_API, cc::TextureAtlas, CCObject);

class NB_CCTextureAtlas : TNitClass<CCTextureAtlas>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(totalQuads),
			PROP_ENTRY_R(capacity),
			PROP_ENTRY_R(description),
			PROP_ENTRY	(texture),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(source: StreamSource, capacity: uint)\n"
										"(texture: cc.Texture2D, capacity: uint)"),

			FUNC_ENTRY_H(insertQuadFromIndex, "(from: uint, new: uint)"),
			FUNC_ENTRY_H(removeQuadAtIndex, "(idx: uint)"),
			FUNC_ENTRY_H(removeAllQuads, "()"),
			FUNC_ENTRY_H(resizeCapacity, "(n: uint): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (is<CCTexture2D>(v, 2))
			setSelf(v, CCTextureAtlas::textureAtlasWithTexture(get<CCTexture2D>(v, 2), (uint)getInt(v, 3)));
		else
			setSelf(v, CCTextureAtlas::textureAtlasWithFile(get<StreamSource>(v, 2), (uint)getInt(v, 3)));

		return 0;
	}

	NB_PROP_GET(totalQuads)				{ return push(v, self(v)->getTotalQuads()); }
	NB_PROP_GET(capacity)				{ return push(v, self(v)->getCapacity()); }
	NB_PROP_GET(description)			{ return push(v, (const char*)self(v)->description()); }

	NB_PROP_GET(texture)				{ return push(v, self(v)->getTexture()); }
	NB_PROP_SET(texture)				{ self(v)->setTexture(get<CCTexture2D>(v, 2)); return 0; }

	NB_FUNC	(insertQuadFromIndex)		{ self(v)->insertQuadFromIndex((uint)getInt(v, 2), (uint)getInt(v, 3)); return 0; }
	NB_FUNC	(removeQuadAtIndex)			{ self(v)->removeQuadAtIndex((uint)getInt(v, 2)); return 0; }
	NB_FUNC	(removeAllQuads)			{ self(v)->removeAllQuads(); return 0; }
	NB_FUNC	(resizeCapacity)			{ return push(v, self(v)->resizeCapacity((uint)getInt(v, 2))); }
};	

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTextureCache, TextureCache);
NB_TYPE_WEAK(NIT2D_API, cc::TextureCache, NULL);

class NB_CCTextureCache : TNitClass<CCTextureCache>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(description),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(addImage,		"(source: SteamSource): cc.Texture2D"),
			FUNC_ENTRY_H(removeAllTextures, "()"),
			FUNC_ENTRY_H(removeUnusedTextures, "()"),
			FUNC_ENTRY_H(removeTexture, "(tex: cc.Texture2D)"),
			FUNC_ENTRY_H(dumpCachedTextureInfo, "()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(description)			{ return push(v, (const char*)self(v)->description()); }

	NB_FUNC	(addImage)					{ return push(v, self(v)->addImage(get<StreamSource>(v, 2))); }
	NB_FUNC	(removeAllTextures)			{ self(v)->removeAllTextures(); return 0; }
	NB_FUNC	(removeUnusedTextures)		{ self(v)->removeUnusedTextures(); return 0; }
	NB_FUNC	(removeTexture)				{ self(v)->removeTexture(get<CCTexture2D>(v, 2)); return 0; }
	NB_FUNC	(dumpCachedTextureInfo)		{ self(v)->dumpCachedTextureInfo(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCRibbon, Ribbon);
NB_TYPE_CC_REF(NIT2D_API, cc::Ribbon, CCNode);

class NB_CCRibbon : TNitClass<CCRibbon>
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
			FUNC_ENTRY_H(addPointAt,	"(location:CCPoint, width:float)"),
			FUNC_ENTRY_H(updateRibbon,	"(delta: ccTime(float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(addPointAt)					{ self(v)->addPointAt(*get<CCPoint>(v,2), getFloat(v,3)); return 0; }
	NB_FUNC(updateRibbon)				{ self(v)->updateRibbon(getFloat(v,2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCMotionStreak, MotionStreak);
NB_TYPE_CC_REF(NIT2D_API, cc::MotionStreak, CCNode);

class NB_CCMotionStreak : TNitClass<CCMotionStreak>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(ribbon),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(fade, seg: float, source: StreamSource, width, length: float, color: Color)"),
			NULL
		};

		bind(v, props, funcs);

		NB_CCTextureProtocol::Register<type>(v);
	}

	NB_PROP_GET(ribbon)					{ return push(v, self(v)->getRibbon()); }

	NB_CONS()							{ setSelf(v, CCMotionStreak::streakWithFade(getFloat(v, 2), getFloat(v, 3), get<StreamSource>(v, 4), getFloat(v, 5), getFloat(v, 6), ToColor4B(*get<Color>(v, 7)))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCProgressTimer, ProgressTimer);
NB_TYPE_CC_REF(NIT2D_API, cc::ProgressTimer, CCNode);

class NB_CCProgressTimer : TNitClass<CCProgressTimer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY(percentage),
			PROP_ENTRY(sprite),
			PROP_ENTRY(type),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(StreamSource: sourse)\n"
										"(pTexture: CCTexture2D)"),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "TYPE_RADIAL_CCW",				(int)kCCProgressTimerTypeRadialCCW);
		addStatic(v, "TYPE_RADIAL_CW",				(int)kCCProgressTimerTypeRadialCW);
		addStatic(v, "TYPE_HORZ_BAR_LR",			(int)kCCProgressTimerTypeHorizontalBarLR);
		addStatic(v, "TYPE_HORZ_BAR_RL",			(int)kCCProgressTimerTypeHorizontalBarRL);
		addStatic(v, "TYPE_VERT_BAR_BT",			(int)kCCProgressTimerTypeVerticalBarBT);
		addStatic(v, "TYPE_VERT_BAR_TB",			(int)kCCProgressTimerTypeVerticalBarTB);
	}

 	NB_CONS()
 	{
 		if (is<StreamSource>(v, 2))
 			setSelf(v, CCProgressTimer::progressWithFile(get<StreamSource>(v, 2)) );
 		else
 			setSelf(v, CCProgressTimer::progressWithTexture(get<CCTexture2D>(v, 2)) );
 		return 0;
 	}
 
 	NB_PROP_GET(percentage)				{ return push(v, (int)self(v)->getPercentage()); }
 	NB_PROP_GET(sprite)					{ return push(v, self(v)->getSprite()); }
 	NB_PROP_GET(type)					{ return push(v, (int)self(v)->getType()); }
 
 	NB_PROP_SET(percentage)				{ self(v)->setPercentage(getFloat(v, 2)); return 0; }
 	NB_PROP_SET(sprite)					{ self(v)->setSprite(get<CCSprite>(v, 2)); return 0; }
 	NB_PROP_SET(type)					{ self(v)->setType((CCProgressTimerType)getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCParallaxNode, ParallaxNode);
NB_TYPE_CC_REF(NIT2D_API, cc::ParallaxNode, CCNode);

class NB_CCParallaxNode : TNitClass<CCParallaxNode>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(parallaxArray),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY_H(addChild,		"(child: cc.Node, z: uint, paralratio: cc.Point, posOffset: cc.Point)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET	(parallaxArray)
	{
		_ccArray* list = self(v)->getParallaxArray();

		sq_newarray(v, 0);
		for (uint i = 0; i < list->num; ++i)
			arrayAppend(v, -1, list->arr[i]);

		return 1;
	}

	NB_CONS()							{ setSelf(v, CCParallaxNode::node()); return 0; }

	NB_FUNC(addChild)					{ self(v)->addChild(get<CCNode>(v, 2), (uint)getInt(v, 3), 
															*get<CCPoint>(v, 4), *get<CCPoint>(v, 5)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCActionManager, ActionManager);
NB_TYPE_WEAK(NIT2D_API, cc::ActionManager, NULL);

class NB_CCActionManager : TNitClass<CCActionManager>
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
			// NOTE: Shorten the method names.
			FUNC_ENTRY_H(add,			"(action: cc.Action, target: cc.Node, paused=false)"),
			FUNC_ENTRY_H(removeAll,		"(target: cc.Object=null) // if target=null remove everything"),
			FUNC_ENTRY_H(remove,		"(action: cc.Action)"),
			FUNC_ENTRY_H(removeByTag,	"(tag: int, target: cc.Object)"),
			FUNC_ENTRY_H(getByTag,		"(tag: int, target: cc.Object): cc.Action"),
			FUNC_ENTRY_H(count,			"(target: cc.Object): int"),
			FUNC_ENTRY_H(pause,			"(target: cc.Object)"),
			FUNC_ENTRY_H(resume,		"(target: cc.Obejct)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(add)						{ self(v)->addAction(get<CCAction>(v, 2), get<CCNode>(v, 3), optBool(v, 4, false)); return 0; }
	NB_FUNC(remove)						{ self(v)->removeAction(get<CCAction>(v, 2)); return 0; }

	NB_FUNC(removeAll)					
	{ 
		CCObject* target = opt<CCObject>(v, 2, NULL); 
		if (target) 
			self(v)->removeAllActionsFromTarget(target); 
		else 
			self(v)->removeAllActions(); 
		return 0; 
	}

	NB_FUNC(removeByTag)				{ self(v)->removeActionByTag(getInt(v, 2), get<CCObject>(v, 3)); return 0; }
	NB_FUNC(getByTag)					{ return push(v, self(v)->getActionByTag(getInt(v, 2), get<CCObject>(v, 3))); }
	NB_FUNC(count)						{ return push(v, self(v)->numberOfRunningActionsInTarget(get<CCObject>(v, 2))); }
	NB_FUNC(pause)						{ self(v)->pauseTarget(get<CCObject>(v, 2)); return 0; }
	NB_FUNC(resume)						{ self(v)->resumeTarget(get<CCObject>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCAction, Action);
NB_TYPE_CC_REF(NIT2D_API, cc::Action, CCObject);

class NB_CCAction : TNitClass<CCAction>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(description),
			PROP_ENTRY_R(done),
			PROP_ENTRY	(target),
			PROP_ENTRY	(originalTarget),
			PROP_ENTRY	(tag),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(description)			{ return push(v, (const char*)self(v)->description()); }
	NB_PROP_GET(done)					{ return push(v, self(v)->isDone()); }
	NB_PROP_GET(target)					{ return push(v, self(v)->getTarget()); }
	NB_PROP_GET(originalTarget)			{ return push(v, self(v)->getOriginalTarget()); }
	NB_PROP_GET(tag)					{ return push(v, self(v)->getTag()); }

	NB_PROP_SET(target)					{ self(v)->setTarget(get<CCNode>(v, 2)); return 0; }
	NB_PROP_SET(originalTarget)			{ self(v)->setOriginalTarget(get<CCNode>(v, 2)); return 0; }
	NB_PROP_SET(tag)					{ self(v)->setTag(getInt(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCFiniteTimeAction, FiniteTimeAction);
NB_TYPE_CC_REF(NIT2D_API, cc::FiniteTimeAction, CCAction);

class NB_CCFiniteTimeAction : TNitClass<CCFiniteTimeAction>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(duration),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(reverse,		"(): cc.FiniteTimeAction"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(duration)				{ return push(v, self(v)->getDuration()); }
	
	NB_PROP_SET(duration)				{ self(v)->setDuration(getFloat(v, 2)); return 0; }

	NB_FUNC(reverse)					{ return push(v, self(v)->reverse()); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCActionInterval, ActionInterval);
NB_TYPE_CC_REF(NIT2D_API, cc::ActionInterval, CCFiniteTimeAction);

class NB_CCActionInterval : TNitClass<CCActionInterval>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(elapsed),
			PROP_ENTRY	(amplitudeRate),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(elapsed)				{ return push(v, self(v)->getElapsed()); }
	NB_PROP_GET(amplitudeRate)			{ return push(v, self(v)->getAmplitudeRate()); }

	NB_PROP_SET(amplitudeRate)			{ self(v)->setAmplitudeRate(getFloat(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCActionEase, ActionEase);
NB_TYPE_CC_REF(NIT2D_API, cc::ActionEase, CCActionInterval);

class NB_CCActionEase : TNitClass<CCActionEase>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		// Noting to do
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

NS_IMPORT_NIT(cc, CCActionCamera, ActionCamera);
NB_TYPE_CC_REF(NIT2D_API, cc::ActionCamera, CCActionInterval);

class NB_CCActionCamera : TNitClass<CCActionCamera>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		// Nothing to do

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

NS_IMPORT_NIT(cc, CCActionInstant, ActionInstant);
NB_TYPE_CC_REF(NIT2D_API, cc::ActionInstant, CCFiniteTimeAction);

class NB_CCActionInstant : TNitClass<CCActionInstant>
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

NS_IMPORT_NIT(cc, CCParticleSystem, ParticleSystem);
NB_TYPE_CC_REF(NIT2D_API, cc::ParticleSystem, CCNode);

class NB_CCParticleSystem : TNitClass<CCParticleSystem>
{
public:

	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
 			PROP_ENTRY_R(active),
 			PROP_ENTRY_R(particleCount),
 			PROP_ENTRY	(duration),
 			PROP_ENTRY	(sourcePosition),
 			PROP_ENTRY	(posVar),
 			PROP_ENTRY	(life),
 			PROP_ENTRY	(lifeVar),
 			PROP_ENTRY	(angle),
 			PROP_ENTRY	(angleVar),
 
 			PROP_ENTRY	(startSize),
 			PROP_ENTRY	(startSizeVar),
 			PROP_ENTRY	(endSize),
 			PROP_ENTRY	(endSizeVar),
			PROP_ENTRY	(startColor),
			PROP_ENTRY	(startColorVar),
			PROP_ENTRY	(endColor),
			PROP_ENTRY	(endColorVar),
 			PROP_ENTRY	(startSpin),
 			PROP_ENTRY	(startSpinVar),
 			PROP_ENTRY	(endSpin),
 			PROP_ENTRY	(endSpinVar),
			PROP_ENTRY  (emissionRate),
 			PROP_ENTRY	(totalParticles),
			PROP_ENTRY_R(full),
			PROP_ENTRY	(blendFuncSrc),
			PROP_ENTRY	(blendFuncDst),
 			PROP_ENTRY	(blendAdditive),
			PROP_ENTRY	(positionType),
 			PROP_ENTRY	(autoRemoveOnFinish),
			PROP_ENTRY	(emitterMode),
 
			// mode a : gravity
			PROP_ENTRY	(gravity),
			PROP_ENTRY	(speed),
			PROP_ENTRY	(speedVar),
			PROP_ENTRY	(tangentialAccel),
			PROP_ENTRY	(tangentialAccelVar),
			PROP_ENTRY	(radialAccel),
			PROP_ENTRY	(radialAccelVar),

			// mode b : radius
			PROP_ENTRY	(startRadius),
			PROP_ENTRY	(startRadiusVar),
			PROP_ENTRY	(endRadius),
			PROP_ENTRY	(endRadiusVar),
			PROP_ENTRY	(rotatePerSecond),
			PROP_ENTRY	(rotatePerSecondVar),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(addParticle,	"(): bool"),
			FUNC_ENTRY_H(stopSystem,	"()"),
			FUNC_ENTRY_H(resetSystem,	"()"),
			NULL
		};

		bind(v, props, funcs);

		NB_CCTextureProtocol::Register<type>(v);

		addStatic(v, "DURATION_INFINITY",	(int)kCCParticleDurationInfinity);
		addStatic(v, "START_SIZE_EQUAL_TO_END_SIZE", (int)kCCParticleStartSizeEqualToEndSize);
		addStatic(v, "START_RADIUS_EQUAL_TO_END_RADIUS", (int)kCCParticleStartRadiusEqualToEndRadius);
		addStatic(v, "MODE_GRAVITY",		(int)kCCParticleModeGravity);
		addStatic(v, "MODE_RADIUS",			(int)kCCParticleModeRadius);
		addStatic(v, "POSITION_FREE",		(int)kCCPositionTypeFree);
		addStatic(v, "POSITION_RELATIVE",	(int)kCCPositionTypeRelative);
		addStatic(v, "POSITION_GROUPED",	(int)kCCPositionTypeGrouped);
	}

	NB_PROP_GET(active)					{ return push(v, self(v)->getIsActive()); }
	NB_PROP_GET(particleCount)			{ return push(v, (int)self(v)->getParticleCount()); }
	NB_PROP_GET(startColor)				{ return push(v, FromColor4F(self(v)->getStartColor())); }
	NB_PROP_GET(startColorVar)			{ return push(v, FromColor4F(self(v)->getStartColorVar())); }
	NB_PROP_GET(endColor)				{ return push(v, FromColor4F(self(v)->getEndColor())); }
	NB_PROP_GET(endColorVar)			{ return push(v, FromColor4F(self(v)->getEndColorVar())); }
	NB_PROP_GET(blendFuncSrc)			{ return push(v, (int)self(v)->getBlendFunc().src); }
	NB_PROP_GET(blendFuncDst)			{ return push(v, (int)self(v)->getBlendFunc().dst); }
	NB_PROP_GET(positionType)			{ return push(v, (int)self(v)->getPositionType()); }
	NB_PROP_GET(emitterMode)			{ return push(v, (int)self(v)->getEmitterMode()); }
	NB_PROP_GET(duration)				{ return push(v, self(v)->getDuration()); }
	NB_PROP_GET(sourcePosition)			{ return push(v, self(v)->getSourcePosition()); }
	NB_PROP_GET(posVar)					{ return push(v, self(v)->getPosVar()); }
	NB_PROP_GET(life)					{ return push(v, self(v)->getLife()); }
	NB_PROP_GET(lifeVar)				{ return push(v, self(v)->getLifeVar()); }
	NB_PROP_GET(angle)					{ return push(v, self(v)->getAngle()); }
	NB_PROP_GET(angleVar)				{ return push(v, self(v)->getAngleVar()); }
	NB_PROP_GET(startSize)				{ return push(v, self(v)->getStartSize()); }
	NB_PROP_GET(startSizeVar)			{ return push(v, self(v)->getStartSizeVar()); }
	NB_PROP_GET(endSize)				{ return push(v, self(v)->getEndSize()); }
	NB_PROP_GET(endSizeVar)				{ return push(v, self(v)->getEndSizeVar()); }
	NB_PROP_GET(startSpin)				{ return push(v, self(v)->getStartSpin()); }
	NB_PROP_GET(startSpinVar)			{ return push(v, self(v)->getStartSpinVar()); }
	NB_PROP_GET(endSpin)				{ return push(v, self(v)->getEndSpin()); }
	NB_PROP_GET(endSpinVar)				{ return push(v, self(v)->getEndSpinVar()); }
	NB_PROP_GET(emissionRate)			{ return push(v, self(v)->getEmissionRate()); }
	NB_PROP_GET(totalParticles)			{ return push(v, self(v)->getTotalParticles()); }
	NB_PROP_GET(blendAdditive)			{ return push(v, self(v)->getIsBlendAdditive()); }
	NB_PROP_GET(autoRemoveOnFinish)		{ return push(v, self(v)->getIsAutoRemoveOnFinish()); }
	NB_PROP_GET(full)					{ return push(v, self(v)->isFull()); }


	NB_PROP_SET(startColor)				{ self(v)->setStartColor(ToColor4F(*get<Color>(v, 2))); return 0; }
	NB_PROP_SET(startColorVar)			{ self(v)->setStartColorVar(ToColor4F(*get<Color>(v, 2))); return 0; }
	NB_PROP_SET(endColor)				{ self(v)->setEndColor(ToColor4F(*get<Color>(v, 2))); return 0; }
	NB_PROP_SET(endColorVar)			{ self(v)->setEndColorVar(ToColor4F(*get<Color>(v, 2))); return 0; }
	NB_PROP_SET(blendFuncSrc)			{ cocos2d::ccBlendFunc bf = self(v)->getBlendFunc(); bf.src = (GLenum)getInt(v, 2); self(v)->setBlendFunc(bf); return 0; }
	NB_PROP_SET(blendFuncDst)			{ cocos2d::ccBlendFunc bf = self(v)->getBlendFunc(); bf.dst = (GLenum)getInt(v, 2); self(v)->setBlendFunc(bf); return 0; }
	NB_PROP_SET(positionType)			{ self(v)->setPositionType(tCCPositionType(getInt(v, 2))); return 0; }
	NB_PROP_SET(emitterMode)			{ self(v)->setEmitterMode(getInt(v, 2)); return 0; }
	NB_PROP_SET(duration)				{ self(v)->setDuration(getFloat(v, 2)); return 0; }
	NB_PROP_SET(sourcePosition)			{ self(v)->setSourcePosition(*get<CCPoint>(v, 2)); return 0; }
	NB_PROP_SET(posVar)					{ self(v)->setPosVar(*get<CCPoint>(v, 2)); return 0; }
	NB_PROP_SET(life)					{ self(v)->setLife(getFloat(v, 2)); return 0; }
	NB_PROP_SET(lifeVar)				{ self(v)->setLifeVar(getFloat(v, 2)); return 0; }
	NB_PROP_SET(angle)					{ self(v)->setAngle(getFloat(v, 2)); return 0; }
	NB_PROP_SET(angleVar)				{ self(v)->setAngleVar(getFloat(v, 2)); return 0; }
	NB_PROP_SET(startSize)				{ self(v)->setStartSize(getFloat(v, 2)); return 0; }
	NB_PROP_SET(startSizeVar)			{ self(v)->setStartSizeVar(getFloat(v, 2)); return 0; }
	NB_PROP_SET(endSize)				{ self(v)->setEndSize(getFloat(v, 2)); return 0; }
	NB_PROP_SET(endSizeVar)				{ self(v)->setEndSizeVar(getFloat(v, 2)); return 0; }
	NB_PROP_SET(startSpin)				{ self(v)->setStartSpin(getFloat(v,2)); return 0; }
	NB_PROP_SET(startSpinVar)			{ self(v)->setStartSpinVar(getFloat(v, 2)); return 0; }
	NB_PROP_SET(endSpin)				{ self(v)->setEndSpin(getFloat(v, 2)); return 0; }
	NB_PROP_SET(endSpinVar)				{ self(v)->setEndSpinVar(getFloat(v, 2)); return 0; }
	NB_PROP_SET(emissionRate)			{ self(v)->setEmissionRate(getFloat(v, 2)); return 0; }
	NB_PROP_SET(totalParticles)			{ self(v)->setTotalParticles((unsigned int)getInt(v, 2)); return 0; }
	NB_PROP_SET(blendAdditive)			{ self(v)->setIsBlendAdditive(getBool(v, 2)); return 0; }
	NB_PROP_SET(autoRemoveOnFinish)		{ self(v)->setIsAutoRemoveOnFinish(getBool(v, 2)); return 0; }


#define IF_MODE_A(stmtTrue, stmtFalse) if (self(v)->getEmitterMode() == kCCParticleModeGravity) { stmtTrue; } else { stmtFalse; }
	NB_PROP_GET(gravity)				{ IF_MODE_A(return push(v, self(v)->getGravity()), return 0); }
	NB_PROP_GET(speed)					{ IF_MODE_A(return push(v, self(v)->getSpeed()), return 0); }
	NB_PROP_GET(speedVar)				{ IF_MODE_A(return push(v, self(v)->getSpeedVar()), return 0); }
	NB_PROP_GET(tangentialAccel)		{ IF_MODE_A(return push(v, self(v)->getTangentialAccel()), return 0); }
	NB_PROP_GET(tangentialAccelVar)		{ IF_MODE_A(return push(v, self(v)->getTangentialAccelVar()), return 0); }
	NB_PROP_GET(radialAccel)			{ IF_MODE_A(return push(v, self(v)->getRadialAccel()), return 0); }
	NB_PROP_GET(radialAccelVar)			{ IF_MODE_A(return push(v, self(v)->getRadialAccelVar()), return 0); }

	NB_PROP_SET(gravity)				{ IF_MODE_A(self(v)->setGravity(*get<CCPoint>(v, 2)); return 0, return sq_throwerror(v, "no gravity mode")); }
	NB_PROP_SET(speed)					{ IF_MODE_A(self(v)->setSpeed(getFloat(v, 2)); return 0, return sq_throwerror(v, "no gravity mode")); }
	NB_PROP_SET(speedVar)				{ IF_MODE_A(self(v)->setSpeedVar(getFloat(v, 2)); return 0, return sq_throwerror(v, "no gravity mode")); }
	NB_PROP_SET(tangentialAccel)		{ IF_MODE_A(self(v)->setTangentialAccel(getFloat(v, 2)); return 0, return sq_throwerror(v, "no gravity mode")); }
	NB_PROP_SET(tangentialAccelVar)		{ IF_MODE_A(self(v)->setTangentialAccelVar(getFloat(v, 2)); return 0, return sq_throwerror(v, "no gravity mode")); }
	NB_PROP_SET(radialAccel)			{ IF_MODE_A(self(v)->setRadialAccel(getFloat(v, 2)); return 0, return sq_throwerror(v, "no gravity mode")); }
	NB_PROP_SET(radialAccelVar)			{ IF_MODE_A(self(v)->setRadialAccelVar(getFloat(v, 2)); return 0, return sq_throwerror(v, "no gravity mode")); }
#undef IF_MODE_A

#define IF_MODE_B(stmtTrue, stmtFalse) if (self(v)->getEmitterMode() == kCCParticleModeRadius) { stmtTrue; } else { stmtFalse; }
	NB_PROP_GET(startRadius)			{ IF_MODE_B(return push(v, self(v)->getStartRadius()), return 0); }
	NB_PROP_GET(startRadiusVar)			{ IF_MODE_B(return push(v, self(v)->getStartRadiusVar()), return 0); }
	NB_PROP_GET(endRadius)				{ IF_MODE_B(return push(v, self(v)->getEndRadius()), return 0); }
	NB_PROP_GET(endRadiusVar)			{ IF_MODE_B(return push(v, self(v)->getEndRadiusVar()), return 0); }
	NB_PROP_GET(rotatePerSecond)		{ IF_MODE_B(return push(v, self(v)->getRotatePerSecond()), return 0); }
	NB_PROP_GET(rotatePerSecondVar)		{ IF_MODE_B(return push(v, self(v)->getRotatePerSecondVar()), return 0); }

	NB_PROP_SET(startRadius)			{ IF_MODE_B(self(v)->setStartRadius(getFloat(v, 2)); return 0, return sq_throwerror(v, "no radius mode")); }
	NB_PROP_SET(startRadiusVar)			{ IF_MODE_B(self(v)->setStartRadiusVar(getFloat(v, 2)); return 0, return sq_throwerror(v, "no radius mode")); }
	NB_PROP_SET(endRadius)				{ IF_MODE_B(self(v)->setEndRadius(getFloat(v, 2)); return 0, return sq_throwerror(v, "no radius mode")); }
	NB_PROP_SET(endRadiusVar)			{ IF_MODE_B(self(v)->setEndRadiusVar(getFloat(v, 2)); return 0, return sq_throwerror(v, "no radius mode")); }
	NB_PROP_SET(rotatePerSecond)		{ IF_MODE_B(self(v)->setRotatePerSecond(getFloat(v, 2)); return 0, return sq_throwerror(v, "no radius mode")); }
	NB_PROP_SET(rotatePerSecondVar)		{ IF_MODE_B(self(v)->setRotatePerSecondVar(getFloat(v, 2)); return 0, return sq_throwerror(v, "no radius mode")); }
#undef IF_MODE_A

	NB_FUNC(addParticle)				{ return push(v, self(v)->addParticle()); }
	NB_FUNC(stopSystem)					{ self(v)->stopSystem(); return 0; }
	NB_FUNC(resetSystem)				{ self(v)->resetSystem(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCParticleSystemQuad, ParticleSystemQuad);
NB_TYPE_CC_REF(NIT2D_API, cc::ParticleSystemQuad, CCParticleSystem);

class NB_CCParticleSystemQuad : TNitClass<CCParticleSystemQuad>
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
			CONS_ENTRY_H(						"(numParticles: int)\n"
												"(source: StreamSource)"),
			FUNC_ENTRY_H(setTextureWithRect,	"(texture: cc.Texture2D, rect: cc.Rect)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							
	{ 
		CCParticleSystemQuad* ps = NULL;

		if (isInt(v, 2))
		{
			int numParticles = getInt(v, 2);
			ps = new CCParticleSystemQuad();
			if (ps->initWithTotalParticles(numParticles))
			{
				ps->autorelease();
			}
			else
			{
				delete ps;
				ps = NULL;
			}
		}
		else
			ps = CCParticleSystemQuad::particleWithFile(get<StreamSource>(v, 2));

		setSelf(v, ps); 
		return 0; 
	}

	NB_FUNC(setTextureWithRect)			{ self(v)->setTextureWithRect(get<CCTexture2D>(v, 2), *get<CCRect>(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCParticleSystemPoint, ParticleSystemPoint);
NB_TYPE_CC_REF(NIT2D_API, cc::ParticleSystemPoint, CCParticleSystem);

class NB_CCParticleSystemPoint : TNitClass<CCParticleSystemPoint>
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
			CONS_ENTRY_H(						"(numParticles: int)\n"
												"(source: StreamSource)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		CCParticleSystemPoint* ps = NULL;

		if (isInt(v, 2))
		{
			int numParticles = getInt(v, 2);
			ps = new CCParticleSystemPoint();
			if (ps->initWithTotalParticles(numParticles))
			{
				ps->autorelease();
			}
			else
			{
				delete ps;
				ps = NULL;
			}
		}
		else
			ps = CCParticleSystemPoint::particleWithFile(get<StreamSource>(v, 2));

		setSelf(v, ps); 
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCConfiguration, Configuration);
NB_TYPE_WEAK(NIT2D_API, cc::Configuration, NULL);

class NB_CCConfiguration : TNitClass<CCConfiguration>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(glesVersion),
			PROP_ENTRY_R(maxTextureSize),
			PROP_ENTRY_R(maxModelviewStackDepth),
			PROP_ENTRY_R(supportsNPOT),
			PROP_ENTRY_R(supportsPVRTC),
			PROP_ENTRY_R(supportsBGRA8888),
			PROP_ENTRY_R(supportsDiscardFramebuffer),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(checkForGLExtension, "(searchName: string): bool"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(maxTextureSize)				{ return push(v, self(v)->getMaxTextureSize()); }
	NB_PROP_GET(maxModelviewStackDepth)		{ return push(v, self(v)->getMaxModelviewStackDepth()); }
	NB_PROP_GET(supportsNPOT)				{ return push(v, self(v)->isSupportsNPOT()); }
	NB_PROP_GET(supportsPVRTC)				{ return push(v, self(v)->isSupportsPVRTC()); }
	NB_PROP_GET(supportsBGRA8888)			{ return push(v, self(v)->isSupportsBGRA8888()); }
	NB_PROP_GET(supportsDiscardFramebuffer)	{ return push(v, self(v)->isSupportsDiscardFramebuffer()); }

	NB_PROP_GET(glesVersion)
	{
		switch (self(v)->getGlesVersion())
		{
		case GLES_VER_1_0: return push(v, "GLES1.0");
		case GLES_VER_1_1: return push(v, "GLES1.1");
		case GLES_VER_2_0: return push(v, "GLES2.0");
		default: 
			return push(v, "INVALID");
		}
	}

	NB_FUNC(checkForGLExtension)		{ return push(v, self(v)->checkForGLExtension(getString(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTouchInjector, TouchInjector);
NB_TYPE_CC_REF(NIT2D_API, cc::TouchInjector, CCNode);

class NB_CCTouchInjector : TNitClass<CCTouchInjector>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(enabled),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(nViewId: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(enabled)				{ return push(v, self(v)->isEnabled()); }
	NB_PROP_SET(enabled)				{ self(v)->setEnabled(getBool(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCTouchInjector::injectorWithViewId(getInt(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCTouchDispatcher, TouchDispatcher);
NB_TYPE_WEAK(NIT2D_API, cc::TouchDispatcher, NULL);

class NB_CCTouchDispatcher : TNitClass<CCTouchDispatcher>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(enabled),
			PROP_ENTRY_R(modalCount),
			PROP_ENTRY_R(holdCount),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(setPriority,	"(priority: int, delegate: CCTouchDelegate): bool // returns true when delegate is CCTouchDelegate"),

			FUNC_ENTRY_H(beginTouch,	"(touch0: cc.Touch, ...)"),
			FUNC_ENTRY_H(moveTouch,		"(touch0: cc.Touch, ...)"),
			FUNC_ENTRY_H(endTouch,		"(touch0: cc.Touch, ...)"),
			FUNC_ENTRY_H(cancelTouch,	"(touch0: cc.Touch, ...)"),

			FUNC_ENTRY_H(injectBeginTouch,	"(touch0: cc.Touch, ...)"),
			FUNC_ENTRY_H(injectMoveTouch,	"(touch0: cc.Touch, ...)"),
			FUNC_ENTRY_H(injectEndTouch,	"(touch0: cc.Touch, ...)"),
			FUNC_ENTRY_H(injectCancelTouch,	"(touch0: cc.Touch, ...)"),

			FUNC_ENTRY_H(holdUserTouch,	"()"),
			FUNC_ENTRY_H(unholdUserTouch, "()"),

			FUNC_ENTRY_H(addModal,		"(delegate: CCTouchDelegate): bool // returns true when delegate is CCTouchDelegate"),
			FUNC_ENTRY_H(removeModal,	"(delegate: CCTouchDelegate): bool // returns true when delegate is CCTouchDelegate"),
			FUNC_ENTRY_H(clearModals,	"()"),

			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(enabled)				{ return push(v, self(v)->isDispatchEvents()); }
	NB_PROP_GET(modalCount)				{ return push(v, self(v)->GetModalCount()); }
	NB_PROP_GET(holdCount)				{ return push(v, self(v)->getHoldCount()); }

	NB_PROP_SET(enabled)				{ self(v)->setDispatchEvents(getBool(v, 2)); return 0; }

	NB_FUNC(holdUserTouch)				{ self(v)->holdUserTouch(); return 0; }
	NB_FUNC(unholdUserTouch)			{ self(v)->unholdUserTouch(); return 0; }

	NB_FUNC(setPriority)				
	{ 
		CCTouchDelegate* delegate = getInterface<CCTouchDelegate>(v, 3, true);
		if (delegate) 
			self(v)->setPriority(getInt(v, 2), delegate); 
		return push(v, delegate != NULL); 
	}

	NB_FUNC(addModal)					
	{ 
		CCTouchDelegate* delegate = getInterface<CCTouchDelegate>(v, 2, true);
		if (delegate) 
			self(v)->AddModal(delegate); 
		return push(v, delegate != NULL); 
	}

	NB_FUNC(removeModal)				
	{ 
		CCTouchDelegate* delegate = getInterface<CCTouchDelegate>(v, 2, true);
		if (delegate) 
			self(v)->RemoveModal(delegate); 
		return push(v, delegate != NULL); 
	}

	NB_FUNC(clearModals)				{ self(v)->ClearModals(); return 0; }

	NB_FUNC(beginTouch)
	{
		CCSet touches;
		int nargs = sq_gettop(v);
		for (int i=2; i <= nargs; ++i)
		{
			touches.addObject(get<CCTouch>(v, i));
		}

		self(v)->touchesBegan(&touches);
		return 0; 
	}

	NB_FUNC(moveTouch)
	{
		CCSet touches;
		int nargs = sq_gettop(v);
		for (int i=2; i <= nargs; ++i)
		{
			touches.addObject(get<CCTouch>(v, i));
		}

		self(v)->touchesMoved(&touches);
		return 0; 
	}

	NB_FUNC(endTouch)
	{
		CCSet touches;
		int nargs = sq_gettop(v);
		for (int i=2; i <= nargs; ++i)
		{
			touches.addObject(get<CCTouch>(v, i));
		}

		self(v)->touchesEnded(&touches);
		return 0; 
	}

	NB_FUNC(cancelTouch)
	{
		CCSet touches;
		int nargs = sq_gettop(v);
		for (int i=2; i <= nargs; ++i)
		{
			touches.addObject(get<CCTouch>(v, i));
		}

		self(v)->touchesCancelled(&touches);
		return 0; 
	}

	NB_FUNC(injectBeginTouch)
	{
		CCSet touches;
		int nargs = sq_gettop(v);
		for (int i=2; i <= nargs; ++i)
		{
			touches.addObject(get<CCTouch>(v, i));
		}

		self(v)->injectTouches(&touches, CCTOUCHBEGAN);
		return 0; 
	}

	NB_FUNC(injectMoveTouch)
	{
		CCSet touches;
		int nargs = sq_gettop(v);
		for (int i=2; i <= nargs; ++i)
		{
			touches.addObject(get<CCTouch>(v, i));
		}

		self(v)->injectTouches(&touches, CCTOUCHMOVED);
		return 0; 
	}

	NB_FUNC(injectEndTouch)
	{
		CCSet touches;
		int nargs = sq_gettop(v);
		for (int i=2; i <= nargs; ++i)
		{
			touches.addObject(get<CCTouch>(v, i));
		}

		self(v)->injectTouches(&touches, CCTOUCHENDED);
		return 0; 
	}

	NB_FUNC(injectCancelTouch)
	{
		CCSet touches;
		int nargs = sq_gettop(v);
		for (int i=2; i <= nargs; ++i)
		{
			touches.addObject(get<CCTouch>(v, i));
		}

		self(v)->injectTouches(&touches, CCTOUCHCANCELLED);
		return 0; 
	}

	NB_FUNC(channel)					{ return push(v, self(v)->Channel()); }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCKeypadDispatcher, KeypadDispatcher);
NB_TYPE_WEAK(NIT2D_API, cc::KeypadDispatcher, NULL);

class NB_CCKeypadDispatcher : TNitClass<CCKeypadDispatcher>
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

		// NOTHING TO DO
		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCIMEDispatcher, IMEDispatcher);
NB_TYPE_WEAK(NIT2D_API, cc::IMEDispatcher, NULL);

class NB_CCIMEDispatcher : TNitClass<CCIMEDispatcher>
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

NS_IMPORT_NIT(cc, CCFontManager, FontManager);
NB_TYPE_WEAK(NIT2D_API, cc::FontManager, NULL);

class NB_CCFontManager : TNitClass<CCFontManager>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(default),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(register,		"(alias: string, font: cc.Font): cc.Font\n"
										"(alias: string, src: StreamSource, defaultSize=0.0): cc.Font\n"
										"(alias: string, src: StreamReader, defaultSize=0.0): cc.Font\n"),
			FUNC_ENTRY_H(unregister,	"(alias: string)"),
			FUNC_ENTRY_H(getFont,		"(alias: string): cc.Font"),
			
			FUNC_ENTRY_H(install,		"(facename: string, src: StreamSource, faceIndex=0): cc.FontFace"
										"(facename: string, src: StreamReader, faceIndex=0): cc.FontFace"),
			FUNC_ENTRY_H(uninstall,		"(facename: string)"),
			FUNC_ENTRY_H(getFontFace,	"(facename: string): cc.FontFace"),

			FUNC_ENTRY_H(compact,		"()"),

			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(default)				{ return push(v, self(v)->getDefault()); }

	NB_PROP_SET(default)				{ self(v)->setDefault(get<CCFont>(v, 2)); return 0; }

	NB_FUNC(register)
	{
		if (is<StreamSource>(v, 3))
			return push(v, self(v)->Register(getString(v, 2), get<StreamSource>(v, 3), optFloat(v, 4, 0.0f)));
		else if (is<StreamReader>(v, 3))
			return push(v, self(v)->Register(getString(v, 2), get<StreamSource>(v, 3), optFloat(v, 4, 0.0f)));
		else
			return push(v, self(v)->Register(getString(v, 2), get<CCFont>(v, 3)));
	}

	NB_FUNC(unregister)					{ self(v)->unregister(getString(v, 2)); return 0; }
	NB_FUNC(getFont)					{ return push(v, self(v)->getFont(getString(v, 2))); }

	NB_FUNC(install)
	{
		if (is<StreamSource>(v, 3))
			return push(v, self(v)->install(getString(v, 2), get<StreamSource>(v, 3), optInt(v, 4, 0)));
		else
			return push(v, self(v)->install(getString(v, 2), get<StreamReader>(v, 3), optInt(v, 4, 0)));
	}

	NB_FUNC(uninstall)					{ self(v)->uninstall(getString(v, 2)); return 0; }
	NB_FUNC(getFontFace)				{ return push(v, self(v)->getFontFace(getString(v, 2))); }

	NB_FUNC(compact)					{ self(v)->compact(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCFontFace, FontFace);
NB_TYPE_WEAK(NIT2D_API, cc::FontFace, NULL);

class NB_CCFontFace : TNitClass<CCFontFace>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY	(flags),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(addAlternative,"(face: cc.FontFace, codeBegin: int, codeEnd: int, scaling=1.0)"),
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "FLAGS");
		newSlot(v, -1, "DEFAULT",		(int)CCFontFace::FO_DEFAULT);
		newSlot(v, -1, "NO_HINTING",	(int)CCFontFace::FO_NO_HINTING);
		newSlot(v, -1, "NO_AUTO_HINT",	(int)CCFontFace::FO_NO_AUTO_HINT);
		newSlot(v, -1, "NO_KERNING",	(int)CCFontFace::FO_NO_KERNING);
		newSlot(v, -1, "NO_BITMAP",		(int)CCFontFace::FO_NO_BITMAP);
		newSlot(v, -1, "NO_ANTIALIAS",	(int)CCFontFace::FO_NO_ANTIALIAS);
		newSlot(v, -1, "LIGHT_HINTING",	(int)CCFontFace::FO_LIGHT_HINTING);
		sq_poptop(v);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(flags)					{ return push(v, self(v)->getFlags()); }

	NB_PROP_SET(flags)					{ self(v)->setFlags(getInt(v, 2)); return 0; }

	NB_FUNC(addAlternative)				{ self(v)->addAlternative(get<CCFontFace>(v, 2), getInt(v, 3), getInt(v, 4), optFloat(v, 5, 1.0f)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NS_IMPORT_NIT(cc, CCFont, Font);
NB_TYPE_REF(NIT2D_API, cc::Font, RefCounted, incRefCount, decRefCount);

class NB_CCFont : TNitClass<CCFont>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(face),
			PROP_ENTRY	(defaultSize),
			PROP_ENTRY	(kerningScale),
			PROP_ENTRY	(advanceScale),
			PROP_ENTRY	(lineScale),
			PROP_ENTRY	(lineSpacing),
			PROP_ENTRY	(charSpacing),
			PROP_ENTRY	(dpi),
			PROP_ENTRY	(color),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(defaultFace: cc.FontFace, defaultSize: float)\n"
										"(deriveFrom: cc.Font)\n"
										"(fromAlias: string)\n"),
			FUNC_ENTRY_H(addStroke,		"(width: float, color: Color, offx=0.0, offy=0.0)"),
			FUNC_ENTRY_H(clearStrokes,	"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(face)					{ return push(v, self(v)->getFace()); }
	NB_PROP_GET(defaultSize)			{ return push(v, self(v)->getDefaultSize()); }
	NB_PROP_GET(kerningScale)			{ return push(v, self(v)->getKerningScale()); }
	NB_PROP_GET(advanceScale)			{ return push(v, self(v)->getAdvanceScale()); }
	NB_PROP_GET(lineScale)				{ return push(v, self(v)->getLineScale()); }
	NB_PROP_GET(lineSpacing)			{ return push(v, self(v)->getLineSpacing()); }
	NB_PROP_GET(charSpacing)			{ return push(v, self(v)->getCharSpacing()); }
	NB_PROP_GET(dpi)					{ return push(v, self(v)->getDpi()); }
	NB_PROP_GET(color)					{ return push(v, FromColor4B(self(v)->getColor())); }

	NB_PROP_SET(defaultSize)			{ self(v)->setDefaultSize(getFloat(v, 2)); return 0; }
	NB_PROP_SET(kerningScale)			{ self(v)->setKerningScale(getFloat(v, 2)); return 0; }
	NB_PROP_SET(advanceScale)			{ self(v)->setAdvanceScale(getFloat(v, 2)); return 0; }
	NB_PROP_SET(lineScale)				{ self(v)->setLineScale(getFloat(v, 2)); return 0; }
	NB_PROP_SET(lineSpacing)			{ self(v)->setLineSpacing(getInt(v, 2)); return 0; }
	NB_PROP_SET(charSpacing)			{ self(v)->setCharSpacing(getInt(v, 2)); return 0; }
	NB_PROP_SET(dpi)					{ self(v)->setDpi(getInt(v, 2)); return 0; }
	NB_PROP_SET(color)					{ self(v)->setColor(ToColor4B(*get<Color>(v, 2))); return 0; }

	NB_CONS()
	{
		if (isString(v, 2))
			setSelf(v, new CCFont(getString(v, 2)));
		else if (is<CCFont>(v, 2))
			setSelf(v, new CCFont(get<CCFont>(v, 2)));
		else
			setSelf(v, new CCFont(get<CCFontFace>(v, 2), getFloat(v, 3)));
		return 0;
	}

	NB_FUNC(addStroke)					{ self(v)->addStroke(getFloat(v, 2), ToColor4B(*get<Color>(v, 3)), optFloat(v, 4, 0.0f), optFloat(v, 5, 0.0f)); return 0; }
	NB_FUNC(clearStrokes)				{ self(v)->clearStrokes(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NIT2D_API SQRESULT NitLibCocos(HSQUIRRELVM v)
{
	NB_CCSize::Register(v);
	NB_CCRect::Register(v);
	NB_CCAffine::Register(v);
	NB_CCGridSize::Register(v);

	NB_CCObject::Register(v);
	NB_CCNode::Register(v);

	NB_CCGridBase::Register(v);
	NB_CCGrid3D::Register(v);
	NB_CCTiledGrid3D::Register(v);

	NB_CCCamera::Register(v);
	NB_CCTouch::Register(v);

	NB_CCDirector::Register(v);

	NB_CCScene::Register(v);

	NB_CCLayer::Register(v);
	NB_CCLayerColor::Register(v);
	NB_CCLayerGradient::Register(v);
	NB_CCLayerMultiplex::Register(v);

	NB_CCSprite::Register(v);
	NB_CCSpriteBatchNode::Register(v);
	NB_CCSpriteFrame::Register(v);
	NB_CCSpriteFrameCache::Register(v);

	NB_CCAnimation::Register(v);
	NB_CCAnimationCache::Register(v);

	NB_CCAtlasNode::Register(v);
	NB_CCLabelAtlas::Register(v);
	NB_CCLabelBMFont::Register(v);
	NB_CCLabelTTF::Register(v);

	NB_CCMenu::Register(v);
	NB_CCMenuItem::Register(v);
	NB_CCMenuItemLabel::Register(v);
	NB_CCMenuItemAtlasFont::Register(v);
	NB_CCMenuItemFont::Register(v);
	NB_CCMenuItemSprite::Register(v);
	NB_CCMenuItemImage::Register(v);
	NB_CCMenuItemToggle::Register(v);

	NB_CCTextFieldTTF::Register(v);

	NB_CCRenderTexture::Register(v);

	NB_CCTexture2D::Register(v);
	NB_CCTextureAtlas::Register(v);
	NB_CCTextureCache::Register(v);

	NB_CCRibbon::Register(v);
	NB_CCMotionStreak::Register(v);
	NB_CCProgressTimer::Register(v);
	NB_CCParallaxNode::Register(v);

	NB_CCActionManager::Register(v);
	NB_CCAction::Register(v);
	NB_CCFiniteTimeAction::Register(v);
	NB_CCActionInterval::Register(v);
	NB_CCActionEase::Register(v);
	NB_CCActionCamera::Register(v);
	NB_CCActionInstant::Register(v);

	NB_CCParticleSystem::Register(v);
	NB_CCParticleSystemQuad::Register(v);
	NB_CCParticleSystemPoint::Register(v);

	NB_CCConfiguration::Register(v);

	NB_CCTouchInjector::Register(v);

	NB_CCTouchDispatcher::Register(v);
	NB_CCKeypadDispatcher::Register(v);
	NB_CCIMEDispatcher::Register(v);

	NB_CCFontManager::Register(v);
	NB_CCFontFace::Register(v);
	NB_CCFont::Register(v);

	sq_dostring(v, "cc.Point := nit.Vector2");

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
