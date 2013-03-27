var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var TAG =
{
	LAYER = 1
}

var sceneIdx 	= -1; 
var MAX_LAYER 	= 2

////////////////////////////////////////////////////////////////////////////////

var function createMotionLayer(nIndex)
{
	switch (nIndex)
	{
		case 0: return MotionStreakTest1();
		case 1: return MotionStreakTest2();
	}
}

var function nextMotionAction()
{
	sceneIdx++;
	sceneIdx = sceneIdx % MAX_LAYER;

	var pLayer = createMotionLayer(sceneIdx);
	
	return pLayer;
}

var function backMotionAction()
{
	sceneIdx--;
	var total = MAX_LAYER;
	if (sceneIdx < 0)
		sceneIdx += total;	
	
	var pLayer = createMotionLayer(sceneIdx);

	return pLayer;
}

var function restartMotionAction()
{
	var pLayer = createMotionLayer(sceneIdx);

	return pLayer;
} 

////////////////////////////////////////////////////////////////////////////////

class MotionStreakTest : cc.ScriptLayer
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		var x = size.width
		var y = size.height
	
		var label = cc.LabelTTF(title(), "Arial", 28)
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
		this.addChild(menu, 1);	
		
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
		var s = MotionStreakTestScene()
		s.addChild(restartMotionAction())
		cocos.director.replaceScene(s)
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = MotionStreakTestScene()
		s.addChild(nextMotionAction())
		cocos.director.replaceScene(s)
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		var s = MotionStreakTestScene()
		s.addChild(backMotionAction())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class MotionStreakTest1 : MotionStreakTest
{
	var _root = null
	var _target = null
	var _streak = null
	
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		var x = size.width
		var y = size.height
		
		// the root object just rotates around
		_root = cc.Sprite(s_pPathR1)
		addChild(_root, 1)
		_root.position = cc.Point(x/2, y/2)
		
		// the target object is offset from root, and the streak is moved to follow it
		_target = cc.Sprite(s_pPathR1)
		_root.addChild(_target)
		_target.position = cc.Point(100, 0)
		
		// create the streak object and add it to the scene
		_streak = cc.MotionStreak(2, 3, s_streak, 32, 32, nit.Color(0,1,0,1))
		addChild(_streak)
		
		// schedule an update on each frame so we can syncronize the streak with the target
		cocos.director.scheduler.repeat(this, onUpdate, 0.01)
		
		var a1 = cc.action.RotateBy(2, 360)
		var action1 = cc.action.RepeatForever(a1)
		var motion = cc.action.MoveBy(2, cc.Point(100, 0))
		_root.runAction(cc.action.RepeatForever(cc.action.Sequence(motion, motion.reverse())))
		_root.runAction(action1)
	}
	
	function onUpdate(delta)
	{
		_streak.position = _target.toWorld(cc.Point(0,0))
	}
	
	function title()
	{
		return "MotionStreak test 1";
	}
	
	function subtitle()
	{
		return "";
	}
}

////////////////////////////////////////////////////////////////////////////////

class MotionStreakTest2 : MotionStreakTest
{
	_root = null
	_target = null
	_streak = null
	
	constructor()
	{
		base.constructor()
			
		var s = cocos.director.winSize
		
		// create the streak object and add it to the scene
		_streak = cc.MotionStreak(3, 3, s_streak, 64, 32, nit.Color(1,1,1,1));
		addChild(_streak);
		
		_streak.position = cc.Point(s.width/2, s.height/2)
		
		this.touchEnabled = true;
		this.channel().bind(Events.OnCCTouchMoved, this, ccTouchesMoved)
	}
	
	function ccTouchesMoved(evt: cc.TouchEvent)
	{
		var touchLocation = evt.touch0.locationInView(evt.touch0.view)
		touchLocation = cocos.director.toGl(touchLocation)
		_streak.position = touchLocation
	}
	
	function title()
	{	
		return "MotionStreak test 2";
	}
}

////////////////////////////////////////////////////////////////////////////////

class MotionStreakTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = nextMotionAction()
		addChild(pLayer)
		cocos.director.replaceScene(this)
	}
}

return MotionStreakTestScene()