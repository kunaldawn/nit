var pack = script.locator

kTagLayer := 1

sceneIdx 	:= -1; 

MAX_LAYER 	:= 1
s_currentOrientation := 0

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

function createZwoptexLayer(nIndex)
{
	switch(nIndex)
	{
		case 0:	return ZwoptexGenericTest()
	}
}

function nextZwoptexTest()
{
	sceneIdx++;
	sceneIdx = sceneIdx % MAX_LAYER;

	var pLayer = createZwoptexLayer(sceneIdx);
	
	return pLayer;
}

function backZwoptexTest()
{
	sceneIdx--;
	var total = MAX_LAYER;
	if( sceneIdx < 0 )
		sceneIdx += total;	
	
	var pLayer = createZwoptexLayer(sceneIdx);

	return pLayer;
}

function restartZwoptexTest()
{
	var pLayer = createZwoptexLayer(sceneIdx);

	return pLayer;
} 

class ZwoptexTest : cc.ScriptLayer
{
	var bRet = false;
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		var x = size.width
		var y = size.height
	
		var label = cc.LabelTTF( title(), "Arial", 26)
		label.position = cc.Point(x/2, y-50)
		addChild(label, 1)
		
		var strSubtitle = subtitle()
		if ( strSubtitle != "")
		{
			var l = cc.LabelTTF( strSubtitle, "Thonburi", 16)
			addChild(l, 1)
			l.position = cc.Point(size.width/2, size.height-80)
		}
		
		var item1 = cc.MenuItemImage(s_pPathB1,s_pPathB2, this,backCallBack);
		var item2 = cc.MenuItemImage(s_pPathR1, s_pPathR2, this,restartCallBack)	
		var item3 = cc.MenuItemImage(s_pPathF1, s_pPathF2, this,nextCallBack);
		
		var menu = cc.Menu(item1, item2, item3);
		menu.position= cc.Point(0,0);
		item1.position= cc.Point( size.width/2 - 100,30) ;
		item2.position= cc.Point( size.width/2, 30) ;
		item3.position= cc.Point( size.width/2 + 100,30) ;
		this.addChild( menu, 1 );	
		
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
		s.addChild( restartZwoptexTest() )
		cocos.director.replaceScene(s)
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = ZwoptexTestScene()
		s.addChild( nextZwoptexTest() )
		cocos.director.replaceScene(s)
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		var s = ZwoptexTestScene()
		s.addChild( backZwoptexTest() )
		cocos.director.replaceScene(s)
	}
}

//------------------------------------------------------------------
//
// ZwoptexGenericTest
//
//------------------------------------------------------------------

class ZwoptexGenericTest : ZwoptexTest
{
	sprite1 = null
	sprite2 = null
	counter =  null
	spriteFrameIndex = 0
	
	constructor()
	{
		base.constructor()
			
		var s = cocos.director.winSize
		
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist", "*zwoptex*"))
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini-generic.plist", "*zwoptex*"))
		
		var layer1 = cc.LayerColor(nit.Color(1, 0, 0, 1), 85, 121)
		layer1.position = cc.Point(s.width/2 - 80 - (85 * 0.5), s.height/2 - (121 * 0.5))
		addChild(layer1)
		
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite1.position = cc.Point(s.width/2 -80, s.height/2)
		addChild(sprite1)
		
		sprite1.flipX = false
		sprite1.flipY = false
		
		var layer2 = cc.LayerColor(nit.Color(1, 0, 0, 1), 85, 121)
		layer2.position = cc.Point(s.width/2 + 80 - (85 * 0.5), s.height/2 - (121 * 0.5))
		addChild(layer2)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_generic_01.png"))
		sprite2.position = cc.Point(s.width/2 +80, s.height/2)
		addChild(sprite2)
		
		sprite2.flipX = false
		sprite2.flipY = false
		
		session.scheduler.once(this, startIn05Secs, 1.0)
		
		counter = 0;
	}
	
	function startIn05Secs()
	{
		session.scheduler.repeat(this, flipSprites, 0.5)
	}
	
	function flipSprites()
	{
		counter++
		
		var fx = false
		var fy = false
		var i = counter % 4
		
		switch ( i )
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
		sprite1.flipX=fx;
		sprite2.flipX=fx;
		sprite1.flipY=fy;
		sprite2.flipY=fy;
		
		spriteFrameIndex++
		if ( spriteFrameIndex > 14)
		{
			spriteFrameIndex = 1
		}
		var str1 = ""
		var str2 = ""
		
		var index = spriteFrameIndex<10 ? "0"+spriteFrameIndex : ""+spriteFrameIndex
		print(index)
		str1 = "grossini_dance_" + index + ".png"
		if (index == "09")	index = "08"
		str2 = "grossini_dance_generic_" + index + ".png"
		
		sprite1.displayedFrame = cocos.spriteFrameCache.spriteFrameByName(str1)
		sprite2.displayedFrame = cocos.spriteFrameCache.spriteFrameByName(str2)
		
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