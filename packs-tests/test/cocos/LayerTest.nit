var pack = script.locator

kTagLayer := 1

sceneIdx 	:= -1; 

MAX_LAYER 	:= 4

class LayerTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = nextTestAction()
		addChild(pLayer)
		cocos.director.replaceScene(this)
	}
}

function createTestLayer(nIndex)
{
	switch(nIndex)
	{
		case 0: return  LayerTest1();
		case 1: return  LayerTest2();
		case 2: return  LayerTestBlend();
        case 3: return  LayerGradient();
	}

	return NULL;
}

function nextTestAction()
{
	sceneIdx++;
	sceneIdx = sceneIdx % MAX_LAYER;

	var pLayer = createTestLayer(sceneIdx);
	
	return pLayer;
}

function backTestAction()
{
	sceneIdx--;
	var total = MAX_LAYER;
	if( sceneIdx < 0 )
		sceneIdx += total;	
	
	var pLayer = createTestLayer(sceneIdx);

	return pLayer;
}

function restartTestAction()
{
	var pLayer = createTestLayer(sceneIdx);

	return pLayer;
} 


class LayerTest : cc.ScriptLayer
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		var x = size.width
		var y = size.height
	
		var label = cc.LabelTTF( title(), "Marker Felt", 28)
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
		var s = LayerTestScene()
		s.addChild( restartTestAction() )
		cocos.director.replaceScene(s)
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = LayerTestScene()
		s.addChild( nextTestAction() )
		cocos.director.replaceScene(s)
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		var s = LayerTestScene()
		s.addChild( backTestAction() )
		cocos.director.replaceScene(s)
	}
}

//------------------------------------------------------------------
//
// LayerTest1
//
//------------------------------------------------------------------
class LayerTest1 : LayerTest
{
	constructor()
	{
		base.constructor()
			
		var size = cocos.director.winSize
		var layer = cc.LayerColor(nit.Color(1, 0, 0, 0.5), 200, 200)
		layer.relativeAnchorPoint = true
		layer.position = cc.Point(size.width/2, size.height/2)
		addChild(layer, 1, kTagLayer)
		
		this.touchEnabled = true;
		this.channel().priority(-10000).bind(Events.OnCCTouchBegin, this, ccTouchesBegin)
		this.channel().priority(-10000).bind(Events.OnCCTouchMoved, this, ccTouchesMoved)
		this.channel().priority(-10000).bind(Events.OnCCTouchEnded, this, ccTouchesEnded)
	}
	
	function updateSize( touch)
	{
		var touchLocation = touch.locationInView(touch.view)
		touchLocation = cocos.director.toGl(touchLocation)
		
		var s = cocos.director.winSize
		var newSize = cc.Size(math.fabs(touchLocation.x - s.width/2)*2, 
								math.fabs(touchLocation.y - s.height/2)*2)
		var l = getChildByTag(kTagLayer)
		l.contentSize = newSize
	}
	
	function ccTouchesBegin(evt: cc.TouchEvent)
	{
		var touch = evt.touches[0]
		updateSize(touch)
		
		return true
	}
	function ccTouchesMoved(evt: cc.TouchEvent)
	{
		var touch = evt.touches[0]
		updateSize(touch)
	}
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		var touch = evt.touches[0]
		updateSize(touch)
	}
	
	function title()
	{	
		return "ColorLayer resize (tap & move)";
	}
}
//------------------------------------------------------------------
//
// LayerTest2
//
//------------------------------------------------------------------
class LayerTest2 : LayerTest
{
	constructor()
	{
		base.constructor()
			
		var size = cocos.director.winSize
		var layer1 = cc.LayerColor(nit.Color(1, 1, 0, 80/255), 100, 300)
		layer1.relativeAnchorPoint = true
		layer1.position = cc.Point(size.width/3, size.height/2)
		addChild(layer1, 1)
		
		var layer2 = cc.LayerColor(nit.Color(0, 0, 1, 1), 100, 300)
		layer2.relativeAnchorPoint = true
		layer2.position = cc.Point(size.width/3 * 2, size.height/2)
		addChild(layer2, 1)
		
		var actionTint = cc.action.TintBy(2, -255, -127, 0)
		var actionTintBack = actionTint.reverse()
		var seq1 = cc.action.Sequence(actionTint, actionTintBack)
		layer1.runAction(seq1)
		
		var actionFade = cc.action.FadeOut(2.0)
		var actionFadeBack = actionFade.reverse()
		var seq2 = cc.action.Sequence(actionFade, actionFadeBack)
		layer2.runAction(seq2)
	}
	
	function title()
	{	
		return "ColorLayer: fade and tint";
	}
}
//------------------------------------------------------------------
//
// LayerTestBlend
//
//------------------------------------------------------------------
class LayerTestBlend : LayerTest
{
	constructor()
	{
		base.constructor()
			
		var s = cocos.director.winSize
		var layer1 = cc.LayerColor(nit.Color(1, 1, 1, 80/255))
		
		var sister1 = cc.Sprite(s_pPathSister1)
		var sister2 = cc.Sprite(s_pPathSister2)
		
		addChild(sister1)
		addChild(sister2)
		addChild(layer1, 100, kTagLayer)
		
		sister1.position = cc.Point(160, s.height/2)
		sister2.position = cc.Point(320, s.height/2)
		
		session.scheduler.repeat(this, newBlend, 1.0)
	}
	
	function newBlend(dt)
	{
		var layer = getChildByTag(kTagLayer)
		
		var src = null
		var dst = null
		
		if (layer.blendFuncDst == 0)
		{
			src = cc.Sprite.SRC_BLEND_ONE
			dst = cc.Sprite.DST_BLEND_ONE_MINUS_SRC_ALPHA
		}
		else
		{
			src = cc.Sprite.SRC_BLEND_ONE_MINUS_DST_COLOR
			dst = cc.Sprite.DST_BLEND_ZERO
		}
		
		layer.blendFuncSrc = src
		layer.blendFuncDst = dst
		
	}
	
	function title()
	{	
		return "ColorLayer: blend";
	}
}
//------------------------------------------------------------------
//
// LayerGradient
//
//------------------------------------------------------------------
class LayerGradient : LayerTest
{
	constructor()
	{
		base.constructor()
			
		var size = cocos.director.winSize
		
		var layer1 = cc.LayerGradient(nit.Color(1, 0, 0, 1), nit.Color(0, 1, 0, 1), cc.Point(0.9, 0.9))
		addChild(layer1, 0, kTagLayer)
		
		var label1 = cc.LabelTTF("Compressed Interpolation: Enabled", "Marker Felt", 26)
		var label2 = cc.LabelTTF("Compressed Interpolation: Disabled", "Marker Felt", 26)
		var item1 = cc.MenuItemLabel(label1)
		var item2 = cc.MenuItemLabel(label2)
		var item = cc.MenuItemToggle( this, toggleItem, item1, item2)
		
		var menu = cc.Menu(item)
		addChild(menu)
		menu.position = cc.Point(size.width/2, 100)
		
		this.touchEnabled = true;
		this.channel().priority(-10000).bind(Events.OnCCTouchMoved, this, ccTouchesMoved)
	}
	
	function toggleItem(sender)
	{
		var gradient = getChildByTag(kTagLayer)
		gradient.compressedInterpolation = !gradient.compressedInterpolation
	}
	
	function ccTouchesMoved(evt: cc.TouchEvent)
	{
		var s = cocos.director.winSize
		
		var touch = evt.touches[0]
		var start = touch.locationInView(touch.view)
		start = cocos.director.toGl(start)
		
		var diff = cc.Point(s.width/2 - start.x, s.height/2 - start.y)
		diff = diff.Unit
		
		var gradient = getChildByTag(1)
		gradient.Vector =diff 
	}
	
	function title()
	{	
		return  "LayerGradient";
	}
	
	function subtitle()
	{
		return "Touch the screen and move your finger"
	}
}