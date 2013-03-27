var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var TAG =
{
	TEXT_LAYER	= 1

	SPRITE1		= 1
	SPRITE2		= 2

	BACKGROUND	= 1
	LABEL		= 2
}

var IDC_NEXT 	= 100
var IDC_BACK 	= 101
var IDC_RESTART = 102

var sceneIdx 	= -1

var MAX_LAYER 	= 6

////////////////////////////////////////////////////////////////////////////////

var function createEffectAdvanceLayer(nIndex)
{
	switch (nIndex)
	{
		case 0: return Effect3()
		case 1: return Effect2()
		case 2: return Effect1()
		case 3: return Effect4()
		case 4: return Effect5()
		case 5: return Issue631()
	}  
}

var function nextEffectAdvanceAction()
{
	sceneIdx++
	sceneIdx = sceneIdx % MAX_LAYER

	var pLayer = createEffectAdvanceLayer(sceneIdx)
	
	return pLayer
}

var function backEffectAdvanceAction()
{
	sceneIdx--
	var total = MAX_LAYER
	if (sceneIdx < 0)
		sceneIdx += total	
	
	var pLayer = createEffectAdvanceLayer(sceneIdx)

	return pLayer
}

var function restartEffectAdvanceAction()
{
	var pLayer = createEffectAdvanceLayer(sceneIdx)

	return pLayer
} 

////////////////////////////////////////////////////////////////////////////////

class EffectAdvancedTextLayer : cc.ScriptLayer
{
	_atlas = null
	_strTitle = null
	
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		var x = size.width
		var y = size.height
		
		var bg = cc.Sprite(pack.locate("background3.png"))
		addChild(bg, 0, TAG.BACKGROUND)
		bg.position = cc.Point(x/2, y/2)
		
		var grossini = cc.Sprite(pack.locate("grossinis_sister2.png"))
		bg.addChild(grossini, 1, TAG.SPRITE1)
		grossini.position = cc.Point(x/3, 200)
		
		var sc = cc.action.ScaleBy(2, 5)
		var sc_back = sc.reverse()
		grossini.runAction(cc.action.RepeatForever(cc.action.Sequence(sc, sc_back)))
		
		var tamara = cc.Sprite(pack.locate("grossinis_sister1.png"))
		bg.addChild(tamara, 1, TAG.SPRITE2)
		tamara.position = cc.Point(2*x/3, 200)
		
		var sc2 = cc.action.ScaleBy(2, 5)
		var sc2_back = sc2.reverse()
		tamara.runAction(cc.action.RepeatForever(cc.action.Sequence(sc2, sc2_back)))
	
		var label = cc.LabelTTF(title(), "Arial", 28)
		label.position = cc.Point(x/2, y-80)
		addChild(label)
		label.tag = TAG.LABEL
		
		var strSubtitle = subtitle()
		if (strSubtitle != "")
		{
			var l = cc.LabelTTF(strSubtitle, "Arial", 16)
			addChild(l, 101)
			l.position = cc.Point(size.width/2, size.height-80)
		}
		
		var item1 = cc.MenuItemImage(IMG.B1, IMG.B2, this, onBack)
		var item2 = cc.MenuItemImage(IMG.R1, IMG.R2, this, onRestart)	
		var item3 = cc.MenuItemImage(IMG.F1, IMG.F2, this, onNext)
		
		var menu = cc.Menu(item1, item2, item3)
		menu.position = cc.Point(0, 0)
		item1.position = cc.Point(size.width/2 - 100, 30)
		item2.position = cc.Point(size.width/2, 30)
		item3.position = cc.Point(size.width/2 + 100, 30)
		this.addChild(menu, 1)	
	}
	
	function title()
	{
		return "No title"
	}
	
	function subtitle()
	{
		return ""
	}
	
	function onRestart(evt: cc.MenuItemEvent)
	{
		var s = EffectAdvanceScene()
		s.addChild(restartEffectAdvanceAction())
		cocos.director.replaceScene(s)
	}
	
	function onNext(evt: cc.MenuItemEvent)
	{
		var s = EffectAdvanceScene()
		s.addChild(nextEffectAdvanceAction())
		cocos.director.replaceScene(s)
	}
	
	function onBack(evt: cc.MenuItemEvent)
	{
		var s = EffectAdvanceScene()
		s.addChild(backEffectAdvanceAction())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class Effect1 : EffectAdvancedTextLayer
{
	constructor()
	{
		base.constructor()
		
		var target = getChildByTag(TAG.BACKGROUND)
		// To reuse a grid the grid size and the grid type must be the same.
		// in this case:
		//     Lens3D is Grid3D and it's size is (15, 10)
		//     Waves3D is Grid3D and it's size is (15, 10)
		var size = cocos.director.winSize
		var lens = cc.action.Lens3D(cc.Point(size.width/2, size.height/2), 240, cc.GridSize(15, 10), 0)
		var waves = cc.action.Waves3D(18, 15, cc.GridSize(15, 10), 10)
		
		var reuse = cc.action.ReuseGrid(1)
		var delay = cc.action.DelayTime(8)
		
		var orbit = cc.action.OrbitCamera(5, 1, 2, 0, 180, 0, -90)
		var orbit_back = orbit.reverse()
		
		target.runAction(cc.action.RepeatForever(cc.action.Sequence(orbit, orbit_back)))
		target.runAction(cc.action.Sequence(lens, delay, reuse, waves))
		
	}
	
	function title()
	{	
		return "Lens + Waves3d and OrbitCamera"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Effect2 : EffectAdvancedTextLayer
{
	constructor()
	{
		base.constructor()
		
		var target = getChildByTag(TAG.BACKGROUND)
		// To reuse a grid the grid size and the grid type must be the same.
		// in this case:
		//     ShakyTiles is TiledGrid3D and it's size is (15, 10)
		//     Shuffletiles is TiledGrid3D and it's size is (15, 10)
		//	   TurnOfftiles is TiledGrid3D and it's size is (15, 10)
		var shaky = cc.action.ShakyTiles3D(4, false, cc.GridSize(15, 10), 5)
		var shuffle = cc.action.ShuffleTiles(0, cc.GridSize(15, 10), 3)
		var turnoff = cc.action.TurnOffTiles(0, cc.GridSize(15, 10), 3)
		var turnon = turnoff.reverse()
		// reuse 2 times:
		//   1 for shuffle
		//   2 for turn off
		//   turnon tiles will use a new grid
		var resue = cc.action.ReuseGrid(2)
		var delay1 = cc.action.DelayTime(1)
		var delay2 = cc.action.DelayTime(1)
		//	id orbit = [OrbitCamera::actionWithDuration:5 radius:1 deltaRadius:2 	angleZ:0 deltaAngleZ:180 angleX:0 deltaAngleX:-90];
		//	id orbit_back = [orbit reverse];
		//
		//	[target runAction: [RepeatForever::actionWithAction: [Sequence actions: orbit, orbit_back, nil]]];
		target.runAction(cc.action.Sequence(shaky, delay1, resue, shuffle, delay2, turnoff, turnon))
		
	}
	
	function title()
	{	
		return "ShakyTiles + ShuffleTiles + TurnOffTiles"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Effect3 : EffectAdvancedTextLayer
{
	constructor()
	{
		base.constructor()
		
		var bg = getChildByTag(TAG.BACKGROUND)
		var target1 = bg.getChildByTag(TAG.SPRITE1)
		var target2 = bg.getChildByTag(TAG.SPRITE2)
		
		var waves = cc.action.Waves(5, 20, true, false, cc.GridSize(15, 10), 10)
		var shaky = cc.action.Shaky3D(4, false, cc.GridSize(15, 10), 5)
		
		target1.runAction(cc.action.RepeatForever(waves))
		target2.runAction(cc.action.RepeatForever(shaky))
		
		// moving background. Testing issue #244
		var move = cc.action.MoveBy(3, cc.Point(200, 0))
		bg.runAction(cc.action.RepeatForever(cc.action.Sequence(move, move.reverse())))
	}
	
	function title()
	{	
		return "Effects on 2 sprites"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Effect4 : EffectAdvancedTextLayer
{
	constructor()
	{
		base.constructor()
		
		var lens = cc.action.Lens3D(cc.Point(100, 180), 150, cc.GridSize(32, 24), 10)
		//dump(lens)
		//id move = [MoveBy::actionWithDuration:5 position:ccp(400, 0)];
		/**
		//@todo we only support CCNode run actions now.
		*/
		// 	CCActionInterval* move = CCJumpBy::actionWithDuration(5, ccp(380, 0), 100, 4);
		// 	CCActionInterval* move_back = move->reverse();
		// 	CCActionInterval* seq = (CCActionInterval *)(CCSequence::actions(move, move_back, NULL));
		//  CCActionManager::sharedManager()->addAction(seq, lens, false);
		runAction(lens)
	}
	
	function title()
	{	
		return "Jumpy Lens3D"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Effect5 : EffectAdvancedTextLayer
{
	constructor()
	{
		base.constructor()
		
		var effect = cc.action.Liquid(1, 20, cc.GridSize(32, 24), 2)
		var stopEffect = cc.action.Sequence(
			effect, 
			cc.action.DelayTime(2), 
			cc.action.StopGrid()
		)
									
		var bg = getChildByTag(TAG.BACKGROUND)
		bg.runAction(stopEffect)
	}
	
	function title()
	{	
		return "Test Stop-Copy-Restart"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Issue631 : EffectAdvancedTextLayer
{
	constructor()
	{
		base.constructor()

		var effect = cc.action.Sequence(
			cc.action.DelayTime(2), 
			cc.action.Shaky3D(16, false, cc.GridSize(5, 5), 5)
		)
		
		//cleanup
		var bg = getChildByTag(TAG.BACKGROUND)
		removeChild(bg, true)
		
		//background
		var layer = cc.LayerColor(nit.Color(255/255, 0, 0, 255/255))
		addChild(layer, -10)
		var sprite = cc.Sprite(pack.locate("grossini.png"))
		sprite.position = cc.Point(50, 80)
		layer.addChild(sprite, 10)
		
		//foreground
		var layer2 = cc.LayerColor(nit.Color(0, 255/255, 0, 255/255))
		var fog = cc.Sprite(pack.locate("Fog.png"))
		fog.blendFuncSrc = cc.Sprite.SRC_BLEND_SRC_ALPHA
		fog.blendFuncDst = cc.Sprite.DST_BLEND_ONE_MINUS_SRC_ALPHA
		layer2.addChild(fog, 1)
		addChild(layer2, 1)
		
		layer2.runAction(cc.action.RepeatForever(effect))
	}
	
	function title()
	{	
		return "Testing Opacity"
	}
	function subtitle()
	{
		return "Effect image should be 100% opaque. Testing issue #631"
	}
}

////////////////////////////////////////////////////////////////////////////////

class EffectAdvanceScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = nextEffectAdvanceAction()
		addChild(pLayer)
		cocos.director.replaceScene(this)
	}
}

return EffectAdvanceScene()