var pack = script.locator

TRANSITION_DURATION := 1.2

function fadeWhiteTransition(t, s)
{
	return cc.transition.Fade (t, s, nit.Color.WHITE)
}

function flipXLeftOver(t, s)
{
	return cc.transition.FlipX (t, s, 0)
}
function flipXRightOver(t, s)
{
	return cc.transition.FlipX (t, s, 1)
}

function flipYUpOver(t, s)
{
	return cc.transition.FlipY (t, s, 0)
}
function flipYDownOver(t, s)
{
	return cc.transition.FlipY (t, s, 1)
}

function flipAngularLeftOver(t, s)
{
	return cc.transition.FlipAngular(t, s, 0)
}
function flipAngularRightOver(t, s)
{
	return cc.transition.FlipAngular(t, s, 1)
}


function zoomFlipXLeftOver(t, s)
{
	return cc.transition.ZoomFlipX (t, s, 0)
}
function zoomFlipXRightOver(t, s)
{
	return cc.transition.ZoomFlipX (t, s, 1)
}

function zoomFlipYUpOver(t, s)
{
	return cc.transition.ZoomFlipY (t, s, 0)
}
function zoomFlipYDownOver(t, s)
{
	return cc.transition.ZoomFlipY (t, s, 1)
}

function zoomFlipAngularLeftOver(t, s)
{
	return cc.transition.ZoomFlipAngular(t, s, 0)
}
function zoomFlipAngularRightOver(t, s)
{
	return cc.transition.ZoomFlipAngular(t, s, 1)
}

function pageTransitionForward(t, s)
{
	return cc.transition.PageTurn(t, s, false)
}
function pageTransitionBackward(t, s)
{
	return cc.transition.PageTurn(t, s, true)
}


transitionInfo :=
{
	MAX_LAYER = 37
	s_nSceneIdx = 0
}

transitions := 
[
	"JumpZoomTransition",
    "FadeTransition",
    "FadeWhiteTransition",
    "FlipXLeftOver",
    "FlipXRightOver",
	"FlipYUpOver",
    "FlipYDownOver",
    "FlipAngularLeftOver",
    "FlipAngularRightOver",
    "ZoomFlipXLeftOver",
    "ZoomFlipXRightOver",
    "ZoomFlipYUpOver",
    "ZoomFlipYDownOver",
    "ZoomFlipAngularLeftOver",
    "ZoomFlipAngularRightOver",
	
	"ShrinkGrowTransition",
    "RotoZoomTransition",
    "MoveInLTransition",
    "MoveInRTransition",
    "MoveInTTransition",
    "MoveInBTransition",
    "SlideInLTransition",
    "SlideInRTransition",
    "SlideInTTransition",
    "SlideInBTransition",

    "CCTransitionCrossFade",
    "CCTransitionRadialCCW",
    "CCTransitionRadialCW",
	
	"PageTransitionForward",
    "PageTransitionBackward",
	
	"FadeTRTransition",
    "FadeBLTransition",
    "FadeUpTransition",
    "FadeDownTransition",
    "TurnOffTilesTransition",
    "SplitRowsTransition",
    "SplitColsTransition",
]

function createTransition(nIndex, t, s)
{
	switch(nIndex)
	{
		case 0: return cc.transition.JumpZoom(t, s);
		case 1: return cc.transition.Fade(t, s);
		case 2: return fadeWhiteTransition(t, s);
		case 3: return flipXLeftOver(t, s);
		case 4: return flipXRightOver(t, s);
		case 5: return flipYUpOver(t, s);
		case 6: return flipYDownOver(t, s);
		case 7: return flipAngularLeftOver(t, s);
		case 8: return flipAngularRightOver(t, s);
		case 9: return zoomFlipXLeftOver(t, s);
		case 10: return zoomFlipXRightOver(t, s);
		case 11: return zoomFlipYUpOver(t, s);
		case 12: return zoomFlipYDownOver(t, s);
		case 13: return zoomFlipAngularLeftOver(t, s);
		case 14: return zoomFlipAngularRightOver(t, s);
		
		case 15: return cc.transition.ShrinkGrow(t, s);
		case 16: return cc.transition.RotoZoom(t, s);
		case 17: return cc.transition.MoveInL(t, s);
		case 18: return cc.transition.MoveInR(t, s);
		case 19: return cc.transition.MoveInT(t, s);
		case 20: return cc.transition.MoveInB(t, s);
		case 21: return cc.transition.SlideInL(t, s);
		case 22: return cc.transition.SlideInR(t, s);
		case 23: return cc.transition.SlideInT(t, s);
		case 24: return cc.transition.SlideInB(t, s);
		case 25: return cc.transition.CrossFade(t,s);
		case 26: return cc.transition.RadialCCW(t,s);
		case 27: return cc.transition.RadialCW(t,s);
		case 28: return pageTransitionForward(t, s);
		case 29: return pageTransitionBackward(t, s);
		case 30: return cc.transition.FadeTR(t, s);
		case 31: return cc.transition.FadeBL(t, s);
		case 32: return cc.transition.FadeUp(t, s);
		case 33: return cc.transition.FadeDown(t, s);
		case 34: return cc.transition.TurnOffTiles(t, s);
		case 35: return cc.transition.SplitRows(t, s);
		case 36: return cc.transition.SplitCols(t, s);
	}
}


class TransitionsTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = TestLayer1()
		addChild(pLayer);
		cocos.director.replaceScene(this)
	}
}

class TestLayer1 : cc.ScriptLayer
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize;
		var w = size.width;
		var h = size.height;
		
		var bg1 = cc.Sprite(pack.locate("background1.png"))
		bg1.position = cc.Point(w/2, h/2);
		this.addChild(bg1, -1);
		
		
		var title = cc.LabelTTF(transitions[transitionInfo.s_nSceneIdx], "Arial", 32)
		title.color = nit.Color.RED
		title.position = cc.Point(w / 2, h - 100)
		this.addChild(title)
		
		var label = cc.LabelTTF("SCENE 1", "Arial", 38)
		label.color = nit.Color.BLUE
		label.position = cc.Point(w / 2, h/2)
		this.addChild(label)
		
		var item1 = cc.MenuItemImage(pack.locate("b1.png"),pack.locate("b2.png"), this,backCallBack);
		var item2 = cc.MenuItemImage(pack.locate("r1.png"), pack.locate("r2.png"), this,restartCallBack);		
		var item3 = cc.MenuItemImage(pack.locate("f1.png"), pack.locate("f2.png"), this,nextCallBack);
		
		var menu = cc.Menu(item1, item2, item3);
		menu.position= cc.Point(0,0);
		item1.position= cc.Point( w/2 - 100,30) ;
		item2.position= cc.Point( w/2, 30) ;
		item3.position= cc.Point( w/2 + 100,30) ;
		this.addChild( menu, 1 );	
		
		var m_emitter = cc.particle.Galaxy()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		m_emitter.position = cc.Point( 0 , h / 2 + 100)
		m_emitter.positionType = 1;
			
		var action = cc.action.MoveTo(2, cc.Point(w, h/2 + 100))
		var action_back = cc.action.MoveTo(2, cc.Point(0, h/2 + 100))
		var seq = cc.action.Sequence(action, action_back)
		
		m_emitter.runAction( cc.action.RepeatForever( seq ) )
		this.addChild(m_emitter, 10);
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		transitionInfo.s_nSceneIdx--;
		var total = transitionInfo.MAX_LAYER;
		if (transitionInfo.s_nSceneIdx < 0)
			transitionInfo.s_nSceneIdx += total
		
		var s = TransitionsTestScene()
		var pLayer = TestLayer2()
		s.addChild(pLayer)
		
		print("TransitionInfo.s_nSceneIdx :" + transitionInfo.s_nSceneIdx)
		var pScene = createTransition(transitionInfo.s_nSceneIdx, TRANSITION_DURATION, s)
		if (pScene)
		{
			cocos.director.replaceScene(pScene)
		}
	}
	
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		var s = TransitionsTestScene()
		var pLayer = TestLayer2()
		s.addChild(pLayer)
		
		var pScene = createTransition(transitionInfo.s_nSceneIdx, TRANSITION_DURATION, s)
		if (pScene)
		{
			cocos.director.replaceScene(pScene)
		}
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		transitionInfo.s_nSceneIdx++
		if (transitionInfo.s_nSceneIdx >= transitionInfo.MAX_LAYER)
			transitionInfo.s_nSceneIdx = 0
		
		var s = TransitionsTestScene()
		var pLayer = TestLayer2()
		s.addChild(pLayer)
		
		print("TransitionInfo.s_nSceneIdx :" + transitionInfo.s_nSceneIdx)
		var pScene = createTransition(transitionInfo.s_nSceneIdx, TRANSITION_DURATION, s)
		if (pScene)
		{
			cocos.director.replaceScene(pScene)
		}
	}
}


class TestLayer2 : cc.ScriptLayer
{
	constructor()
	{
		
		base.constructor()
		
		var size = cocos.director.winSize;
		var w = size.width;
		var h = size.height;
		
		var bg1 = cc.Sprite(pack.locate("background2.png"))
		bg1.position = cc.Point(w/2, h/2);
		this.addChild(bg1, -1);
		
		var title = cc.LabelTTF(transitions[transitionInfo.s_nSceneIdx], "Arial", 32)
		title.color = nit.Color.RED
		title.position = cc.Point(w / 2, h - 100)
		this.addChild(title)
		
		var label = cc.LabelTTF("SCENE 2", "Arial", 38)
		label.color = nit.Color.BLUE
		label.position = cc.Point(w / 2, h/2)
		this.addChild(label)
		
		var item1 = cc.MenuItemImage(pack.locate("b1.png"),pack.locate("b2.png"), this,backCallBack);
		var item2 = cc.MenuItemImage(pack.locate("r1.png"), pack.locate("r2.png"), this,restartCallBack);		
		var item3 = cc.MenuItemImage(pack.locate("f1.png"), pack.locate("f2.png"), this,nextCallBack);
		
		var menu = cc.Menu(item1, item2, item3);
		menu.position= cc.Point(0,0);
		item1.position= cc.Point( w/2 - 100,30) ;
		item2.position= cc.Point( w/2, 30) ;
		item3.position= cc.Point( w/2 + 100,30) ;
		this.addChild( menu, 1 );	
		
		var m_emitter = cc.particle.Sun()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		m_emitter.position = cc.Point( 0 , h / 2 - 100)
		m_emitter.positionType = 1;
		
		var action = cc.action.MoveTo(2, cc.Point(w, h/2 - 100))
		var action_back = cc.action.MoveTo(2, cc.Point(0, h/2 - 100))
		var seq = cc.action.Sequence(action, action_back)
		
		m_emitter.runAction( cc.action.RepeatForever( seq ) )
		this.addChild(m_emitter, 10);
		//cocos.Director.Timer.Channel().Bind(Events.OnTick, this, Tick );
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		transitionInfo.s_nSceneIdx--
		var total = transitionInfo.MAX_LAYER;
		if (transitionInfo.s_nSceneIdx < 0)
			transitionInfo.s_nSceneIdx += total
		
		var s = TransitionsTestScene()
		var pLayer = TestLayer1()
		s.addChild(pLayer)
		
		print("TransitionInfo.s_nSceneIdx :" + transitionInfo.s_nSceneIdx)
		var pScene = createTransition(transitionInfo.s_nSceneIdx, TRANSITION_DURATION, s)
		if (pScene)
		{
			cocos.director.replaceScene(pScene)
		}
	}
	
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		var s = TransitionsTestScene()
		var pLayer = TestLayer1()
		s.addChild(pLayer)
		
		var pScene = createTransition(transitionInfo.s_nSceneIdx, TRANSITION_DURATION, s)
		if (pScene)
		{
			cocos.director.replaceScene(pScene)
		}
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		transitionInfo.s_nSceneIdx++
		if (transitionInfo.s_nSceneIdx >= transitionInfo.MAX_LAYER)
			transitionInfo.s_nSceneIdx = 0
		
		var s = TransitionsTestScene()
		var pLayer = TestLayer1()
		s.addChild(pLayer)
		
		var pScene = createTransition(transitionInfo.s_nSceneIdx, TRANSITION_DURATION, s)
		if (pScene)
		{
			cocos.director.replaceScene(pScene)
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

return TransitionsTestScene()
