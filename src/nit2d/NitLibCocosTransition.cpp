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

////////////////////////////////////////////////////////////////////////////////

// Redeclare namespace as cc -> cc.transition

namespace cc
{
	namespace transition
	{
		// CCTransition.h
		typedef CCTransitionScene			Scene;
		typedef CCTransitionSceneOriented	SceneOriented;
		typedef CCTransitionRotoZoom		RotoZoom;
		typedef CCTransitionJumpZoom		JumpZoom;
		typedef CCTransitionMoveInL			MoveInL;
		typedef CCTransitionMoveInR			MoveInR;
		typedef CCTransitionMoveInT			MoveInT;
		typedef CCTransitionMoveInB			MoveInB;
		typedef CCTransitionSlideInL		SlideInL;
		typedef CCTransitionSlideInR		SlideInR;
		typedef CCTransitionSlideInT		SlideInT;
		typedef CCTransitionSlideInB		SlideInB;
		typedef CCTransitionShrinkGrow		ShrinkGrow;
		typedef CCTransitionFlipX			FlipX;
		typedef CCTransitionFlipY			FlipY;
		typedef CCTransitionFlipAngular		FlipAngular;
		typedef CCTransitionZoomFlipX		ZoomFlipX;
		typedef CCTransitionZoomFlipY		ZoomFlipY;
		typedef CCTransitionZoomFlipAngular	ZoomFlipAngular;
		typedef CCTransitionFade			Fade;
		typedef CCTransitionCrossFade		CrossFade;
		typedef CCTransitionTurnOffTiles	TurnOffTiles;
		typedef CCTransitionSplitCols		SplitCols;
		typedef CCTransitionSplitRows		SplitRows;
		typedef CCTransitionFadeTR			FadeTR;
		typedef CCTransitionFadeBL			FadeBL;
		typedef CCTransitionFadeUp			FadeUp;
		typedef CCTransitionFadeDown		FadeDown;

		// CCTransitionPageTurn.h
		typedef CCTransitionPageTurn		PageTurn;

		// CCTransitionRadial.h
		typedef CCTransitionRadialCCW		RadialCCW;
		typedef CCTransitionRadialCW		RadialCW;
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::Scene, CCScene);

class NB_CCTransitionScene : TNitClass<CCTransitionScene>
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

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::SceneOriented, CCTransitionScene);

class NB_CCTransitionSceneOriented : TNitClass<CCTransitionSceneOriented>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene, orientation)"),
			NULL
		};

		bind(v, props, funcs);

		addStatic(v, "ORIENT_LEFT_OVER", (int)kOrientationLeftOver);
		addStatic(v, "ORIENT_RIGHT_OVER", (int)kOrientationRightOver);
		addStatic(v, "ORIENT_UP_OVER", (int)kOrientationUpOver);
		addStatic(v, "ORIENT_DOWN_OVER", (int)kOrientationDownOver);
	}

	NB_CONS()							{ setSelf(v, CCTransitionSceneOriented::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3), (tOrientation)getInt(v, 4))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::RotoZoom, CCTransitionScene);

class NB_CCTransitionRotoZoom : TNitClass<CCTransitionRotoZoom>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionRotoZoom::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::JumpZoom, CCTransitionScene);

class NB_CCTransitionJumpZoom : TNitClass<CCTransitionJumpZoom>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}
	NB_CONS()							{ setSelf(v, CCTransitionJumpZoom::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::MoveInL, CCTransitionScene);

class NB_CCTransitionMoveInL : TNitClass<CCTransitionMoveInL>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(action),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(action)					{ return push(v, self(v)->action()); }

	NB_CONS()							{ setSelf(v, CCTransitionMoveInL::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::MoveInR, CCTransitionMoveInL);

class NB_CCTransitionMoveInR : TNitClass<CCTransitionMoveInR>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}
	
	NB_CONS()							{ setSelf(v, CCTransitionMoveInR::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::MoveInT, CCTransitionMoveInL);

class NB_CCTransitionMoveInT : TNitClass<CCTransitionMoveInT>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionMoveInT::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::MoveInB, CCTransitionMoveInL);

class NB_CCTransitionMoveInB : TNitClass<CCTransitionMoveInB>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionMoveInB::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::SlideInL, CCTransitionScene);

class NB_CCTransitionSlideInL : TNitClass<CCTransitionSlideInL>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(action),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(action)					{ return push(v, self(v)->action()); }

	NB_CONS()							{ setSelf(v, CCTransitionSlideInL::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::SlideInR, CCTransitionSlideInL);

class NB_CCTransitionSlideInR : TNitClass<CCTransitionSlideInR>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionSlideInR::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::SlideInT, CCTransitionSlideInL);

class NB_CCTransitionSlideInT : TNitClass<CCTransitionSlideInT>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionSlideInT::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::SlideInB, CCTransitionSlideInL);

class NB_CCTransitionSlideInB : TNitClass<CCTransitionSlideInB>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionSlideInB::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::ShrinkGrow, CCTransitionScene);

class NB_CCTransitionShrinkGrow : TNitClass<CCTransitionShrinkGrow>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionShrinkGrow::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::FlipX, CCTransitionSceneOriented);

class NB_CCTransitionFlipX : TNitClass<CCTransitionFlipX>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene, orientation)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionFlipX::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3), (tOrientation)optInt(v, 4,(int)kOrientationRightOver))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::FlipY, CCTransitionSceneOriented);

class NB_CCTransitionFlipY : TNitClass<CCTransitionFlipY>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene, orientation)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionFlipY::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3), (tOrientation)optInt(v, 4,(int)kOrientationUpOver))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::FlipAngular, CCTransitionSceneOriented);

class NB_CCTransitionFlipAngular : TNitClass<CCTransitionFlipAngular>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene, orientation)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionFlipAngular::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3), (tOrientation)optInt(v, 4,(int)kOrientationRightOver))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::ZoomFlipX, CCTransitionSceneOriented);

class NB_CCTransitionZoomFlipX : TNitClass<CCTransitionZoomFlipX>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene, orientation)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionZoomFlipX::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3), (tOrientation)optInt(v, 4,(int)kOrientationRightOver))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::ZoomFlipY, CCTransitionSceneOriented);

class NB_CCTransitionZoomFlipY : TNitClass<CCTransitionZoomFlipY>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene, orientation)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionZoomFlipY::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3), (tOrientation)optInt(v, 4,(int)kOrientationUpOver))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::ZoomFlipAngular, CCTransitionSceneOriented);

class NB_CCTransitionZoomFlipAngular : TNitClass<CCTransitionZoomFlipAngular>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene, orientation)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionZoomFlipAngular::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3), (tOrientation)optInt(v, 4,(int)kOrientationRightOver))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::Fade, CCTransitionScene);

class NB_CCTransitionFade : TNitClass<CCTransitionFade>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene, color: color)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionFade::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3), ToColor3B(*opt<Color>(v, 4,Color::Black)))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::CrossFade, CCTransitionScene);

class NB_CCTransitionCrossFade : TNitClass<CCTransitionCrossFade>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionCrossFade::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::TurnOffTiles, CCTransitionScene);

class NB_CCTransitionTurnOffTiles : TNitClass<CCTransitionTurnOffTiles>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionTurnOffTiles::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }


};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::SplitCols, CCTransitionScene);

class NB_CCTransitionSplitCols : TNitClass<CCTransitionSplitCols>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(action),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(action)					{ return push(v, self(v)->action()); }

	NB_CONS()							{ setSelf(v, CCTransitionSplitCols::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::SplitRows, CCTransitionScene);

class NB_CCTransitionSplitRows : TNitClass<CCTransitionSplitRows>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(action),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(action)					{ return push(v, self(v)->action()); }

	NB_CONS()							{ setSelf(v, CCTransitionSplitRows::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }


};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::FadeTR, CCTransitionScene);

class NB_CCTransitionFadeTR : TNitClass<CCTransitionFadeTR>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionFadeTR::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }


};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::FadeBL, CCTransitionFadeTR);

class NB_CCTransitionFadeBL : TNitClass<CCTransitionFadeBL>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionFadeBL::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }


};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::FadeUp, CCTransitionFadeTR);

class NB_CCTransitionFadeUp : TNitClass<CCTransitionFadeUp>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionFadeUp::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::FadeDown, CCTransitionFadeTR);

class NB_CCTransitionFadeDown : TNitClass<CCTransitionFadeDown>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionFadeDown::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::PageTurn, CCTransitionScene);

class NB_CCTransitionPageTurn : TNitClass<CCTransitionPageTurn>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene, backwards: bool)"),
			FUNC_ENTRY_H(actionWithSize,"(vector: cc.GridSize): cc.ActionInterval"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionPageTurn::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3), getBool(v, 4))); return 0; }
	NB_FUNC(actionWithSize)				{ return push(v, self(v)->actionWithSize(*get<ccGridSize>(v, 2))); }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::RadialCCW, CCTransitionScene);

class NB_CCTransitionRadialCCW : TNitClass<CCTransitionRadialCCW>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionRadialCCW::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::transition::RadialCW, CCTransitionRadialCCW);

class NB_CCTransitionRadialCW : TNitClass<CCTransitionRadialCW>
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
			CONS_ENTRY_H(				"(time: float, scene: cc.Scene)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCTransitionRadialCW::transitionWithDuration(getFloat(v, 2), get<CCScene>(v, 3))); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NIT2D_API SQRESULT NitLibCocosTransition(HSQUIRRELVM v)
{
	NB_CCTransitionScene::Register(v);
	NB_CCTransitionSceneOriented::Register(v);
	NB_CCTransitionRotoZoom::Register(v);
	NB_CCTransitionJumpZoom::Register(v);
	NB_CCTransitionMoveInL::Register(v);
	NB_CCTransitionMoveInR::Register(v);
	NB_CCTransitionMoveInT::Register(v);
	NB_CCTransitionMoveInB::Register(v);
	NB_CCTransitionSlideInL::Register(v);
	NB_CCTransitionSlideInR::Register(v);
	NB_CCTransitionSlideInT::Register(v);
	NB_CCTransitionSlideInB::Register(v);
	NB_CCTransitionShrinkGrow::Register(v);
	NB_CCTransitionFlipX::Register(v);
	NB_CCTransitionFlipY::Register(v);
	NB_CCTransitionFlipAngular::Register(v);
	NB_CCTransitionZoomFlipX::Register(v);
	NB_CCTransitionZoomFlipY::Register(v);
	NB_CCTransitionZoomFlipAngular::Register(v);
	NB_CCTransitionFade::Register(v);
	NB_CCTransitionCrossFade::Register(v);
	NB_CCTransitionTurnOffTiles::Register(v);
	NB_CCTransitionSplitCols::Register(v);
	NB_CCTransitionSplitRows::Register(v);
	NB_CCTransitionFadeTR::Register(v);
	NB_CCTransitionFadeBL::Register(v);
	NB_CCTransitionFadeUp::Register(v);
	NB_CCTransitionFadeDown::Register(v);
	NB_CCTransitionPageTurn::Register(v);
	NB_CCTransitionRadialCCW::Register(v);
	NB_CCTransitionRadialCW::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
