var pack = script.locator

////////////////////////////////////////////////////////////////////////////////
class FontTest
{
	sceneindex = -1
	scenemaxcnt = 9
	fontlist = 
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
		
		layer = FontDemo(fontlist[sceneindex])
		
		return layer
	}
	
	function nextTest()
	{
		sceneindex++
		sceneindex = sceneindex % scenemaxcnt
		
		return _createTest(sceneindex)
	}
	function backTest()
	{
		sceneindex--
		if (sceneindex < 0)
			sceneindex = scenemaxcnt -1
			
		return _createTest(sceneindex)
	}
	function restartTest()
	{
		return _createTest(sceneindex)
	}
}
fontTest := FontTest()
////////////////////////////////////////////////////////////////////////////////

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
		this.addChild(layer)
		cocos.director.replaceScene(this)
	}
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// FontDemo
//
class FontDemo : cc.ScriptLayer
{
	
	constructor(fontname)
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var item1 = cc.MenuItemImage(
			s_pPathB1,
			s_pPathB2,
			this, onBackMenu)
		var item2 = cc.MenuItemImage(
			s_pPathR1,
			s_pPathR2,
			this, onRestartMenu)
		var item3 = cc.MenuItemImage(
			s_pPathF1,
			s_pPathF2,
			this, onNextMenu)
			
		var menu = cc.Menu(item1, item2, item3)
		menu.position = cc.Point(0,0)
		item1.position = cc.Point(s.width / 2 - 100, 30)
		item2.position = cc.Point(s.width / 2, 30)
		item3.position = cc.Point(s.width / 2 + 100, 30)
		this.addChild(menu, 1)
		
		showFont(fontname)
		
		
		
		
		
		
	}
	function showFont(fontname)
	{
		this.removeChildByTag(0, true)
		this.removeChildByTag(1, true)
		this.removeChildByTag(2, true)
		this.removeChildByTag(3, true)
		
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
		
		this.addChild(left, 0, 0)
		this.addChild(right, 0, 1)
		this.addChild(center, 0, 2)
		this.addChild(top, 0, 3)
	
	}
	
	function title()
	{
		return "Font test"
	}
	
	function onBackMenu(evt: cc.MenyItemEvent)
	{
		var s =  FontTestScene()
		s.addChild(fontTest.backTest())
		cocos.director.replaceScene(s)
	}
	function onRestartMenu(evt: cc.MenyItemEvent)
	{
		var s =  FontTestScene()
		s.addChild(fontTest.restartTest())
		cocos.director.replaceScene(s)
	}
	function onNextMenu(evt: cc.MenyItemEvent)
	{
		var s =  FontTestScene()
		s.addChild(fontTest.nextTest())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////


return FontTestScene()