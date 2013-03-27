var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var TAG =
{
	LAYER = 1
}

var sceneIdx = -1; 

var MAX_LAYER = 1
var _currOrientation = 0

////////////////////////////////////////////////////////////////////////////////

var function createZwoptexLayer(nIndex)
{
	switch (nIndex)
	{
		case 0:	return ZwoptexGenericTest()
	}
}

var function nextZwoptexTest()
{
	sceneIdx++;
	sceneIdx = sceneIdx % MAX_LAYER;

	var pLayer = createZwoptexLayer(sceneIdx);
	
	return pLayer;
}

var function backZwoptexTest()
{
	sceneIdx--;
	var total = MAX_LAYER;
	if (sceneIdx < 0)
		sceneIdx += total;	
	
	var pLayer = createZwoptexLayer(sceneIdx);

	return pLayer;
}

var function restartZwoptexTest()
{
	var pLayer = createZwoptexLayer(sceneIdx);

	return pLayer;
} 

////////////////////////////////////////////////////////////////////////////////

class ZwoptexTest : cc.ScriptLayer
{
	var bRet = false;
	
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
		
		var item1 = cc.MenuItemImage(s_pPathB1,s_pPathB2, this,backCallBack);
		var item2 = cc.MenuItemImage(s_pPathR1, s_pPathR2, this,restartCallBack)	
		var item3 = cc.MenuItemImage(s_pPathF1, s_pPathF2, this,nextCallBack);
		
		var menu = cc.Menu(item1, item2, item3);
		menu.position= cc.Point(0,0);
		item1.position= cc.Point(size.width/2 - 100,30) ;
		item2.position= cc.Point(size.width/2, 30) ;
		item3.position= cc.Point(size.width/2 + 100,30) ;
		addChild(menu, 1);	
	}
	
	function title()
	{
		return "No title";
	}
	
	function subtitle()
	{
		return "";
	}
	
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		var s = ZwoptexTestScene()
		s.addChild(restartZwoptexTest())
		cocos.director.replaceScene(s)
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = ZwoptexTestScene()
		s.addChild(nextZwoptexTest())
		cocos.director.replaceScene(s)
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		var s = ZwoptexTestScene()
		s.addChild(backZwoptexTest())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class ZwoptexGenericTest : ZwoptexTest
{
	var _sprite1 = null
	var _sprite2 = null
	var _counter = null
	var _spriteFrameIndex = 0
	
	constructor()
	{
		base.constructor()
			
		var s = cocos.director.winSize
		
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist", "*zwoptex*"))
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini-generic.plist", "*zwoptex*"))
		
		var layer1 = cc.LayerColor(nit.Color(1, 0, 0, 1), 85, 121)
		layer1.position = cc.Point(s.width/2 - 80 - (85 * 0.5), s.height/2 - (121 * 0.5))
		addChild(layer1)
		
		_sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		_sprite1.position = cc.Point(s.width/2 -80, s.height/2)
		addChild(_sprite1)
		
		_sprite1.flipX = false
		_sprite1.flipY = false
		
		var layer2 = cc.LayerColor(nit.Color(1, 0, 0, 1), 85, 121)
		layer2.position = cc.Point(s.width/2 + 80 - (85 * 0.5), s.height/2 - (121 * 0.5))
		addChild(layer2)
		
		_sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_generic_01.png"))
		_sprite2.position = cc.Point(s.width/2 +80, s.height/2)
		addChild(_sprite2)
		
		_sprite2.flipX = false
		_sprite2.flipY = false
		
		cocos.director.scheduler.once(this, startIn05Secs, 1.0)
		
		_counter = 0;
	}
	
	function startIn05Secs()
	{
		cocos.director.scheduler.repeat(this, flipSprites, 0.5)
	}
	
	function flipSprites()
	{
		_counter++
		
		var fx = false
		var fy = false
		var i = _counter % 4
		
		switch (i)
		{
		case 0:
			fx = false;
			fy = false;
			break;
		case 1:
			fx = true;
			fy = false;
			break;
		case 2:
			fx = false;
			fy = true;
			break;
		case 3:
			fx = true;
			fy = true;
			break;
		}
		
		_sprite1.flipX = fx;
		_sprite2.flipX = fx;
		_sprite1.flipY = fy;
		_sprite2.flipY = fy;
		
		_spriteFrameIndex++
		if (_spriteFrameIndex > 14)
		{
			_spriteFrameIndex = 1
		}

		var str1 = format("grossini_dance_%02d.png", _spriteFrameIndex)
		var str2 = format("grossini_dance_generic_%02d.png", _spriteFrameIndex)
		
		_sprite1.displayedFrame = cocos.spriteFrameCache.spriteFrameByName(str1)
		_sprite2.displayedFrame = cocos.spriteFrameCache.spriteFrameByName(str2)
	}
	
	function title()
	{	
		return "Zwoptex Tests";
	}
	
	function subtitle()
	{
		return "Coordinate Formats, Rotation, Trimming, flipX/Y";
	}
}

////////////////////////////////////////////////////////////////////////////////

class ZwoptexTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{	
		var pLayer = nextZwoptexTest()
		addChild(pLayer)
		cocos.director.replaceScene(this)
	}
}

return ZwoptexTestScene()