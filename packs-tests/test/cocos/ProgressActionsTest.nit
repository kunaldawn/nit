var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

class ProgressActionTest
{
	var _sceneIndex = -1
	var _sceneMax = 3
	
	constructor()
	{
	}
	
	function _createTest(index)
	{
		switch (index)
		{
		case 0: return SpriteProgressToRadial()
		case 1: return SpriteProgressToHorizontal()
		case 2: return SpriteProgressToVertical()
		}
	}
	
	function nextTest()
	{
		_sceneIndex++
		_sceneIndex = _sceneIndex % _sceneMax
		
		return _createTest(_sceneIndex)
	}
	
	function backTest()
	{
		_sceneIndex--
		if (_sceneIndex < 0)
			_sceneIndex = _sceneMax -1
			
		return _createTest(_sceneIndex)
	}
	
	function restartTest()
	{
		return _createTest(_sceneIndex)
	}
}

var progressTest = ProgressActionTest()

////////////////////////////////////////////////////////////////////////////////

class SpriteDemo : cc.ScriptLayer
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var label = cc.LabelTTF(title(), "Arial", 18)
		addChild(label, 1)
		label.position = cc.Point(s.width / 2, s.height - 50)
		
		var strSubtitle = subtitle()
		if (strSubtitle != "")
		{
			var l = cc.LabelTTF(strSubtitle, "Arial", 22)
			addChild(l, 1)
			l.position = cc.Point(s.width / 2, s.height - 80)
		}
		
		var item1 = cc.MenuItemImage(
			pack.locate("b1.png"),
			pack.locate("b2.png"),
			this, onBackMenu)
		var item2 = cc.MenuItemImage(
			pack.locate("r1.png"),
			pack.locate("r2.png"),
			this, onRestartMenu)
		var item3 = cc.MenuItemImage(
			pack.locate("f1.png"),
			pack.locate("f2.png"),
			this, onNextMenu)
			
		var menu = cc.Menu(item1, item2, item3)
		menu.position = cc.Point(0,0)
		item1.position = cc.Point(s.width / 2 - 100, 30)
		item2.position = cc.Point(s.width / 2, 30)
		item3.position = cc.Point(s.width / 2 + 100, 30)
		addChild(menu, 1)
	}
	
	function title()
	{
		return "ProgressActionsTest"
	}
	
	function subtitle()
	{
		return ""
	}
	
	function onBackMenu(evt: cc.MenyItemEvent)
	{
		var s =  ProgressActionsTestScene()
		s.addChild(progressTest.backTest())
		cocos.director.replaceScene(s)
	}
	
	function onRestartMenu(evt: cc.MenyItemEvent)
	{
		var s =  ProgressActionsTestScene()
		s.addChild(progressTest.restartTest())
		cocos.director.replaceScene(s)
	}
	
	function onNextMenu(evt: cc.MenyItemEvent)
	{
		var s =  ProgressActionsTestScene()
		s.addChild(progressTest.nextTest())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteProgressToRadial : SpriteDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var to1 = cc.action.ProgressTo(2, 100)
		var to2 = cc.action.ProgressTo(2, 100)
		
		var left = cc.ProgressTimer(s_pPathSister1)
		left.type = cc.ProgressTimer.TYPE_RADIAL_CW
		addChild(left)
		left.position = cc.Point(100, s.height / 2)
		left.runAction(cc.action.RepeatForever(to1))
		
		var right = cc.ProgressTimer(s_pPathBlock)
		right.type = cc.ProgressTimer.TYPE_RADIAL_CCW
		addChild(right)
		right.position = cc.Point(s.width - 100, s.height / 2)
		right.runAction(cc.action.RepeatForever(to2))
	}

	function subtitle()
	{
		return "ProgressTo Radial"
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteProgressToHorizontal : SpriteDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var to1 = cc.action.ProgressTo(2, 100)
		var to2 = cc.action.ProgressTo(2, 100)
		
		var left = cc.ProgressTimer(s_pPathSister1)
		left.type = cc.ProgressTimer.TYPE_HORZ_BAR_LR
		addChild(left)
		left.position = cc.Point(100, s.height / 2)
		left.runAction(cc.action.RepeatForever(to1))
		
		var right = cc.ProgressTimer(s_pPathSister2)
		right.type = cc.ProgressTimer.TYPE_HORZ_BAR_RL
		addChild(right)
		right.position = cc.Point(s.width - 100, s.height / 2)
		right.runAction(cc.action.RepeatForever(to2))
		
	}

	function subtitle()
	{
		return "ProgressTo Horizontal"
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteProgressToVertical : SpriteDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var to1 = cc.action.ProgressTo(2, 100)
		var to2 = cc.action.ProgressTo(2, 100)
		
		var left = cc.ProgressTimer(s_pPathSister1)
		left.type = cc.ProgressTimer.TYPE_VERT_BAR_BT
		addChild(left)
		left.position = cc.Point(100, s.height / 2)
		left.runAction(cc.action.RepeatForever(to1))
		
		var right = cc.ProgressTimer(s_pPathSister2)
		right.type = cc.ProgressTimer.TYPE_VERT_BAR_TB
		addChild(right)
		right.position = cc.Point(s.width - 100, s.height / 2)
		right.runAction(cc.action.RepeatForever(to2))
	}

	function subtitle()
	{
		return "ProgressTo Vertical"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ProgressActionsTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var layer = progressTest.nextTest()
		addChild(layer)
		cocos.director.replaceScene(this)
	}
}

return ProgressActionsTestScene()