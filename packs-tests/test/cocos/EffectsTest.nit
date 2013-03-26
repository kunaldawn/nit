var pack = script.locator

kTagTextLayer := 1

kTagBackground := 1
kTagLabel 	:= 2

actionIdx 	:= 0

MAX_LAYER 	:= 22

SID_RESTART := 1

effectsList :=
[
	"Shaky3D",
	"Waves3D",
	"FlipX3D",
	"FlipY3D",
	"Lens3D",
	"Ripple3D",
	"Liquid",
	"Waves",
	"Twirl",
	"ShakyTiles3D",
	"ShatteredTiles3D",
	"ShuffleTiles",
	"FadeOutTRTiles",
	"FadeOutBLTiles",
	"FadeOutUpTiles",
	"FadeOutDownTiles",
	"TurnOffTiles",
	"WavesTiles3D",
	"JumpTiles3D",
	"SplitRows",
	"SplitCols",
	"PageTurn3D",
] 

function createEffect( nIndex, t)
{
	switch(nIndex)
	{
		case 0: return shaky3DDemo(t);
		case 1: return waves3DDemo(t);
		case 2: return flipX3DDemo(t);
		case 3: return flipY3DDemo(t);
		case 4: return lens3DDemo(t);
		case 5: return ripple3DDemo(t);
		case 6: return liquidDemo(t);
		case 7: return wavesDemo(t);
		case 8: return twirlDemo(t);
		case 9: return shakyTiles3DDemo(t);
		case 10: return shatteredTiles3DDemo(t);
		case 11: return shuffleTilesDemo(t);
		case 12: return fadeOutTRTilesDemo(t);
		case 13: return fadeOutBLTilesDemo(t);
		case 14: return fadeOutUpTilesDemo(t);
		case 15: return fadeOutDownTilesDemo(t);
		case 16: return turnOffTilesDemo(t);
		case 17: return wavesTiles3DDemo(t);
		case 18: return jumpTiles3DDemo(t);
		case 19: return splitRowsDemo(t);
		case 20: return splitColsDemo(t);
		case 21: return pageTurn3DDemo(t);
	}

	return null;
}

function getAction()
{	
	var pEffect = createEffect(actionIdx, 3)
	return pEffect
}

class EffectTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = TextLayer()
		addChild(pLayer)
		cocos.director.replaceScene(this)
	}
}

class TextLayer : cc.ScriptLayer
{
	m_atlas = null
	m_strTitle = null
	m_handler = null
	
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		var x = size.width
		var y = size.height
		
		var bglayer = cc.LayerColor(nit.Color(32/255, 32/255, 32/255, 255/255))
		addChild(bglayer, -1)
		
		var node = cc.Node()
		var effect = getAction()
		node.runAction(effect)
		addChild(node, 0, kTagBackground)
		
		var bg = cc.Sprite(pack.locate("background3.png"))
		node.addChild(bg, 0)
		bg.anchorPoint = cc.Point(0, 0)
		
		var grossini = cc.Sprite(pack.locate("grossinis_sister2.png"))
		bg.addChild(grossini, 1)
		grossini.position = cc.Point(x/3, y/2)
		
		var sc = cc.action.ScaleBy(2, 5)
		var sc_back = sc.reverse()
		grossini.runAction(cc.action.RepeatForever(cc.action.Sequence(sc, sc_back)))
		
		var tamara = cc.Sprite(pack.locate("grossinis_sister1.png"))
		bg.addChild(tamara, 1)
		tamara.position = cc.Point(2*x/3, y/2)
		
		var sc2 = cc.action.ScaleBy(2, 5)
		var sc2_back = sc2.reverse()
		tamara.runAction(cc.action.RepeatForever(cc.action.Sequence(sc2, sc2_back)))
	
		var label = cc.LabelTTF( effectsList[actionIdx], "Marker Felt", 28)
		label.position = cc.Point(x/2, y-80)
		addChild(label)
		label.tag = kTagLabel
		
		var item1 = cc.MenuItemImage(s_pPathB1,s_pPathB2, this,backCallBack);
		var item2 = cc.MenuItemImage(s_pPathR1, s_pPathR2, this,restartCallBack)	
		var item3 = cc.MenuItemImage(s_pPathF1, s_pPathF2, this,nextCallBack);
		
		var menu = cc.Menu(item1, item2, item3);
		menu.position= cc.Point(0,0);
		item1.position= cc.Point( size.width/2 - 100,30) ;
		item2.position= cc.Point( size.width/2, 30) ;
		item3.position= cc.Point( size.width/2 + 100,30) ;
		this.addChild( menu, 1 );	
		
		m_handler = session.scheduler.repeat(this, checkAnim, 0.1)
	}
	
	function title()
	{
		return "No title";
	}
	
	function subtitle()
	{
		return "";
	}
	
	function checkAnim(dt)
	{
		var s2 = getChildByTag(kTagBackground)
		if (s2.numRunningActions == 0 && s2.grid != null)
		{
			s2.grid = null
			session.scheduler.unbind(m_handler)
		}
	}
	
	function node()
	{
		var pLayer = TextLayer()
		return pLayer
	}
	
	function newOrientation()
	{
		var s_currentOrientation = cocos.director.deviceOrientation
		switch(s_currentOrientation)
		{
			case cocos.director.ORIENT_LANDSCAPE_LEFT:
				s_currentOrientation = cocos.director.ORIENT_PORTRAIT_UP;
				break;
			case cocos.director.ORIENT_PORTRAIT_UP:
				s_currentOrientation = cocos.director.ORIENT_LANDSCAPE_RIGHT;
				break;						
			case cocos.director.ORIENT_LANDSCAPE_RIGHT:
				s_currentOrientation = cocos.director.ORIENT_PORTRAIT_DOWN;
				break;
			case cocos.director.ORIENT_PORTRAIT_DOWN:
				s_currentOrientation = cocos.director.ORIENT_LANDSCAPE_LEFT;
				break;
		}
		cocos.director.deviceOrientation = s_currentOrientation
	}
	
	function newScene()
	{
		var s = EffectTestScene()
		var child = node()
		s.addChild(child)
		cocos.director.replaceScene(s)
	}
	
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		newScene()
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		 // update the action index
		actionIdx++
		actionIdx = actionIdx % MAX_LAYER

		/*newOrientation();*/
		newScene();
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		 // update the action index
		actionIdx--
		var total = MAX_LAYER
		if( actionIdx < 0 )
			actionIdx += total

		/*newOrientation();*/
		newScene();
	}
}

//------------------------------------------------------------------
//
//
//------------------------------------------------------------------
function shaky3DDemo(t)
{
	return cc.action.Shaky3D(5, true, cc.GridSize(15,10), t)
}

function waves3DDemo(t)
{
	return cc.action.Waves3D(5, 40, cc.GridSize(15,10), t)
}

function flipX3DDemo(t)
{
	var flipx = cc.action.FlipX3D(t)
	var flipx_back = flipx.reverse()
	var delay = cc.action.DelayTime(2)

	return cc.action.Sequence(flipx, delay, flipx_back)
}

function flipY3DDemo(t)
{
	var flipy = cc.action.FlipY3D(t)
	var flipy_back = flipy.reverse()
	var delay = cc.action.DelayTime(2)

	return cc.action.Sequence(flipy, delay, flipy_back)
}

function lens3DDemo(t)
{
	var size = cocos.director.winSize
	return cc.action.Lens3D(cc.Point(size.width/2, size.height/2), 240, cc.GridSize(15,10), t)
}

function ripple3DDemo(t)
{
	var size = cocos.director.winSize
	return cc.action.Ripple3D(cc.Point(size.width/2, size.height/2), 240, 4, 160, cc.GridSize(32,24), t)
}

function liquidDemo(t)
{
	return cc.action.Liquid(4, 20, cc.GridSize(16,12), t)
}

function wavesDemo(t)
{
	return cc.action.Waves(4, 20, true, true, cc.GridSize(16,12), t)
}

function twirlDemo(t)
{
	var size = cocos.director.winSize
	return cc.action.Twirl(cc.Point(size.width/2, size.height/2),1,2.5,cc.GridSize(12,8), t)
}

function shakyTiles3DDemo(t)
{
	return cc.action.ShakyTiles3D(5, true, cc.GridSize(16,12), t)
}

function shatteredTiles3DDemo(t)
{
	return cc.action.ShatteredTiles3D(5, true, cc.GridSize(16,12), t)
}

function shuffleTilesDemo(t)
{
	var shuffle = cc.action.ShuffleTiles(25, cc.GridSize(16, 12), t)
	var shuffle_back = shuffle.reverse()
	var delay = cc.action.DelayTime(2)

	return cc.action.Sequence(shuffle, delay, shuffle_back)
}

function fadeOutTRTilesDemo(t)
{
	var fadeout = cc.action.FadeOutTRTiles(cc.GridSize(16, 12), t)
	var back = fadeout.reverse()
	var delay = cc.action.DelayTime(0.5)

	return cc.action.Sequence(fadeout, delay, back)
}

function fadeOutBLTilesDemo(t)
{
	var fadeout = cc.action.FadeOutBLTiles(cc.GridSize(16, 12), t)
	var back = fadeout.reverse()
	var delay = cc.action.DelayTime(0.5)

	return cc.action.Sequence(fadeout, delay, back)
}

function fadeOutUpTilesDemo(t)
{
	var fadeout = cc.action.FadeOutUpTiles(cc.GridSize(16, 12), t)
	var back = fadeout.reverse()
	var delay = cc.action.DelayTime(0.5)

	return cc.action.Sequence(fadeout, delay, back)
}

function fadeOutDownTilesDemo(t)
{
	var fadeout = cc.action.FadeOutDownTiles(cc.GridSize(16, 12), t)
	var back = fadeout.reverse()
	var delay = cc.action.DelayTime(0.5)

	return cc.action.Sequence(fadeout, delay, back)
}

function turnOffTilesDemo(t)
{
	var fadeout = cc.action.TurnOffTiles(cc.GridSize(48, 32), t)
	var back = fadeout.reverse()
	var delay = cc.action.DelayTime(0.5)

	return cc.action.Sequence(fadeout, delay, back)
}

function wavesTiles3DDemo(t)
{
	return cc.action.WavesTiles3D(4, 120, cc.GridSize(15,10), t)
}

function jumpTiles3DDemo(t)
{
	return cc.action.JumpTiles3D(2, 30, cc.GridSize(15,10), t)
}

function splitRowsDemo(t)
{
	return cc.action.SplitRows(9, t)
}

function splitColsDemo(t)
{
	return cc.action.SplitCols(9, t)
}

function pageTurn3DDemo(t)
{
	//cocos.director.SetDepthTest(true)
	return cc.action.PageTurn3D(cc.GridSize(15, 10), t)
}


