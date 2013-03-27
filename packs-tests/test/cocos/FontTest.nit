var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

class FontTest
{
	_sceneIndex = -1
	_sceneMax = 9
	
	_fontList = 
	[
		"Arial",
		"A Damn Mess",
		"Abberancy",
		"Abduction",
		"Paint Boy",
		"Schwarzwald",
		"Scissor Cuts",
		"산돌고딕Neo1 Regular",
		"산돌고딕Neo1 Bold",
	]
	
	constructor()
	{
	}
	
	function _createTest(index)
	{
		var layer = null
		
		layer = FontDemo(_fontList[_sceneIndex])
		
		return layer
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

var fontTest = FontTest()

////////////////////////////////////////////////////////////////////////////////

class FontDemo : cc.ScriptLayer
{
	constructor(fontname)
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var item1 = cc.MenuItemImage(
			IMG.B1,
			IMG.B2,
			this, onBackMenu)
		var item2 = cc.MenuItemImage(
			IMG.R1,
			IMG.R2,
			this, onRestartMenu)
		var item3 = cc.MenuItemImage(
			IMG.F1,
			IMG.F2,
			this, onNextMenu)
			
		var menu = cc.Menu(item1, item2, item3)
		menu.position = cc.Point(0,0)
		item1.position = cc.Point(s.width / 2 - 100, 30)
		item2.position = cc.Point(s.width / 2, 30)
		item3.position = cc.Point(s.width / 2 + 100, 30)
		addChild(menu, 1)
		
		showFont(fontname)
	}

	function showFont(fontname)
	{
		removeChildByTag(0, true)
		removeChildByTag(1, true)
		removeChildByTag(2, true)
		removeChildByTag(3, true)
		
		var s = cocos.director.winSize

		var font = cc.Font(fontname) with { defaultSize = 16 }
		var titleFont = cc.Font(font) with { defaultSize = 24 }
		
		var top = cc.LabelTTF("폰트: " + fontname, titleFont)
		var left = cc.LabelTTF("alignment left", cc.Size(s.width, 50), cc.LabelTTF.ALIGN_LEFT, font)
		var center = cc.LabelTTF("alignment center", cc.Size(s.width, 50), cc.LabelTTF.ALIGN_CENTER, font)
		var right = cc.LabelTTF("alignment right", cc.Size(s.width, 50), cc.LabelTTF.ALIGN_RIGHT, font)
		
		top.position = cc.Point(s.width / 2, 250)
		left.position = cc.Point(s.width / 2, 200)
		center.position = cc.Point(s.width / 2, 150)
		right.position = cc.Point(s.width / 2, 100)
		
		addChild(left, 0, 0)
		addChild(right, 0, 1)
		addChild(center, 0, 2)
		addChild(top, 0, 3)
	}
	
	function title()
	{
		return "Font test"
	}
	
	function onBackMenu(evt: cc.MenuItemEvent)
	{
		var s = FontTestScene()
		s.addChild(fontTest.backTest())
		cocos.director.replaceScene(s)
	}

	function onRestartMenu(evt: cc.MenuItemEvent)
	{
		var s = FontTestScene()
		s.addChild(fontTest.restartTest())
		cocos.director.replaceScene(s)
	}

	function onNextMenu(evt: cc.MenuItemEvent)
	{
		var s = FontTestScene()
		s.addChild(fontTest.nextTest())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class FontTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var layer = fontTest.nextTest()
		addChild(layer)
		cocos.director.replaceScene(this)
	}
}

return FontTestScene()