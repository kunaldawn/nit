var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var MAX_NODES = 5000
var NODES_INCREASE = 50
var TEST_COUNT = 7

var TAG = 
{
	INFO_LAYER = 1
	MAIN_LAYER = 2
	MENU_LAYER = (MAX_NODES + 1000)
}
	
var spriteCurCase = 0

var function runSpriteTest()
{
    var pScene = SpritePerformTest1()
    pScene.initWithSubTest(1, 50);
	cocos.director.replaceScene(pScene);
}

////////////////////////////////////////////////////////////////////////////////

class SubTest
{
	var _subtestNumber = null
    var _batchNode = null
	var _parent = null
	
	constructor()
	{
	}
	
	function initWithSubTest(nSubTest, p)
	{
		_subtestNumber = nSubTest
		_parent = p
		/*
		* Tests:
		* 1: 1 (32-bit) PNG sprite of 52 x 139
		* 2: 1 (32-bit) PNG Batch Node using 1 sprite of 52 x 139
		* 3: 1 (16-bit) PNG Batch Node using 1 sprite of 52 x 139
		* 4: 1 (4-bit) PVRTC Batch Node using 1 sprite of 52 x 139

		* 5: 14 (32-bit) PNG sprites of 85 x 121 each
		* 6: 14 (32-bit) PNG Batch Node of 85 x 121 each
		* 7: 14 (16-bit) PNG Batch Node of 85 x 121 each
		* 8: 14 (4-bit) PVRTC Batch Node of 85 x 121 each

		* 9: 64 (32-bit) sprites of 32 x 32 each
		*10: 64 (32-bit) PNG Batch Node of 32 x 32 each
		*11: 64 (16-bit) PNG Batch Node of 32 x 32 each
		*12: 64 (4-bit) PVRTC Batch Node of 32 x 32 each
		*/
		
		// purge textures
		cocos.textureCache.removeTexture(cocos.textureCache.addImage(pack.locate("grossinis_sister1.png")))
		cocos.textureCache.removeTexture(cocos.textureCache.addImage(pack.locate("grossini_dance_atlas.png")))
		cocos.textureCache.removeTexture(cocos.textureCache.addImage(pack.locate("spritesheet1.png")))
		
		switch (_subtestNumber)
		{
			case 1:
			case 4:
			case 7:
				break;

			case 2:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
				_batchNode = cc.Sprite_batchNode(pack.locate("grossinis_sister1.png"), 100)
				p.addChild(_batchNode, 0)
				break;
			case 3:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444);
				_batchNode = cc.Sprite_batchNode(pack.locate("grossinis_sister1.png"), 100)
				p.addChild(_batchNode, 0)
				break;
			case 5:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
				_batchNode = cc.Sprite_batchNode(pack.locate("grossini_dance_atlas.png"), 100)
				p.addChild(_batchNode, 0)
				break;				
			case 6:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444);
				_batchNode = cc.Sprite_batchNode(pack.locate("grossini_dance_atlas.png"), 100)
				p.addChild(_batchNode, 0)
				break;
			case 8:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
				_batchNode = cc.Sprite_batchNode(pack.locate("spritesheet1.png"), 100)
				p.addChild(_batchNode, 0)
				break;
			case 9:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444)
				_batchNode = cc.Sprite_batchNode(pack.locate("spritesheet1.png"), 100)
				p.addChild(_batchNode, 0)
				break;

			default:
				break;
		}
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_DEFAULT)
	}
	
	function createSpriteWithTag(tag)
	{
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
		var sprite = null
		switch (_subtestNumber)
		{
			case 1:
            {
                sprite = cc.Sprite(pack.locate("grossinis_sister1.png"))
                _parent.addChild(sprite, 0, tag+100)
                break;
            }
			case 2:
			case 3: 
			{
				sprite = cc.Sprite(_batchNode, cc.Rect(0, 0, 52, 139))
				_batchNode.addChild(sprite, 0, tag+100)
				break;
			}
			case 4:
			{
				var idx = math.floor((math.random() * 1400 / 100) + 1)
				var index = (idx<10) ? "0"+idx : ""+idx
				var str = "grossini_dance_" + index + ".png"
				sprite = cc.Sprite(pack.locate(str))
				_parent.addChild(sprite, 0, tag+100)
				break;
			}
			case 5:
			case 6:
			{
				var y,x = 0
				var r = (math.random() * 1400 / 100)

				y = r / 5
				x = r % 5

				x *= 85
				y *= 121
				sprite = cc.Sprite(_batchNode, cc.Rect(x,y,85,121))
				_batchNode.addChild(sprite, 0, tag+100)
				break;
			}

			case 7:
			{
				var y,x = 0
				var r = (math.random() * 6400 / 100)

				y = r / 8;
				x = r % 8;

				var str = format("sprite-%d-%d.png", x, y)
				sprite = cc.Sprite(pack.locate(str))
				_parent.addChild(sprite, 0, tag+100);
				break;
			}

			case 8:
			case 9:
			{
				var y,x = 0;
				var r = (math.random() * 6400 / 100)

				y = r / 8;
				x = r % 8;
				
				x = x.tointeger()
				y = y.tointeger()
				
				x *= 32;
				y *= 32;
				sprite = cc.Sprite(_batchNode, cc.Rect(x,y,32,32))
				_batchNode.addChild(sprite, 0, tag+100)
				break;
			}

			default:
				break;
		}
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_DEFAULT)
		
		return sprite
	}
	
	function removeByTag(tag)
	{
		switch (_subtestNumber)
		{
			case 1:
			case 4:
			case 7:
				_parent.removeChildByTag(tag+100, true);
				break;
			case 2:
			case 3:
			case 5:
			case 6:
			case 8:
			case 9:
				_batchNode.removeChildAtIndex(tag, true);
				//			[_batchNode removeChildByTag:tag+100 cleanup:YES];
				break;
			default:
				break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteMenuLayer : PerformBasicLayer
{
	constructor(bControlMenuVisible, nMaxCases, nCurCase)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
	}
	
	function showCurrentTest()
	{
		var pScene = null
		var pPreScene = _parent
		var nSubTest = pPreScene.getSubTestNum()
		var nNodes = pPreScene.getNodesNum()
		
		switch (m_nCurCase)
		{
		case 0:
			pScene = SpritePerformTest1()
			break;
		case 1:
			pScene = SpritePerformTest2()
			break;
		case 2:
			pScene = SpritePerformTest3()
			break;
		case 3:
			pScene = SpritePerformTest4()
			break;
		case 4:
			pScene = SpritePerformTest5()
			break;
		case 5:
			pScene = SpritePerformTest6()
			break;
		case 6:
			pScene = SpritePerformTest7()
			break;
		}
		spriteCurCase = m_nCurCase;

		if (pScene)
		{
			pScene.initWithSubTest(nSubTest, nNodes)
			cocos.director.replaceScene(pScene)
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteMainScene : cc.ScriptScene
{
	var _lastRenderedCount = null
    var _quantityNodes = null
    var _subTest = null
    var _subtestNumber = null
	
	function getSubTestNum()
	{
		return _subtestNumber;
	}
	function getNodesNum()
	{
		return _quantityNodes;
	}
	
	function initWithSubTest(asubtest, nNodes)
	{
		_subtestNumber = asubtest;
		_subTest = SubTest()
		_subTest.initWithSubTest(asubtest, this)
		
		var s = cocos.director.winSize
		
		_lastRenderedCount = 0
		_quantityNodes   = 0
		
		cc.MenuItemFont.setDefaultFontSize(65)
		var decrease = cc.MenuItemFont("- ", this, onDecrease)
		decrease.color = nit.Color(0, 200/255, 20/255, 1)
		var increase = cc.MenuItemFont("+", this, onIncrease)
		increase.color = nit.Color(0, 200/255, 20/255, 1)
		
		var menu = cc.Menu(decrease, increase)
		menu.alignItemsHorizontally()
		menu.position = cc.Point(s.width/2, s.height - 65)
		addChild(menu, 1)
		
		var infoLabel = cc.LabelTTF("0 nodes", "Arial", 30)
		infoLabel.color = nit.Color(0, 200/255, 20/255, 0)
		infoLabel.position = cc.Point(s.width/2, s.height - 90)
		addChild(infoLabel, 1, TAG.INFO_LAYER)
		
		//add menu
		var pMenu = SpriteMenuLayer(true, TEST_COUNT, spriteCurCase)
		addChild(pMenu, 1, TAG.MENU_LAYER)
		
		//Sub Tests
		cc.MenuItemFont.setDefaultFontSize(32)
		var pSubMenu = cc.Menu()
		for (var i = 1; i <= 9; i++)
		{
			var str = ""+i
			var itemFont = null
			itemFont = cc.MenuItemFont(str, this, @testNCallback(itemFont))
			itemFont.tag = i
			pSubMenu.addChild(itemFont, 10)
			
			if (i<=3)
				itemFont.color = nit.Color(200/255, 20/255, 20/255, 1)
			else if (i<=6)
				itemFont.color = nit.Color(0, 200/255, 20/255, 1)
			else
				itemFont.color = nit.Color(0, 20/255, 200/255, 1)
		}
		
		pSubMenu.alignItemsHorizontally()
		pSubMenu.position = cc.Point(s.width/2, 80)
		addChild(pSubMenu, 2)
		
		//Add title label
		var label = cc.LabelTTF(title(), "Arial", 40)
		addChild(label, 1)
		label.position = cc.Point(s.width/2, s.height- 32)
		label.color = nit.Color(255/255, 255/255, 40/255, 0)
		
		while(_quantityNodes < nNodes)
			onIncrease(this)
	}
	
	function title()
	{
		return "No title"
	}
	
	function testNCallback(pSender)
	{
		_subtestNumber = pSender.tag
		var pMenu = getChildByTag(TAG.MENU_LAYER)
		pMenu.restartCallback(pSender)
	}
	
	function updateNodes()
	{
		if (_quantityNodes != _lastRenderedCount)
		{
			var infoLabel =  getChildByTag(TAG.INFO_LAYER)
			var str = "" + _quantityNodes + " nodes"
			infoLabel.string = str
			
			_lastRenderedCount = _quantityNodes
		}
	}
	
	function onIncrease(pSender)
	{
		if (_quantityNodes >= MAX_NODES)
			return

		for (var i=0; i< NODES_INCREASE; i++)
		{
			var sprite = _subTest.createSpriteWithTag(_quantityNodes);
			doTest(sprite);
			_quantityNodes++;
		}

		updateNodes();
	}

	function onDecrease(pSender)
	{
		if (_quantityNodes <= 0)
			return;

		for (var i=0; i < NODES_INCREASE; i++)
		{
			_quantityNodes--;
			_subTest.removeByTag(_quantityNodes);
		}

		updateNodes();
	}
}

////////////////////////////////////////////////////////////////////////////////

var function performanceActions(pSprite)
{
	var size = cocos.director.winSize
	pSprite.position = cc.Point(math.rand()%size.width, math.rand()%size.height)
	
	var period = 0.5 + (math.rand()%1000) / 500.0
	var rot = cc.action.RotateBy(period, 360*math.random())
	var rot_back = rot.reverse()
	var permanentRotation = cc.action.RepeatForever(cc.action.Sequence(rot, rot_back))
	pSprite.runAction(permanentRotation)
	
	var growDuration = 0.5 + math.rand()%1000 / 500.0
	var grow = cc.action.ScaleBy(growDuration, 0.5, 0.5)
	var permanentScaleLoop = cc.action.RepeatForever(cc.action.Sequence(grow, grow.reverse()))
	pSprite.runAction(permanentScaleLoop)
}

var function performanceActions20(pSprite)
{
	var size = cocos.director.winSize
	if (math.random() < 0.2)
		pSprite.position = cc.Point(math.rand()%size.width, math.rand()%size.height)
	else
		pSprite.position = cc.Point(-1000, -1000)
	
	var period = 0.5 + (math.rand()%1000) / 500.0
	var rot = cc.action.RotateBy(period, 360*math.random())
	var rot_back = rot.reverse()
	var permanentRotation = cc.action.RepeatForever(cc.action.Sequence(rot, rot_back))
	pSprite.runAction(permanentRotation)
	
	var growDuration = 0.5 + math.rand()%1000 / 500.0
	var grow = cc.action.ScaleBy(growDuration, 0.5, 0.5)
	var permanentScaleLoop = cc.action.RepeatForever(cc.action.Sequence(grow, grow.reverse()))
	pSprite.runAction(permanentScaleLoop)
}

var function performanceRotationScale(pSprite)
{
	var size = cocos.director.winSize
	pSprite.position = cc.Point(math.rand()%size.width, math.rand()%size.height)
	pSprite.rotation = math.random()*360
	pSprite.scale(math.random()*2)
}

var function performancePosition(pSprite)
{
	var size = cocos.director.winSize
	pSprite.position = cc.Point(math.rand()%size.width, math.rand()%size.height)
}

var function performanceout20(pSprite)
{
    var size = cocos.director.winSize

    if (math.random() < 0.2)
        pSprite.position= cc.Point(math.rand()%size.width, math.rand() % size.height)
    else
        pSprite.position = cc.Point(-1000, -1000)
}

var function performanceOut100(pSprite)
{
	pSprite.position = cc.Point(-1000, -1000)
}

var function performanceScale(pSprite)
{
	var size = cocos.director.winSize
	pSprite.position = cc.Point(math.rand()%size.width, math.rand()%size.height)
	pSprite.scale(math.random() * 100 / 50)
}

////////////////////////////////////////////////////////////////////////////////

class SpritePerformTest1 : SpriteMainScene
{
	function title()
	{
		var str = "A ("+_subtestNumber+") position"
		return str;
	}

	function doTest(sprite)
	{
		performancePosition(sprite);
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpritePerformTest2 : SpriteMainScene
{
	function title()
	{
		var str = "B ("+_subtestNumber+") scale"
		return str;
	}
	function doTest(sprite)
	{
		performanceScale(sprite);
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpritePerformTest3 : SpriteMainScene
{
	function title()
	{
		var str = "C ("+_subtestNumber+") scale+rot"
		return str;
	}

	function doTest(sprite)
	{
		performanceRotationScale(sprite);
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpritePerformTest4 : SpriteMainScene
{
	function title()
	{
		var str = "D ("+_subtestNumber+") 100/% out"
		return str;
	}
	function doTest(sprite)
	{
		performanceOut100(sprite);
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpritePerformTest5 : SpriteMainScene
{
	function title()
	{
		var str = "E ("+_subtestNumber+") 80/% out"
		return str;
	}
	
	function doTest(sprite)
	{
		performanceout20(sprite);
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpritePerformTest6 : SpriteMainScene
{
	function title()
	{
		var str = "F ("+_subtestNumber+") actions"
		return str;
	}

	function doTest(sprite)
	{
		performanceActions(sprite);
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpritePerformTest7 : SpriteMainScene
{
	function title()
	{
		var str = "G ("+_subtestNumber+") 80/% out"
		return str;
	}
		
	function doTest(sprite)
	{
		performanceActions20(sprite);
	}
}