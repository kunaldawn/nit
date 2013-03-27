class TestScene : cc.ScriptScene
{
	portrait = false
	
	constructor(portrait = false)
	{
		base.constructor()
		portrait = portrait
		
		if (portrait)
			cocos.director with deviceOrientation = ORIENT_LANDSCAPE_RIGHT
	}
	
	function onEnter()
	{
		var label = cc.LabelTTF("MainMenu", "Arial", 20)
		var item = cc.MenuItemLabel(label, this, onReturnToMain)
		
		var winSize = cocos.director.winSize
		var menu = cc.Menu(item)
		menu.position = cc.Point(0, 0)
		item.position = cc.Point(winSize.width - 50, 25)
		
		addChild(menu)
	}
	
	function onReturnToMain()
	{
		startTestMain()
	}
	
	function runThisTest()
	{
		throw "not implemented"
	}
}