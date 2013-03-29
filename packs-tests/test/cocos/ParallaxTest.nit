var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var TAG =
{
	NODE = 0
	GROSSINI = 1
}

////////////////////////////////////////////////////////////////////////////////

class ParallaxTest
{
	var _sceneIndex = 0
	
	function _createLayer(index)
	{
		var layer = null
		
		switch (index)
		{
		case 0:
			layer = Parallax1()
			break
		case 1:
			layer = Parallax2()
			break
		}
		
		return layer
	}
	
	function nextAction()
	{
		print("nextAction")
		switch (_sceneIndex)
		{
		case 0:
			_sceneIndex = 1
			break
		case 1:
			_sceneIndex = 0
			break
		}		
		return _createLayer(_sceneIndex)
	}
	function backAction()
	{
		print("backAction")
		switch (_sceneIndex)
		{
		case 0:
			_sceneIndex = 1
			break
		case 1:
			_sceneIndex = 0
			break
		}	
		return _createLayer(_sceneIndex)
	}
	function RestartAction()
	{
		return _createLayer(_sceneIndex)
	}
}

var parallaxtest = ParallaxTest()

////////////////////////////////////////////////////////////////////////////////

class ParallaxDemo : cc.ScriptLayer
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var lavel = cc.LabelTTF(title(), "Arial", 32)
		addChild(lavel,1)
		lavel.position = cc.Point(s.width/2, s.height - 60)
		
		var item1 = cc.MenuItemImage(pack.locate("b1.png"), pack.locate("b2.png"), this, backCallback)
		var item2 = cc.MenuItemImage(pack.locate("r1.png"), pack.locate("r2.png"), this, restartCallback)
		var item3 = cc.MenuItemImage(pack.locate("f1.png"), pack.locate("f2.png"), this, nextCallback)

		var menu = cc.Menu(item1, item2, item3)

		menu.position = cc.Point.ZERO
		item1.position = cc.Point(s.width/2 - 100,30)
		item2.position = cc.Point(s.width/2, 30)
		item3.position = cc.Point(s.width/2 + 100,30)

		addChild(menu, 1)
	}
	
	function restartCallback(evt: cc.MenuItemEvent)
	{
		var s =  CocosParallaxTestScene()
		s.addChild(parallaxtest.RestartAction())
		cocos.director.replaceScene(s)
	}
	
    function nextCallback(evt: cc.MenuItemEvent)
	{
		var s =  CocosParallaxTestScene()
		s.addChild(parallaxtest.nextAction())
		cocos.director.replaceScene(s)
	}
	
    function backCallback(evt: cc.MenuItemEvent)
	{
		var s =  CocosParallaxTestScene()
		s.addChild(parallaxtest.backAction())
		cocos.director.replaceScene(s)
	}
	
	function title()
	{
		return "No title"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Parallax1 : ParallaxDemo
{
	constructor()
	{
		base.constructor()
	
		var cocosImage = cc.Sprite(pack.locate("powered.png"))
		cocosImage.scale(2.5)
		cocosImage.anchorPoint = cc.Point(0, 0)
		
		var tilemap = cc.TileMapAtlas(pack.locate("tiles.png", "*tile*"), pack.locate("levelmap.tga", "*tile*"), 16,16)
		//tilemap.releaseMap()
		
		tilemap.anchorPoint = cc.Point(0, 0)
		tilemap.Texture.setAntiAliasTexParameters()
		
		var background = cc.Sprite(pack.locate("background.png"))
		background.scale(1.5)
		background.anchorPoint = cc.Point(0, 0)
		
		var voidNode = cc.ParallaxNode()
		voidNode.addChild(background, -1, cc.Point(0.4,0.5), cc.Point.ZERO)
		voidNode.addChild(tilemap, 1, cc.Point(2.2,1), cc.Point(0,-200))
		voidNode.addChild(cocosImage, 2, cc.Point(3, 2.5), cc.Point(200,800))
		
		var goUp = cc.action.MoveBy(4, cc.Point(0, -500))
		var goDown = goUp.reverse()
		var go = cc.action.MoveBy(8, cc.Point(-1000, 0))
		var goBack = go.reverse()
		var seq = cc.action.Sequence(goUp, go, goDown, goBack)	

		voidNode.runAction(cc.action.RepeatForever(seq))
		
		addChild(voidNode)		
	}
	
	function title()
	{
		return "Parallax: parent and 3 children"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Parallax2 : ParallaxDemo
{
	constructor()
	{
		base.constructor()
	
		touchEnabled = true
		channel().bind(EVT.CC_TOUCH_MOVED, this, onTouchMoved)
	
		var cocosImage = cc.Sprite(pack.locate("powered.png"))
		cocosImage.scale(2.5)
		cocosImage.anchorPoint = cc.Point(0, 0)
		
		var tilemap = cc.TileMapAtlas(pack.locate("tiles.png", "*tile*"), pack.locate("levelmap.tga", "*tile*"), 16,16)
		//tilemap.releaseMap()
		
		tilemap.anchorPoint = cc.Point(0, 0)
		tilemap.Texture.setAntiAliasTexParameters()
		
		var background = cc.Sprite(pack.locate("background.png"))
		background.scale(1.5)
		background.anchorPoint = cc.Point(0, 0)
		
		var voidNode = cc.ParallaxNode()
		voidNode.addChild(background, -1, cc.Point(0.4,0.5), cc.Point.ZERO)
		voidNode.addChild(tilemap, 1, cc.Point(1,1), cc.Point(0,-200))
		voidNode.addChild(cocosImage, 2, cc.Point(3, 2.5), cc.Point(200,1000))
		
		addChild(voidNode, 0, TAG.NODE)		
	}
	
	function onTouchMoved(evt: cc.TouchEvent)
	{
		var touchLocation = evt.touch0.locationInView(evt.touch0.view)
		var prevLocation = evt.touch0.previousLocationInView(evt.touch0.view)
		
		var convertedLocation = cocos.director.toGl(touchLocation)
		var convertedPrevLocation = cocos.director.toGl(prevLocation)
		
		var diff = convertedLocation - convertedPrevLocation
		
		var node = getChildByTag(TAG.NODE)
		var currentPos = node.position
		
		node.position = currentPos + diff
	}
	
	function title()
	{
		return "Parallax: drag screen"
	}
}

////////////////////////////////////////////////////////////////////////////////

class CocosParallaxTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		addChild(Parallax1())
		cocos.director.replaceScene(this)
	}
}

return CocosParallaxTestScene()