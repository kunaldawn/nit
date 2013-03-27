var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var sceneIdx = -1
var MAX_LAYER = 1
var currOrientation = 0

////////////////////////////////////////////////////////////////////////////////

var function createTestCaseLayer(nIndex)
{
	switch (nIndex)
	{
	case 0: return Director1()
	}
}

var function nextDirectorTestCase()
{
	sceneIdx++
	sceneIdx = sceneIdx % MAX_LAYER

	var pLayer = createTestCaseLayer(sceneIdx)
	
	return pLayer
}

var function backDirectorTestCase()
{
	sceneIdx--
	var total = MAX_LAYER
	if (sceneIdx < 0)
		sceneIdx += total
	
	var pLayer = createTestCaseLayer(sceneIdx)

	return pLayer
}

var function restartDirectorTestCase()
{
	var pLayer = createTestCaseLayer(sceneIdx)

	return pLayer
} 

////////////////////////////////////////////////////////////////////////////////

class DirectorTest : cc.ScriptLayer
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		var x = size.width
		var y = size.height
	
		var label = cc.LabelTTF(title(), "Arial", 26)
		label.position = cc.Point(x/2, y-50)
		addChild(label, 1)
		
		var strSubtitle = subtitle()
		if (strSubtitle != "")
		{
			var l = cc.LabelTTF(strSubtitle, "Arial", 16)
			addChild(l, 1)
			l.position = cc.Point(size.width/2, size.height-80)
		}
		
		var item1 = cc.MenuItemImage(s_pPathB1, s_pPathB2, this, backCallBack)
		var item2 = cc.MenuItemImage(s_pPathR1, s_pPathR2, this, restartCallBack)	
		var item3 = cc.MenuItemImage(s_pPathF1, s_pPathF2, this, nextCallBack)
		
		var menu = cc.Menu(item1, item2, item3)
		menu.position= cc.Point(0, 0)
		item1.position= cc.Point(size.width/2 - 100, 30)
		item2.position= cc.Point(size.width/2, 30)
		item3.position= cc.Point(size.width/2 + 100, 30)
		addChild(menu, 1)
		
	}
	
	function title()
	{
		return "No title"
	}
	
	function subtitle()
	{
		return ""
	}
	
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		var s = DirectorTestScene()
		s.addChild(restartDirectorTestCase())
		cocos.director.replaceScene(s)
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = DirectorTestScene()
		s.addChild(nextDirectorTestCase())
		cocos.director.replaceScene(s)
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		var s = DirectorTestScene()
		s.addChild(backDirectorTestCase())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class Director1 : DirectorTest
{
	_root = null
	_target = null
	_streak = null
	
	constructor()
	{
		base.constructor()
			
		var s = cocos.director.winSize
		var item = cc.MenuItemFont("Rotate Device", this, rotateDevice)
		var menu = cc.Menu(item)
		menu.position = cc.Point(s.width/2, s.height/2)
		addChild(menu)
		
		touchEnabled = true
		channel().bind(Events.OnCCTouchEnded, this, ccTouchesEnded)
	}
	
	function newOrientation()
	{
		switch (currOrientation)
		{
			case cocos.director.ORIENT_LANDSCAPE_LEFT:
				currOrientation = cocos.director.ORIENT_PORTRAIT_UP
				break
			case cocos.director.ORIENT_PORTRAIT_UP:
				currOrientation = cocos.director.ORIENT_LANDSCAPE_RIGHT
				break
			case cocos.director.ORIENT_LANDSCAPE_RIGHT:
				currOrientation = cocos.director.ORIENT_PORTRAIT_DOWN
				break
			case cocos.director.ORIENT_PORTRAIT_DOWN:
				currOrientation = cocos.director.ORIENT_LANDSCAPE_LEFT
				break
		}
		cocos.director.deviceOrientation = currOrientation
	}
	
	function rotateDevice(pSender)
	{
		newOrientation()
		restartCallBack(null)
	}
	
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		foreach (touch in evt.touches)
		{
			var a = touch with locationInView(view)
			var b = cocos.director with toUi(toGl(a))
			print("("+a.x+", "+a.y+") == ("+b.x+", "+b.y+")")
		}
	}
	
	function title()
	{	
		return "Testing conversion"
	}
	
	function subtitle()
	{
		return "Tap screen and see the debug console"
	}
}

////////////////////////////////////////////////////////////////////////////////

class DirectorTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		currOrientation = cocos.director.ORIENT_PORTRAIT_UP
		
		var pLayer = nextDirectorTestCase()
		addChild(pLayer)
		cocos.director.replaceScene(this)
	}
	
	function mainMenuCallback(pSender)
	{
		cocos.director.deviceOrientation = 0
		base.mainMenuCallback(pSender)
	}
}

return DirectorTestScene()