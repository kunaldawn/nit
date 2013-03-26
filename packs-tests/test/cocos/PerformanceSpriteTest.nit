var pack = script.locator

kMaxNodes := 5000
kNodesIncrease := 50
TEST_COUNT := 7

kTagInfoLayer := 1
kTagMainLayer := 2
kTagMenuLayer := (kMaxNodes + 1000)
	
s_nSpriteCurCase := 0

function runSpriteTest()
{
    var pScene = SpritePerformTest1()
    pScene.initWithSubTest(1, 50);
	cocos.director.replaceScene(pScene);
}

////////////////////////////////////////////////////////
//
// SubTest
//
////////////////////////////////////////////////////////
class SubTest
{
	subtestNumber = null
    batchNode = null
	parent = null
	
	constructor()
	{
	}
	
	function initWithSubTest(nSubTest, p)
	{
		subtestNumber = nSubTest
		parent = p
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
		
		switch ( subtestNumber)
		{
			case 1:
			case 4:
			case 7:
				break;

			case 2:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
				batchNode = cc.SpriteBatchNode(pack.locate("grossinis_sister1.png"), 100)
				p.addChild(batchNode, 0)
				break;
			case 3:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444);
				batchNode = cc.SpriteBatchNode(pack.locate("grossinis_sister1.png"), 100)
				p.addChild(batchNode, 0)
				break;
			case 5:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
				batchNode = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"), 100)
				p.addChild(batchNode, 0)
				break;				
			case 6:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444);
				batchNode = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"), 100)
				p.addChild(batchNode, 0)
				break;
			case 8:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
				batchNode = cc.SpriteBatchNode(pack.locate("spritesheet1.png"), 100)
				p.addChild(batchNode, 0)
				break;
			case 9:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444)
				batchNode = cc.SpriteBatchNode(pack.locate("spritesheet1.png"), 100)
				p.addChild(batchNode, 0)
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
		switch(subtestNumber)
		{
			case 1:
            {
                sprite = cc.Sprite(pack.locate("grossinis_sister1.png"))
                parent.addChild(sprite, 0, tag+100)
                break;
            }
			case 2:
			case 3: 
			{
				sprite = cc.Sprite(batchNode, cc.Rect(0, 0, 52, 139))
				batchNode.addChild(sprite, 0, tag+100)
				break;
			}
			case 4:
			{
				var idx = math.floor((math.random() * 1400 / 100) + 1)
				var index = (idx<10) ? "0"+idx : ""+idx
				var str = "grossini_dance_" + index + ".png"
				sprite = cc.Sprite(pack.locate(str))
				parent.addChild(sprite, 0, tag+100)
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
				sprite = cc.Sprite(batchNode, cc.Rect(x,y,85,121))
				batchNode.addChild(sprite, 0, tag+100)
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
				parent.addChild(sprite, 0, tag+100);
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
				sprite = cc.Sprite(batchNode, cc.Rect(x,y,32,32))
				batchNode.addChild(sprite, 0, tag+100)
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
		switch (subtestNumber)
		{
			case 1:
			case 4:
			case 7:
				parent.removeChildByTag(tag+100, true);
				break;
			case 2:
			case 3:
			case 5:
			case 6:
			case 8:
			case 9:
				batchNode.removeChildAtIndex(tag, true);
				//			[batchNode removeChildByTag:tag+100 cleanup:YES];
				break;
			default:
				break;
		}
	}
}
////////////////////////////////////////////////////////
//
// SpriteMenuLayer
//
////////////////////////////////////////////////////////
class SpriteMenuLayer : PerformBasicLayer
{
	constructor(bControlMenuVisible, nMaxCases, nCurCase)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
	}
	
	function showCurrentTest()
	{
		var pScene = null
		var pPreScene = parent
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
		s_nSpriteCurCase = m_nCurCase;

		if (pScene)
		{
			pScene.initWithSubTest(nSubTest, nNodes)
			cocos.director.replaceScene(pScene)
		}
	}
}
////////////////////////////////////////////////////////
//
// SpriteMainScene
//
////////////////////////////////////////////////////////
class SpriteMainScene : cc.ScriptScene
{
	lastRenderedCount = null
    quantityNodes = null
    m_pSubTest = null
    subtestNumber = null
	
	function getSubTestNum()
	{
		return subtestNumber;
	}
	function getNodesNum()
	{
		return quantityNodes;
	}
	
	function initWithSubTest(asubtest, nNodes)
	{
		subtestNumber = asubtest;
		m_pSubTest = SubTest()
		m_pSubTest.initWithSubTest(asubtest, this)
		
		var s = cocos.director.winSize
		
		lastRenderedCount = 0
		quantityNodes   = 0
		
		cc.MenuItemFont.setDefaultFontSize(65)
		var decrease = cc.MenuItemFont("- ", this, onDecrease)
		decrease.color = nit.Color(0, 200/255, 20/255, 1)
		var increase = cc.MenuItemFont("+", this, onIncrease)
		increase.color = nit.Color(0, 200/255, 20/255, 1)
		
		var menu = cc.Menu(decrease, increase)
		menu.alignItemsHorizontally()
		menu.position = cc.Point(s.width/2, s.height - 65)
		addChild(menu, 1)
		
		var infoLabel = cc.LabelTTF("0 nodes", "Marker Felt", 30)
		infoLabel.color = nit.Color(0, 200/255, 20/255, 0)
		infoLabel.position = cc.Point(s.width/2, s.height - 90)
		addChild(infoLabel, 1, kTagInfoLayer)
		
		//add menu
		var pMenu = SpriteMenuLayer(true, TEST_COUNT, s_nSpriteCurCase)
		addChild(pMenu, 1, kTagMenuLayer)
		
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
		var label = cc.LabelTTF( title(), "Arial", 40)
		addChild(label, 1)
		label.position = cc.Point(s.width/2, s.height- 32)
		label.color = nit.Color(255/255, 255/255, 40/255, 0)
		
		while(quantityNodes < nNodes)
			onIncrease(this)
	}
	
	function title()
	{
		return "No title"
	}
	
	function testNCallback(pSender)
	{
		subtestNumber = pSender.tag
		var pMenu = getChildByTag(kTagMenuLayer)
		pMenu.restartCallback(pSender)
	}
	
	function updateNodes()
	{
		if (quantityNodes != lastRenderedCount)
		{
			var infoLabel =  getChildByTag(kTagInfoLayer)
			var str = "" + quantityNodes + " nodes"
			infoLabel.string = str
			
			lastRenderedCount = quantityNodes
		}
	}
	
	function onIncrease(pSender)
	{
		if( quantityNodes >= kMaxNodes)
			return

		for( var i=0; i< kNodesIncrease; i++)
		{
			var sprite = m_pSubTest.createSpriteWithTag(quantityNodes);
			doTest(sprite);
			quantityNodes++;
		}

		updateNodes();
	}

	function onDecrease(pSender)
	{
		if( quantityNodes <= 0 )
			return;

		for( var i=0; i < kNodesIncrease; i++)
		{
			quantityNodes--;
			m_pSubTest.removeByTag(quantityNodes);
		}

		updateNodes();
	}
}
////////////////////////////////////////////////////////
//
// For test functions
//
////////////////////////////////////////////////////////
function performanceActions(pSprite)
{
	var size = cocos.director.winSize
	pSprite.position = cc.Point( math.rand()%size.width, math.rand()%size.height)
	
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

function performanceActions20(pSprite)
{
	var size = cocos.director.winSize
	if (math.random() < 0.2)
		pSprite.position = cc.Point( math.rand()%size.width, math.rand()%size.height)
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

function performanceRotationScale(pSprite)
{
	var size = cocos.director.winSize
	pSprite.position = cc.Point( math.rand()%size.width, math.rand()%size.height)
	pSprite.rotation = math.random()*360
	pSprite.scale(math.random()*2)
}

function performancePosition(pSprite)
{
	var size = cocos.director.winSize
	pSprite.position = cc.Point( math.rand()%size.width, math.rand()%size.height)
}

function performanceout20(pSprite)
{
    var size = cocos.director.winSize

    if( math.random() < 0.2 )
        pSprite.position= cc.Point(math.rand()%size.width, math.rand() % size.height)
    else
        pSprite.position = cc.Point( -1000, -1000)
}

function performanceOut100(pSprite)
{
	pSprite.position = cc.Point(-1000, -1000)
}

function performanceScale(pSprite)
{
	var size = cocos.director.winSize
	pSprite.position = cc.Point( math.rand()%size.width, math.rand()%size.height)
	pSprite.scale(math.random() * 100 / 50)
}
////////////////////////////////////////////////////////
//
// SpritePerformTest1
//
////////////////////////////////////////////////////////
class SpritePerformTest1 : SpriteMainScene
{
	function title()
	{
		var str = "A ("+subtestNumber+") position"
		return str;
	}

	function doTest( sprite)
	{
		performancePosition(sprite);
	}
}
////////////////////////////////////////////////////////
//
// SpritePerformTest2
//
////////////////////////////////////////////////////////
class SpritePerformTest2 : SpriteMainScene
{
	function title()
	{
		var str = "B ("+subtestNumber+") scale"
		return str;
	}
	function doTest(sprite)
	{
		performanceScale(sprite);
	}
}
////////////////////////////////////////////////////////
//
// SpritePerformTest3
//
////////////////////////////////////////////////////////
class SpritePerformTest3 : SpriteMainScene
{
	function title()
	{
		var str = "C ("+subtestNumber+") scale+rot"
		return str;
	}

	function doTest(sprite)
	{
		performanceRotationScale(sprite);
	}
}
////////////////////////////////////////////////////////
//
// SpritePerformTest4
//
////////////////////////////////////////////////////////
class SpritePerformTest4 : SpriteMainScene
{
	function title()
	{
		var str = "D ("+subtestNumber+") 100/% out"
		return str;
	}
	function doTest(sprite)
	{
		performanceOut100(sprite);
	}
}
////////////////////////////////////////////////////////
//
// SpritePerformTest5
//
////////////////////////////////////////////////////////
class SpritePerformTest5 : SpriteMainScene
{
	function title()
	{
		var str = "E ("+subtestNumber+") 80/% out"
		return str;
	}
	
	function doTest(sprite)
	{
		performanceout20(sprite);
	}
}
////////////////////////////////////////////////////////
//
// SpritePerformTest6
//
////////////////////////////////////////////////////////
class SpritePerformTest6 : SpriteMainScene
{
	function title()
	{
		var str = "F ("+subtestNumber+") actions"
		return str;
	}

	function doTest(sprite)
	{
		performanceActions(sprite);
	}
}
////////////////////////////////////////////////////////
//
// SpritePerformTest7
//
////////////////////////////////////////////////////////
class SpritePerformTest7 : SpriteMainScene
{
	function title()
	{
		var str = "G ("+subtestNumber+") 80/% out"
		return str;
	}
		
	function doTest(sprite)
	{
		performanceActions20(sprite);
	}
}