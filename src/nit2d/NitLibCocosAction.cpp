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

#include "CCGrid.h"
#include "CCKeypadDispatcher.h"
#include "CCIMEDispatcher.h"
#include "CCConfiguration.h"

using namespace cocos2d;

////////////////////////////////////////////////////////////////////////////////

// Redeclare namespace as cc -> cc.action

namespace cc
{
	namespace action
	{
		// CCAction.h
		typedef CCSpeed					Speed;
		typedef CCFollow				Follow;
		
		// CCActionCamera.h
		typedef CCOrbitCamera			OrbitCamera;

		// CCActionEase.h
		typedef CCEaseRateAction		EaseRateAction;
		typedef CCEaseIn				EaseIn;
		typedef CCEaseOut				EaseOut;
		typedef CCEaseInOut				EaseInOut;
		typedef CCEaseExponentialIn		EaseExponentialIn;
		typedef CCEaseExponentialOut	EaseExponentialOut;
		typedef CCEaseExponentialInOut	EaseExponentialInOut;
		typedef CCEaseSineIn			EaseSineIn;
		typedef CCEaseSineOut			EaseSineOut;
		typedef CCEaseSineInOut			EaseSineInOut;
		typedef CCEaseElastic			EaseElastic;
		typedef CCEaseElasticIn			EaseElasticIn;
		typedef CCEaseElasticOut		EaseElasticOut;
		typedef CCEaseElasticInOut		EaseElasticInOut;
		typedef CCEaseBounce			EaseBounce;
		typedef CCEaseBounceIn			EaseBounceIn;
		typedef CCEaseBounceOut			EaseBounceOut;
		typedef CCEaseBounceInOut		EaseBounceInOut;
		typedef CCEaseBackIn			EaseBackIn;
		typedef CCEaseBackOut			EaseBackOut;
		typedef CCEaseBackInOut			EaseBackInOut;

		// CCActionGrid.h
		typedef CCGridAction			GridAction;
		typedef CCGrid3DAction			Grid3DAction;
		typedef CCTiledGrid3DAction		TiledGrid3DAction;
		typedef CCAccelDeccelAmplitude	AccelDeccelAmplitude;
		typedef CCAccelAmplitude		AccelAmplitude;
		typedef CCDeccelAmplitude		DeccelAmplitude;
		typedef CCStopGrid				StopGrid;
		typedef CCReuseGrid				ReuseGrid;

		// CCActionGrid3D.h
		typedef CCWaves3D				Waves3D;
		typedef CCFlipX3D				FlipX3D;
		typedef CCFlipY3D				FlipY3D;
		typedef CCLens3D				Lens3D;
		typedef CCRipple3D				Ripple3D;
		typedef CCShaky3D				Shaky3D;
		typedef CCLiquid				Liquid;
		typedef CCWaves					Waves;
		typedef CCTwirl					Twirl;

		// CCActionInstant.h
		typedef CCShow					Show;
		typedef CCHide					Hide;
		typedef CCToggleVisibility		ToggleVisibility;
		typedef CCFlipX					FlipX;
		typedef CCFlipY					FlipY;
		typedef CCPlace					Place;
		typedef CCEventCall				EventCall;

		// CCActionInterval.h
		typedef CCSequence				Sequence;
		typedef CCRepeat				Repeat;
		typedef CCRepeatForever			RepeatForever;
		typedef CCSpawn					Spawn;
		typedef CCRotateTo				RotateTo;
		typedef CCRotateBy				RotateBy;
		typedef CCMoveTo				MoveTo;
		typedef CCMoveBy				MoveBy;
		typedef CCSkewTo				SkewTo;
		typedef CCSkewBy				SkewBy;
		typedef CCJumpBy				JumpBy;
		typedef CCJumpTo				JumpTo;
		typedef CCBezierBy				BezierBy;
		typedef CCBezierTo				BezierTo;
		typedef CCScaleTo				ScaleTo;
		typedef CCScaleBy				ScaleBy;
		typedef CCBlink					Blink;
		typedef CCFadeIn				FadeIn;
		typedef CCFadeOut				FadeOut;
		typedef CCFadeTo				FadeTo;
		typedef CCTintTo				TintTo;
		typedef CCTintBy				TintBy;
		typedef CCDelayTime				DelayTime;
		typedef CCReverseTime			ReverseTime;
		typedef CCAnimate				Animate;

		// CCActionPageTurn3d.h
		typedef CCPageTurn3D			PageTurn3D;

		// CCActionProgressTimer.h
		typedef CCProgressTo			ProgressTo;
		typedef CCProgressFromTo		ProgressFromTo;

		// CCActionTiledGrid.h
		typedef CCShakyTiles3D			ShakyTiles3D;
		typedef CCShatteredTiles3D		ShatteredTiles3D;
		typedef CCShuffleTiles			ShuffleTiles;
		typedef CCFadeOutTRTiles		FadeOutTRTiles;
		typedef CCFadeOutBLTiles		FadeOutBLTiles;
		typedef CCFadeOutUpTiles		FadeOutUpTiles;
		typedef CCFadeOutDownTiles		FadeOutDownTiles;
		typedef CCTurnOffTiles			TurnOffTiles;
		typedef CCWavesTiles3D			WavesTiles3D;
		typedef CCJumpTiles3D			JumpTiles3D;
		typedef CCSplitRows				SplitRows;
		typedef CCSplitCols				SplitCols;
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Speed, CCAction);

class NB_CCSpeed : TNitClass<CCSpeed>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(speed),
			PROP_ENTRY	(innerAction),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(Action: cc.ActionInterval, Rate: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(speed)					{ return push(v, self(v)->getSpeed()); }
	NB_PROP_GET(innerAction)			{ return push(v, self(v)->getInnerAction()); }

	NB_PROP_SET(speed)					{ self(v)->setSpeed(getFloat(v, 2)); return 0; }
	NB_PROP_SET(innerAction)			{ self(v)->setInnerAction(get<CCActionInterval>(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCSpeed::actionWithAction(get<CCActionInterval>(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Follow, CCAction);

class NB_CCFollow : TNitClass<CCFollow>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(boundarySet),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(followedNode: cc.Node)\n"
										"(followedNode: cc.Node, rect: cc.Rect)"),
			NULL
		};

		bind(v, props, funcs);

	}

	NB_PROP_GET(boundarySet)			{ return push(v, self(v)->isBoundarySet()); }

	NB_PROP_SET(boundarySet)			{ self(v)->setBoudarySet(getBool(v, 2)); return 0; }

	NB_CONS()						
	{
		if(isNone(v, 3))
			setSelf(v, CCFollow::actionWithTarget(get<CCNode>(v, 2)));
		else
			setSelf(v, CCFollow::actionWithTarget(get<CCNode>(v, 2), *get<CCRect>(v, 3)));
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::OrbitCamera, CCActionCamera);

class NB_CCOrbitCamera : TNitClass<CCOrbitCamera>
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
			CONS_ENTRY_H(				"(time, radius, deltaRadius, angleZ, deltaAngleZ, angleX, deltaAngleX: float)"),
			FUNC_ENTRY_H(sphericalRadius, "(): {radius:float, zenith:float, azimuth:float}"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCOrbitCamera::actionWithDuration(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4), getFloat(v, 5), getFloat(v, 6), getFloat(v, 7), getFloat(v, 8))); return 0; }
	NB_FUNC(sphericalRadius)
	{
		float r, zenith, azimuth;
		self(v)->sphericalRadius(&r, &zenith, &azimuth);
		sq_newtable(v);
		newSlot(v, -1, "radius", r);
		newSlot(v, -1, "zenith", r);
		newSlot(v, -1, "azimuth", r);
		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseRateAction, CCActionEase);

class NB_CCEaseRateAction : TNitClass<CCEaseRateAction>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(rate),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(rate)					{ return push(v, self(v)->getRate()); }
	NB_PROP_SET(rate)					{ self(v)->setRate(getFloat(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseIn, CCEaseRateAction);

class NB_CCEaseIn : TNitClass<CCEaseIn>
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
			CONS_ENTRY_H(				"(action: cc.ActionInterval, rate: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseIn::actionWithAction(get<CCActionInterval>(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseOut, CCEaseRateAction);

class NB_CCEaseOut : TNitClass<CCEaseOut>
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
			CONS_ENTRY_H(				"(action: cc.ActionInterval, rate: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseOut::actionWithAction(get<CCActionInterval>(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseInOut, CCEaseRateAction);

class NB_CCEaseInOut : TNitClass<CCEaseInOut>
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
			CONS_ENTRY_H(				"(action: cc.ActionInterval, rate: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseInOut::actionWithAction(get<CCActionInterval>(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseExponentialIn, CCActionEase);

class NB_CCEaseExponentialIn : TNitClass<CCEaseExponentialIn>
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
			CONS_ENTRY_H(				"(action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseExponentialIn::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseExponentialOut, CCActionEase);

class NB_CCEaseExponentialOut : TNitClass<CCEaseExponentialOut>
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
			CONS_ENTRY_H(				"(action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseExponentialOut::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseExponentialInOut, CCActionEase);

class NB_CCEaseExponentialInOut : TNitClass<CCEaseExponentialInOut>
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
			CONS_ENTRY_H(				"(action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseExponentialInOut::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseSineIn, CCActionEase);

class NB_CCEaseSineIn : TNitClass<CCEaseSineIn>
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
			CONS_ENTRY_H(				"(action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseSineIn::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseSineOut, CCActionEase);

class NB_CCEaseSineOut : TNitClass<CCEaseSineOut>
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
			CONS_ENTRY_H(				"(action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseSineOut::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseSineInOut, CCActionEase);

class NB_CCEaseSineInOut : TNitClass<CCEaseSineInOut>
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
			CONS_ENTRY_H(				"(action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseSineInOut::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseElastic, CCActionEase);

class NB_CCEaseElastic : TNitClass<CCEaseElastic>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(period),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(period)					{ return push(v, self(v)->getPeriod()); }

	NB_PROP_SET(period)					{ self(v)->setPeriod(getFloat(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseElasticIn, CCEaseElastic);

class NB_CCEaseElasticIn : TNitClass<CCEaseElasticIn>
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
			CONS_ENTRY_H(				"(Action: cc.ActionInterval)\n"
										"(Action: cc.ActionInterval, Period: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()						
	{
		if(isNone(v, 3))
			setSelf(v, CCEaseElasticIn::actionWithAction(get<CCActionInterval>(v, 2)));
		else
			setSelf(v, CCEaseElasticIn::actionWithAction(get<CCActionInterval>(v, 2), getFloat(v, 3)));
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseElasticOut, CCEaseElastic);

class NB_CCEaseElasticOut : TNitClass<CCEaseElasticOut>
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
			CONS_ENTRY_H(				"(Action: cc.ActionInterval)\n"
										"(Action: cc.ActionInterval, Period: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()						
	{
		if(isNone(v, 3))
			setSelf(v, CCEaseElasticOut::actionWithAction(get<CCActionInterval>(v, 2)));
		else
			setSelf(v, CCEaseElasticOut::actionWithAction(get<CCActionInterval>(v, 2), getFloat(v, 3)));
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseElasticInOut, CCEaseElastic);

class NB_CCEaseElasticInOut : TNitClass<CCEaseElasticInOut>
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
			CONS_ENTRY_H(				"(Action: cc.ActionInterval)\n"
										"(Action: cc.ActionInterval, Period: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()						
	{
		if(isNone(v, 3))
			setSelf(v, CCEaseElasticInOut::actionWithAction(get<CCActionInterval>(v, 2)));
		else
			setSelf(v, CCEaseElasticInOut::actionWithAction(get<CCActionInterval>(v, 2), getFloat(v, 3)));
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseBounce, CCActionEase);

class NB_CCEaseBounce : TNitClass<CCEaseBounce>
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
			FUNC_ENTRY_H(bounceTime,	"(time: float): float"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(bounceTime)					{ return push(v, self(v)->bounceTime(getFloat(v, 2))); return 0;}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseBounceIn, CCEaseBounce);

class NB_CCEaseBounceIn : TNitClass<CCEaseBounceIn>
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
			CONS_ENTRY_H(				"(Action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseBounceIn::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseBounceOut, CCEaseBounce);

class NB_CCEaseBounceOut : TNitClass<CCEaseBounceOut>
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
			CONS_ENTRY_H(				"(Action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseBounceOut::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseBounceInOut, CCEaseBounce);

class NB_CCEaseBounceInOut : TNitClass<CCEaseBounceInOut>
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
			CONS_ENTRY_H(				"(Action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseBounceInOut::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseBackIn, CCActionEase);

class NB_CCEaseBackIn : TNitClass<CCEaseBackIn>
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
			CONS_ENTRY_H(				"(Action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseBackIn::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseBackOut, CCActionEase);

class NB_CCEaseBackOut : TNitClass<CCEaseBackOut>
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
			CONS_ENTRY_H(				"(Action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseBackOut::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EaseBackInOut, CCActionEase);

class NB_CCEaseBackInOut : TNitClass<CCEaseBackInOut>
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
			CONS_ENTRY_H(				"(Action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCEaseBackInOut::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::GridAction, CCActionInterval);

class NB_CCGridAction : TNitClass<CCGridAction>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(grid),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(grid)					{ return push(v, self(v)->getGrid()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Grid3DAction, CCGridAction);

class NB_CCGrid3DAction : TNitClass<CCGrid3DAction>
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
			//FUNC_ENTRY_H(vertex,		"(pos: cc.GridSize): cc.Vertex3F"),
			//FUNC_ENTRY_H(originalVertex,"(pos: cc.GridSize): cc.Vertex3F"),
			//FUNC_ENTRY_H(setVertex,		"(pos: cc.GridSize, vertex: cc.Vertex3F)"),
			NULL
		};

		bind(v, props, funcs);
	}

	//NB_FUNC(vertex)					{ return Push(v, Self(v)->vertex(*Get<ccGridSize>(v, 2)));return 0; }
	//NB_FUNC(originalVertex)			{ return Push(v, Self(v)->originalVertex(*Get<ccGridSize>(v, 2))); return 0;}
	//NB_FUNC(setVertex)				{ Self(v)->setVertex(*Get<ccGridSize>(v, 2), *Get<ccVertex3F>(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::TiledGrid3DAction, CCGridAction);

class NB_CCTiledGrid3DAction : TNitClass<CCTiledGrid3DAction>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		// NOTHING TO DO
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

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::AccelDeccelAmplitude, CCActionInterval);

class NB_CCAccelDeccelAmplitude : TNitClass<CCAccelDeccelAmplitude>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(rate),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(action: cc.Action, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(rate)					{ return push(v, self(v)->getRate()); }

	NB_PROP_SET(rate)					{ self(v)->setRate(getFloat(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCAccelDeccelAmplitude::actionWithAction(get<CCAction>(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::AccelAmplitude, CCActionInterval);

class NB_CCAccelAmplitude : TNitClass<CCAccelAmplitude>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(rate),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(action: cc.Action, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(rate)					{ return push(v, self(v)->getRate()); }

	NB_PROP_SET(rate)					{ self(v)->setRate(getFloat(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCAccelAmplitude::actionWithAction(get<CCAction>(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::DeccelAmplitude, CCActionInterval);

class NB_CCDeccelAmplitude : TNitClass<CCDeccelAmplitude>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(rate),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(action: cc.Action, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(rate)					{ return push(v, self(v)->getRate()); }

	NB_PROP_SET(rate)					{ self(v)->setRate(getFloat(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCDeccelAmplitude::actionWithAction(get<CCAction>(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::StopGrid, CCActionInstant);

class NB_CCStopGrid : TNitClass<CCStopGrid>
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

	NB_CONS()							{ setSelf(v, CCStopGrid::action()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::ReuseGrid, CCActionInstant);

class NB_CCReuseGrid : TNitClass<CCReuseGrid>
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
			CONS_ENTRY_H(				"(times: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCReuseGrid::actionWithTimes(getInt(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Waves3D, CCGrid3DAction);

class NB_CCWaves3D : TNitClass<CCWaves3D>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(amplitude),
			PROP_ENTRY	(amplitudeRate),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(wav: int, amp: float, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}
	
	NB_PROP_GET(amplitude)				{ return push(v, self(v)->getAmplitude()); }
	NB_PROP_GET(amplitudeRate)			{ return push(v, self(v)->getAmplitudeRate()); }

	NB_PROP_SET(amplitude)				{ self(v)->setAmplitude(getFloat(v, 2)); return 0; }
	NB_PROP_SET(amplitudeRate)			{ self(v)->setAmplitudeRate(getFloat(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCWaves3D::actionWithWaves(getInt(v, 2), getFloat(v, 3),*get<ccGridSize>(v, 4), getFloat(v, 5))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FlipX3D, CCGrid3DAction);

class NB_CCFlipX3D : TNitClass<CCFlipX3D>
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
			CONS_ENTRY_H(				"(duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCFlipX3D::actionWithDuration(getFloat(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FlipY3D, CCFlipX3D);

class NB_CCFlipY3D : TNitClass<CCFlipY3D>
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
			CONS_ENTRY_H(				"(duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}
	NB_CONS()							{ setSelf(v, CCFlipY3D::actionWithDuration(getFloat(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Lens3D, CCGrid3DAction);

class NB_CCLens3D : TNitClass<CCLens3D>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(lensEffect),
			PROP_ENTRY	(position),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(pos: cc.Point, r: float, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}
	NB_PROP_GET(lensEffect)				{ return push(v, self(v)->getLensEffect()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->getPosition()); }

	NB_PROP_SET(lensEffect)				{ self(v)->setLensEffect(getFloat(v, 2)); return 0; }
	NB_PROP_SET(position)				{ self(v)->setPosition(*get<CCPoint>(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCLens3D::actionWithPosition(*get<CCPoint>(v, 2), getFloat(v, 3),*get<ccGridSize>(v, 4), getFloat(v, 5))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Ripple3D, CCGrid3DAction);

class NB_CCRipple3D : TNitClass<CCRipple3D>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{			
			PROP_ENTRY	(position),
			PROP_ENTRY	(amplitude),
			PROP_ENTRY	(amplitudeRate),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(pos: cc.Point, r: float, wav: float, amp: float, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}
	NB_PROP_GET(position)				{ return push(v, self(v)->getPosition()); }
	NB_PROP_GET(amplitude)				{ return push(v, self(v)->getAmplitude()); }
	NB_PROP_GET(amplitudeRate)			{ return push(v, self(v)->getAmplitudeRate()); }

	NB_PROP_SET(position)				{ self(v)->setPosition(*get<CCPoint>(v, 2)); return 0; }
	NB_PROP_SET(amplitude)				{ self(v)->setAmplitude(getFloat(v, 2)); return 0; }
	NB_PROP_SET(amplitudeRate)			{ self(v)->setAmplitudeRate(getFloat(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCRipple3D::actionWithPosition(*get<CCPoint>(v, 2), getFloat(v, 3),getInt(v, 4), getFloat(v, 5), *get<ccGridSize>(v, 6), getFloat(v, 7))); return 0; }

};


////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Shaky3D, CCGrid3DAction);

class NB_CCShaky3D : TNitClass<CCShaky3D>
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
			CONS_ENTRY_H(				"(range: int, shakeZ: bool, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCShaky3D::actionWithRange(getInt(v, 2), getBool(v, 3), *get<ccGridSize>(v, 4), getFloat(v, 5))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Liquid, CCGrid3DAction);

class NB_CCLiquid : TNitClass<CCLiquid>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(amplitude),
			PROP_ENTRY	(amplitudeRate),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(wav: int, amp: float, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}
	NB_PROP_GET(amplitude)				{ return push(v, self(v)->getAmplitude()); }
	NB_PROP_GET(amplitudeRate)			{ return push(v, self(v)->getAmplitudeRate()); }

	NB_PROP_SET(amplitude)				{ self(v)->setAmplitude(getFloat(v, 2)); return 0; }
	NB_PROP_SET(amplitudeRate)			{ self(v)->setAmplitudeRate(getFloat(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCLiquid::actionWithWaves(getInt(v, 2), getFloat(v, 3), *get<ccGridSize>(v, 4), getFloat(v, 5))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Waves, CCGrid3DAction);

class NB_CCWaves : TNitClass<CCWaves>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(amplitude),
			PROP_ENTRY	(amplitudeRate),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(wav: int, amp: float, h: bool, v: bool, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(amplitude)				{ return push(v, self(v)->getAmplitude()); }
	NB_PROP_GET(amplitudeRate)			{ return push(v, self(v)->getAmplitudeRate()); }

	NB_PROP_SET(amplitude)				{ self(v)->setAmplitude(getFloat(v, 2)); return 0; }
	NB_PROP_SET(amplitudeRate)			{ self(v)->setAmplitudeRate(getFloat(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCWaves::actionWithWaves(getInt(v, 2), getFloat(v, 3), getBool(v, 4), getBool(v, 5), *get<ccGridSize>(v, 6), getFloat(v, 7))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Twirl, CCGrid3DAction);

class NB_CCTwirl : TNitClass<CCTwirl>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(position),
			PROP_ENTRY	(amplitude),
			PROP_ENTRY	(amplitudeRate),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(pos: cc.Point, t: int, amp: float, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(position)				{ return push(v, self(v)->getPosition()); }
	NB_PROP_GET(amplitude)				{ return push(v, self(v)->getAmplitude()); }
	NB_PROP_GET(amplitudeRate)			{ return push(v, self(v)->getAmplitudeRate()); }

	NB_PROP_SET(position)				{ self(v)->setPosition(*get<CCPoint>(v, 2)); return 0; }
	NB_PROP_SET(amplitude)				{ self(v)->setAmplitude(getFloat(v, 2)); return 0; }
	NB_PROP_SET(amplitudeRate)			{ self(v)->setAmplitudeRate(getFloat(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCTwirl::actionWithPosition(*get<CCPoint>(v, 2), getInt(v, 3),getFloat(v, 4), *get<ccGridSize>(v, 5), getFloat(v, 6))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Show, CCActionInstant);

class NB_CCShow : TNitClass<CCShow>
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

	NB_CONS()							{ setSelf(v, CCShow::action()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Hide, CCActionInstant);

class NB_CCHide : TNitClass<CCHide>
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

	NB_CONS()							{ setSelf(v, CCHide::action()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::ToggleVisibility, CCActionInstant);

class NB_CCToggleVisibility : TNitClass<CCToggleVisibility>
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

	NB_CONS()							{ setSelf(v, CCToggleVisibility::action()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FlipX, CCActionInstant);

class NB_CCFlipX : TNitClass<CCFlipX>
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
			CONS_ENTRY_H(				"(x: bool)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCFlipX::actionWithFlipX(getBool(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FlipY, CCActionInstant);

class NB_CCFlipY : TNitClass<CCFlipY>
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
			CONS_ENTRY_H(				"(y: bool)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCFlipY::actionWithFlipY(getBool(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Place, CCActionInstant);

class NB_CCPlace : TNitClass<CCPlace>
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
			CONS_ENTRY_H(				"(pos: cc.Point)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCPlace::actionWithPosition(*get<CCPoint>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::EventCall, CCActionInstant);

class NB_CCEventCall : TNitClass<CCEventCall>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(event),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(evt: Event, handler: EventHandler)\n"
										"(evt: Event, sink, closure)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(event)					{ return push(v, self(v)->getEvent()); }
	
	NB_CONS()							{ setSelf(v, CCEventCall::actionWithTarget(opt<Event>(v, 2, NULL), ScriptEventHandler::get(v, 3, 4))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Sequence, CCActionInterval);

class NB_CCSequence : TNitClass<CCSequence>
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
			CONS_ENTRY_H(				"(act1, act2, ...: cc.FiniteTimeAction)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		int top = sq_gettop(v);

		if (sq_gettype(v, 2) == OT_ARRAY)
		{
			CCFiniteTimeAction* now = NULL;
			CCFiniteTimeAction* prev = NULL;

			for (NitIterator itr(v, -1); itr.hasNext(); itr.next())
			{
				if (!is<CCFiniteTimeAction>(v, -1))
					return sq_throwerror(v, "cc.FiniteTimeAction expected");	

				if (!prev)
				{
					prev = get<CCFiniteTimeAction>(v, -1);
					continue;
				}
				
				now = get<CCFiniteTimeAction>(v, -1);
 				prev = CCSequence::actionOneTwo(prev, now);			
			}
			
			if (!now)
				return sq_throwerror(v, "at least two cc.FiniteTimeAction expected");	
			
			setSelf(v, static_cast<CCSequence*>(prev));
			return 0;
		}

		if (top < 3)
			return sq_throwerror(v, "at least two cc.FiniteTimeAction expected");

		// Check all parameters in advance
		for (int i = 2; i <= top; ++i)
			if (!is<CCFiniteTimeAction>(v, i))
				return sq_throwerror(v, "cc.FiniteTimeAction expected");

		CCFiniteTimeAction* now;
		CCFiniteTimeAction* prev = get<CCFiniteTimeAction>(v, 2);

		int z = 0;
		for (int i = 3; i <= top; ++i)
		{
			now = get<CCFiniteTimeAction>(v, i);
			prev = CCSequence::actionOneTwo(prev, now);
		}

		setSelf(v, static_cast<CCSequence*>(prev));
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Repeat, CCActionInterval);

class NB_CCRepeat : TNitClass<CCRepeat>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(innerAction),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(inner: cc.FiniteTimeAction, times: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(innerAction)			{ return push(v, self(v)->getInnerAction()); }

	NB_PROP_SET(innerAction)			{ self(v)->setInnerAction(get<CCFiniteTimeAction>(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCRepeat::actionWithAction(get<CCFiniteTimeAction>(v, 2), getInt(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::RepeatForever, CCActionInterval);

class NB_CCRepeatForever : TNitClass<CCRepeatForever>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(innerAction),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(Action: cc.ActionInterval)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(innerAction)			{ return push(v, self(v)->getInnerAction()); }

	NB_PROP_SET(innerAction)			{ self(v)->setInnerAction(get<CCActionInterval>(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCRepeatForever::actionWithAction(get<CCActionInterval>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Spawn, CCActionInterval);

class NB_CCSpawn : TNitClass<CCSpawn>
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
			CONS_ENTRY_H(				"(act1, act2, ...: cc.FiniteTimeAction)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		int top = sq_gettop(v);

		if (sq_gettype(v, 2) == OT_ARRAY)
		{
			CCFiniteTimeAction* now = NULL;
			CCFiniteTimeAction* prev = NULL;

			for (NitIterator itr(v, -1); itr.hasNext(); itr.next())
			{
				if (!is<CCFiniteTimeAction>(v, -1))
					return sq_throwerror(v, "cc.FiniteTimeAction expected");	

				if (!prev)
				{
					prev = get<CCFiniteTimeAction>(v, -1);
					continue;
				}

				now = get<CCFiniteTimeAction>(v, -1);
				prev = CCSpawn::actionOneTwo(prev, now);			
			}

			if (!now)
				return sq_throwerror(v, "at least two cc.FiniteTimeAction expected");	

			setSelf(v, static_cast<CCSpawn*>(prev));
			return 0;
		}

		if (top < 3)
			return sq_throwerror(v, "at least two cc.FiniteTimeAction expected");

		// Check all parameters in advance
		for (int i = 2; i <= top; ++i)
			if (!is<CCFiniteTimeAction>(v, i))
				return sq_throwerror(v, "cc.FiniteTimeAction expected");

		CCFiniteTimeAction* now;
		CCFiniteTimeAction* prev = get<CCFiniteTimeAction>(v, 2);

		int z = 0;
		for (int i = 3; i <= top; ++i)
		{
			now = get<CCFiniteTimeAction>(v, i);
			prev = CCSpawn::actionOneTwo(prev, now);
		}

		setSelf(v, static_cast<CCSpawn*>(prev));
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::RotateTo, CCActionInterval);

class NB_CCRotateTo : TNitClass<CCRotateTo>
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
			CONS_ENTRY_H(				"(duration: float, DeltaAngle: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCRotateTo::actionWithDuration(getFloat(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::RotateBy, CCActionInterval);

class NB_CCRotateBy : TNitClass<CCRotateBy>
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
			CONS_ENTRY_H(				"(duration: float, DeltaAngle: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCRotateBy::actionWithDuration(getFloat(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::MoveTo, CCActionInterval);

class NB_CCMoveTo : TNitClass<CCMoveTo>
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
			CONS_ENTRY_H(				"(duration: float, position: cc.Point)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCMoveTo::actionWithDuration(getFloat(v, 2), *get<CCPoint>(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::MoveBy, CCMoveTo);

class NB_CCMoveBy : TNitClass<CCMoveBy>
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
			CONS_ENTRY_H(				"(duration: float, position: cc.Point)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCMoveBy::actionWithDuration(getFloat(v, 2), *get<CCPoint>(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::SkewTo, CCActionInterval);

class NB_CCSkewTo : TNitClass<CCSkewTo>
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
			CONS_ENTRY_H(				"(t: float, sx: float, sy: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCSkewTo::actionWithDuration(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::SkewBy, CCSkewTo);

class NB_CCSkewBy : TNitClass<CCSkewBy>
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
			CONS_ENTRY_H(				"(t: float, deltaSkewX: float, deltaSkewY: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCSkewBy::actionWithDuration(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::JumpBy, CCActionInterval);

class NB_CCJumpBy : TNitClass<CCJumpBy>
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
			CONS_ENTRY_H(				"(duration:float, pos:CCPoint, height:float, jumps:int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		setSelf(v, CCJumpBy::actionWithDuration(getFloat(v, 2), *get<CCPoint>(v, 3), getFloat(v, 4), getInt(v, 5)) );
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::JumpTo, CCJumpBy);

class NB_CCJumpTo : TNitClass<CCJumpTo>
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
			CONS_ENTRY_H(				"(duration:float, pos:CCPoint, height:float, jumps:int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		setSelf(v, CCJumpTo::actionWithDuration(getFloat(v, 2), *get<CCPoint>(v, 3), getFloat(v, 4), getInt(v, 5)) );
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::BezierBy, CCActionInterval);

class NB_CCBezierBy : TNitClass<CCBezierBy>
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
			CONS_ENTRY_H(				"(t:ccTime, endPosition,controlPoint_1,controlPoint_2 :CCPoint)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{ 
		ccBezierConfig config;
		config.endPosition = *get<CCPoint>(v, 3);
		config.controlPoint_1 = *get<CCPoint>(v, 4);
		config.controlPoint_2 = *get<CCPoint>(v, 5);
		setSelf(v, CCBezierBy::actionWithDuration(getFloat(v, 2), config)  );
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::BezierTo, CCBezierBy);

class NB_CCBezierTo : TNitClass<CCBezierTo>
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
			CONS_ENTRY_H(				"(t:ccTime, endPosition,controlPoint_1,controlPoint_2 :CCPoint)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{ 
		ccBezierConfig config;
		config.endPosition = *get<CCPoint>(v, 3);
		config.controlPoint_1 = *get<CCPoint>(v, 4);
		config.controlPoint_2 = *get<CCPoint>(v, 5);
		setSelf(v, CCBezierTo::actionWithDuration(getFloat(v, 2), config)  );
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::ScaleTo, CCActionInterval);

class NB_CCScaleTo : TNitClass<CCScaleTo>
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
			CONS_ENTRY_H(				"(duration:float, s:float)\n"
										"(duration:float, sx, sy:float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 4))
			setSelf(v, CCScaleTo::actionWithDuration(getFloat(v, 2), getFloat(v, 3)) );
		else
			setSelf(v, CCScaleTo::actionWithDuration(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4)) );
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::ScaleBy, CCScaleTo);

class NB_CCScaleBy : TNitClass<CCScaleBy>
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
			CONS_ENTRY_H(				"(duration:float, s:float)\n"
										"(duration:float, sx, sy:float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (isNone(v, 4))
			setSelf(v, CCScaleBy::actionWithDuration(getFloat(v, 2), getFloat(v, 3)) );
		else
			setSelf(v, CCScaleBy::actionWithDuration(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4)) );
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Blink, CCActionInterval);

class NB_CCBlink : TNitClass<CCBlink>
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
			CONS_ENTRY_H(				"(duration:float, uBlinks:int)\n"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		setSelf(v, CCBlink::actionWithDuration(getFloat(v, 2), getInt(v, 3)) );
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FadeIn, CCActionInterval);

class NB_CCFadeIn : TNitClass<CCFadeIn>
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
			CONS_ENTRY_H(				"(duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCFadeIn::actionWithDuration(getFloat(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FadeOut, CCActionInterval);

class NB_CCFadeOut : TNitClass<CCFadeOut>
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
			CONS_ENTRY_H(				"(duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCFadeOut::actionWithDuration(getFloat(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FadeTo, CCActionInterval);

class NB_CCFadeTo : TNitClass<CCFadeTo>
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
			CONS_ENTRY_H(				"(duration: float, opacity: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCFadeTo::actionWithDuration(getFloat(v, 2), (GLubyte)getInt(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::TintTo, CCActionInterval);

class NB_CCTintTo : TNitClass<CCTintTo>
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
			CONS_ENTRY_H(				"(duration: float, red, green, blue: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTintTo::actionWithDuration(getFloat(v, 2), (GLubyte)getInt(v, 3), (GLubyte)getInt(v, 4), (GLubyte)getInt(v, 5))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::TintBy, CCActionInterval);

class NB_CCTintBy : TNitClass<CCTintBy>
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
			CONS_ENTRY_H(				"(duration: float, deltaRed, deltaGreen, deltaBlue: int)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTintBy::actionWithDuration(getFloat(v, 2), (GLshort)getInt(v, 3), (GLshort)getInt(v, 4), (GLshort)getInt(v, 5))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::DelayTime, CCActionInterval);

class NB_CCDelayTime : TNitClass<CCDelayTime>
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
			CONS_ENTRY_H(				"(d: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCDelayTime::actionWithDuration(getFloat(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::ReverseTime, CCActionInterval);

class NB_CCReverseTime : TNitClass<CCReverseTime>
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
			CONS_ENTRY_H(				"(Action: cc.FiniteTimeAction)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCReverseTime::actionWithAction(get<CCFiniteTimeAction>(v, 2))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::Animate, CCActionInterval);

class NB_CCAnimate : TNitClass<CCAnimate>
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
			CONS_ENTRY_H(				"(Animation: cc.Animation)\n"
										"(Animation: cc.Animation, RestoreOriginalFrame: bool)\n"
										"(duration: float, Animation: cc.Animation, RestoreOriginalFrame: bool)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()						
	{
		if (isNone(v, 3))
			setSelf(v, CCAnimate::actionWithAnimation(get<CCAnimation>(v, 2)));
		else if (isNone(v, 4))
			setSelf(v, CCAnimate::actionWithAnimation(get<CCAnimation>(v, 2), getBool(v, 3)));
		else
			setSelf(v, CCAnimate::actionWithDuration(getFloat(v, 2), get<CCAnimation>(v, 3), getBool(v, 4)));
		return 0; 
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::PageTurn3D, CCGrid3DAction);

class NB_CCPageTurn3D : TNitClass<CCPageTurn3D>
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
			CONS_ENTRY_H(				"(gridSize: cc.GridSize, time: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCPageTurn3D::actionWithSize(*get<ccGridSize>(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::ProgressTo, CCActionInterval);

class NB_CCProgressTo : TNitClass<CCProgressTo>
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
			CONS_ENTRY_H(				"(duration: float, Percent: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCProgressTo::actionWithDuration(getFloat(v, 2), getFloat(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::ProgressFromTo, CCActionInterval);

class NB_CCProgressFromTo : TNitClass<CCProgressFromTo>
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
			CONS_ENTRY_H(				"(duration, fFromPercentage,fToPercentage : float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCProgressFromTo::actionWithDuration(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::ShakyTiles3D, CCTiledGrid3DAction);

class NB_CCShakyTiles3D : TNitClass<CCShakyTiles3D>
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
			CONS_ENTRY_H(				"(nRange: int, bShatterZ: bool, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCShakyTiles3D::actionWithRange(getInt(v, 2), getBool(v, 3),*get<ccGridSize>(v, 4), getFloat(v, 5))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::ShatteredTiles3D, CCTiledGrid3DAction);

class NB_CCShatteredTiles3D : TNitClass<CCShatteredTiles3D>
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
			CONS_ENTRY_H(				"(nRange: int, bShatterZ: bool, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}
	
	NB_CONS()							{ setSelf(v, CCShatteredTiles3D::actionWithRange(getInt(v, 2), getBool(v, 3),*get<ccGridSize>(v, 4), getFloat(v, 5))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::ShuffleTiles, CCTiledGrid3DAction);

class NB_CCShuffleTiles : TNitClass<CCShuffleTiles>
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
			CONS_ENTRY_H(				"(s: int, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}
	NB_CONS()							{ setSelf(v, CCShuffleTiles::actionWithSeed(getInt(v, 2),*get<ccGridSize>(v, 3), getFloat(v, 4))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FadeOutTRTiles, CCTiledGrid3DAction);

class NB_CCFadeOutTRTiles : TNitClass<CCFadeOutTRTiles>
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
			CONS_ENTRY_H(				"(gridSize: cc.GridSize, time: float))"),
			FUNC_ENTRY_H(turnOnTile,	"(gridSize: cc.GridSize)"),
			FUNC_ENTRY_H(turnOffTile,	"(gridSize: cc.GridSize)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(turnOnTile)					{ self(v)->turnOnTile(*get<ccGridSize>(v, 2)); return 0; }
	NB_FUNC(turnOffTile)				{ self(v)->turnOffTile(*get<ccGridSize>(v, 2)); return 0; }
	NB_CONS()							{ setSelf(v, CCFadeOutTRTiles::actionWithSize(*get<ccGridSize>(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FadeOutBLTiles, CCFadeOutTRTiles);

class NB_CCFadeOutBLTiles : TNitClass<CCFadeOutBLTiles>
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
			CONS_ENTRY_H(				"(gridSize: cc.GridSize, time: float))"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCFadeOutBLTiles::actionWithSize(*get<ccGridSize>(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FadeOutUpTiles, CCFadeOutTRTiles);

class NB_CCFadeOutUpTiles : TNitClass<CCFadeOutUpTiles>
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
			CONS_ENTRY_H(				"(gridSize: cc.GridSize, time: float))"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCFadeOutUpTiles::actionWithSize(*get<ccGridSize>(v, 2), getFloat(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::FadeOutDownTiles, CCFadeOutUpTiles);

class NB_CCFadeOutDownTiles : TNitClass<CCFadeOutDownTiles>
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
			CONS_ENTRY_H(				"(gridSize: cc.GridSize, time: float))"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCFadeOutDownTiles::actionWithSize(*get<ccGridSize>(v, 2), getFloat(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::TurnOffTiles, CCTiledGrid3DAction);

class NB_CCTurnOffTiles : TNitClass<CCTurnOffTiles>
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
			CONS_ENTRY_H(				"(gridSize: cc.GridSize, time: float)\n"
										"(s: int, gridSize: cc.GridSize, time: float)"),
			FUNC_ENTRY_H(turnOnTile,	"(gridSize: cc.GridSize)"),
			FUNC_ENTRY_H(turnOffTile,	"(gridSize: cc.GridSize)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(turnOnTile)					{ self(v)->turnOnTile(*get<ccGridSize>(v, 2)); return 0; }
	NB_FUNC(turnOffTile)				{ self(v)->turnOffTile(*get<ccGridSize>(v, 2)); return 0; }

	NB_CONS()	
	{
		if (is<ccGridSize>(v, 2))
		{
			setSelf(v, CCTurnOffTiles::actionWithSize(*get<ccGridSize>(v, 2), getFloat(v, 3)));
		}
		else 
		{
			setSelf(v, CCTurnOffTiles::actionWithSeed(getInt(v, 2), *get<ccGridSize>(v, 3), getFloat(v, 4)));
		}
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::WavesTiles3D, CCTiledGrid3DAction);

class NB_CCWavesTiles3D : TNitClass<CCWavesTiles3D>
{
public:

	static void Register(HSQUIRRELVM v)
	{	
		PropEntry props[] =
		{
			PROP_ENTRY	(amplitude),
			PROP_ENTRY	(amplitudeRate),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(wav: int, amp: float, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(amplitude)				{ return push(v, self(v)->getAmplitude()); }
	NB_PROP_GET(amplitudeRate)			{ return push(v, self(v)->getAmplitudeRate()); }

	NB_PROP_SET(amplitude)				{ self(v)->setAmplitude(getFloat(v, 2)); return 0; }
	NB_PROP_SET(amplitudeRate)			{ self(v)->setAmplitudeRate(getFloat(v, 2)); return 0; }
	
	NB_CONS()							{ setSelf(v, CCWavesTiles3D::actionWithWaves(getInt(v, 2), getFloat(v, 3),*get<ccGridSize>(v, 4), getFloat(v, 5))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::JumpTiles3D, CCTiledGrid3DAction);

class NB_CCJumpTiles3D : TNitClass<CCJumpTiles3D>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(amplitude),
			PROP_ENTRY	(amplitudeRate),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(j: int, amp: float, gridSize: cc.GridSize, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}
	NB_PROP_GET(amplitude)				{ return push(v, self(v)->getAmplitude()); }
	NB_PROP_GET(amplitudeRate)			{ return push(v, self(v)->getAmplitudeRate()); }

	NB_PROP_SET(amplitude)				{ self(v)->setAmplitude(getFloat(v, 2)); return 0; }
	NB_PROP_SET(amplitudeRate)			{ self(v)->setAmplitudeRate(getFloat(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, CCJumpTiles3D::actionWithJumps(getInt(v, 2), getFloat(v, 3),*get<ccGridSize>(v, 4), getFloat(v, 5))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::SplitRows, CCTiledGrid3DAction);

class NB_CCSplitRows : TNitClass<CCSplitRows>
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
			CONS_ENTRY_H(				"(nRow: int, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}
	NB_CONS()							{ setSelf(v, CCSplitRows::actionWithRows(getInt(v, 2), getFloat(v, 3))); return 0; }
	
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::action::SplitCols, CCTiledGrid3DAction);

class NB_CCSplitCols : TNitClass<CCSplitCols>
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
			CONS_ENTRY_H(				"(nCols: int, duration: float)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCSplitCols::actionWithCols(getInt(v, 2), getFloat(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NIT2D_API SQRESULT NitLibCocosAction(HSQUIRRELVM v)
{
	NB_CCSpeed::Register(v);
	NB_CCFollow::Register(v);

	NB_CCOrbitCamera::Register(v);
	
	NB_CCEaseRateAction::Register(v);
	NB_CCEaseIn::Register(v);
	NB_CCEaseOut::Register(v);
	NB_CCEaseInOut::Register(v);
	NB_CCEaseExponentialIn::Register(v);
	NB_CCEaseExponentialOut::Register(v);
	NB_CCEaseExponentialInOut::Register(v);
	NB_CCEaseSineIn::Register(v);
	NB_CCEaseSineOut::Register(v);
	NB_CCEaseSineInOut::Register(v);
	NB_CCEaseElastic::Register(v);
	NB_CCEaseElasticIn::Register(v);
	NB_CCEaseElasticOut::Register(v);
	NB_CCEaseElasticInOut::Register(v);
	NB_CCEaseBounce::Register(v);
	NB_CCEaseBounceIn::Register(v);
	NB_CCEaseBounceOut::Register(v);
	NB_CCEaseBounceInOut::Register(v);
	NB_CCEaseBackIn::Register(v);
	NB_CCEaseBackOut::Register(v);
	NB_CCEaseBackInOut::Register(v);


	NB_CCGridAction::Register(v);
	NB_CCGrid3DAction::Register(v);
	NB_CCTiledGrid3DAction::Register(v);
	NB_CCAccelDeccelAmplitude::Register(v);
	NB_CCAccelAmplitude::Register(v);
	NB_CCDeccelAmplitude::Register(v);
	NB_CCStopGrid::Register(v);
	NB_CCReuseGrid::Register(v);	

	NB_CCWaves3D::Register(v);
	NB_CCFlipX3D::Register(v);
	NB_CCFlipY3D::Register(v);
	NB_CCLens3D::Register(v);
	NB_CCRipple3D::Register(v);
	NB_CCShaky3D::Register(v);
	NB_CCLiquid::Register(v);
	NB_CCWaves::Register(v);
	NB_CCTwirl::Register(v);

	NB_CCShow::Register(v);
	NB_CCHide::Register(v);
	NB_CCToggleVisibility::Register(v);
	NB_CCFlipX::Register(v);
	NB_CCFlipY::Register(v);
	NB_CCPlace::Register(v);
	NB_CCEventCall::Register(v);

	NB_CCSequence::Register(v);
	NB_CCRepeat::Register(v);
	NB_CCRepeatForever::Register(v);
	NB_CCSpawn::Register(v);
	NB_CCRotateTo::Register(v);
	NB_CCRotateBy::Register(v);
	NB_CCMoveTo::Register(v);
	NB_CCMoveBy::Register(v);
	NB_CCSkewTo::Register(v);
	NB_CCSkewBy::Register(v);
	NB_CCJumpBy::Register(v);
	NB_CCJumpTo::Register(v);
	NB_CCBezierBy::Register(v);
	NB_CCBezierTo::Register(v);
	NB_CCScaleTo::Register(v);
	NB_CCScaleBy::Register(v);
	NB_CCBlink::Register(v);
	NB_CCFadeIn::Register(v);
	NB_CCFadeOut::Register(v);
	NB_CCFadeTo::Register(v);
	NB_CCTintTo::Register(v);
	NB_CCTintBy::Register(v);
	NB_CCDelayTime::Register(v);
	NB_CCReverseTime::Register(v);
	NB_CCAnimate::Register(v);

	NB_CCPageTurn3D::Register(v);

	NB_CCProgressTo::Register(v);
	NB_CCProgressFromTo::Register(v);

	NB_CCShakyTiles3D::Register(v);
	NB_CCShatteredTiles3D::Register(v);
	NB_CCShuffleTiles::Register(v);
	NB_CCFadeOutTRTiles::Register(v);
	NB_CCFadeOutBLTiles::Register(v);
	NB_CCFadeOutUpTiles::Register(v);
	NB_CCFadeOutDownTiles::Register(v);
	NB_CCTurnOffTiles::Register(v);
	NB_CCWavesTiles3D::Register(v);
	NB_CCJumpTiles3D::Register(v);
	NB_CCSplitRows::Register(v);
	NB_CCSplitCols::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
