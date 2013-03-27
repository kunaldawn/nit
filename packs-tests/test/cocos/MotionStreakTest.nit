var pack = script.locator

kTagLayer := 1

sceneIdx 	:= -1; 

MAX_LAYER 	:= 2

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

function createMotionLayer(nIndex)
{
	switch(nIndex)
	{
		case 0: return MotionStreakTest1();
		case 1: return MotionStreakTest2();
	}
}

function nextMotionAction()
{
	sceneIdx++;
	sceneIdx = sceneIdx % MAX_LAYER;

	var pLayer = createMotionLayer(sceneIdx);
	
	return pLayer;
}

function backMotionAction()
{
	sceneIdx--;
	var total = MAX_LAYER;
	if( sceneIdx < 0 )
		sceneIdx += total;	
	
	var pLayer = createMotionLayer(sceneIdx);

	return pLayer;
}

function restartMotionAction()
{
	var pLayer = createMotionLayer(sceneIdx);

	return pLayer;
} 

class MotionStreakTest : cc.ScriptLayer
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
		var s = MotionStreakTestScene()
		s.addChild( restartMotionAction() )
		cocos.director.replaceScene(s)
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = MotionStreakTestScene()
		s.addChild( nextMotionAction() )
		cocos.director.replaceScene(s)
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		var s = MotionStreakTestScene()
		s.addChild( backMotionAction() )
		cocos.director.replaceScene(s)
	}
}

//------------------------------------------------------------------
//
// MotionStreakTest1
//
//------------------------------------------------------------------
class MotionStreakTest1 : MotionStreakTest
{
	m_root = null
	m_target = null
	m_streak = null
	
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		var x = size.width
		var y = size.height
		// the root object just rotates around
		m_root = cc.Sprite(s_pPathR1)
		addChild(m_root, 1)
		m_root.position = cc.Point(x/2, y/2)
		// the target object is offset from root, and the streak is moved to follow it
		m_target = cc.Sprite(s_pPathR1)
		m_root.addChild(m_target)
		m_target.position = cc.Point(100, 0)
		
		// create the streak object and add it to the scene
		m_streak = cc.MotionStreak(2, 3, s_streak, 32, 32, nit.Color(0,1,0,1))
		addChild(m_streak)
		// schedule an update on each frame so we can syncronize the streak with the target
		session.scheduler.repeat(this, onUpdate, 0.01)
		
		var a1 = cc.action.RotateBy(2, 360)
		var action1 = cc.action.RepeatForever(a1)
		var motion = cc.action.MoveBy(2, cc.Point(100, 0))
		m_root.runAction( cc.action.RepeatForever(cc.action.Sequence(motion, motion.reverse())))
		m_root.runAction(action1)
	}
	
	function onUpdate(delta)
	{
		m_streak.position = m_target.toWorld(cc.Point(0,0))
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
//------------------------------------------------------------------
//
// MotionStreakTest2
//
//------------------------------------------------------------------
class MotionStreakTest2 : MotionStreakTest
{
	m_root = null
	m_target = null
	m_streak = null
	
	constructor()
	{
		base.constructor()
			
		var s = cocos.director.winSize
		
		// create the streak object and add it to the scene
		m_streak = cc.MotionStreak(3, 3, s_streak, 64, 32, nit.Color(1,1,1,1) );
		addChild( m_streak );
		
		m_streak.position = cc.Point(s.width/2, s.height/2)
		
		this.touchEnabled = true;
		this.channel().priority(-10000).bind(Events.OnCCTouchMoved, this, ccTouchesMoved)
	}
	function ccTouchesMoved(evt: cc.TouchEvent)
	{
		var touchLocation = evt.touch0.locationInView(evt.touch0.view)
		touchLocation = cocos.director.toGl(touchLocation)
		m_streak.position = touchLocation
	}
	
	function title()
	{	
		return "MotionStreak test 2";
	}
}

return MotionStreakTestScene()