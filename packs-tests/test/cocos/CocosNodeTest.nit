var pack = script.locator

var TAG =
{
	SPRITE1 = 1
	SPRITE2 = 2
	SPRITE3 = 3
	SLIDER  = 4
}

////////////////////////////////////////////////////////////////////////////////

var sceneIdx = -1
var MAX_LAYER = 12

var function createCocosNodeLayer(idx)
{
	switch (idx)
	{
	case 0: return CameraCenterTest()
	case 1: return TestTest2()
	case 2: return TestTest4()
	case 3: return TestTest5()
	case 4: return TestTest6()
	case 5: return StressTest1()
	case 6: return StressTest2()
	case 7: return NodeToWorld()
	case 8: return SchedulerTest1()
	case 9: return CameraOrbitTest()
	case 10: return CameraZoomTest()
	case 11: return ConvertToNode()
	}
}

var function nextCocosNodeAction()
{
	sceneIdx++
	sceneIdx = sceneIdx % MAX_LAYER

	return createCocosNodeLayer(sceneIdx)
}

var function backCocosNodeAction()
{
	sceneIdx--
	var total = MAX_LAYER
	if (sceneIdx < 0)
		sceneIdx += total

	return createCocosNodeLayer(sceneIdx)	
}

var function restartCocosNodeAction()
{
	return createCocosNodeLayer(sceneIdx)
}

////////////////////////////////////////////////////////////////////////////////

class TestCocosNodeDemo : cc.ScriptLayer
{
	function title()		{ return ""	}
	function subtitle()		{ return ""	}
	
	function onEnter()		
	{
		var winSize = cocos.director.winSize	
		var label = cc.LabelTTF(title(), "Arial", 24)
		addChild(label, 1)
		label.position = cc.Point(winSize.width / 2, winSize.height - 50)
		
		if (subtitle().len() > 0)
		{
			var sublabel = cc.LabelTTF(subtitle(), "Arial", 16)
			addChild(sublabel, 1)
			sublabel.position = cc.Point(winSize.width / 2, winSize.height - 80)
		}
		
		var backBT = cc.MenuItemImage(s_pPathB1, s_pPathB2, this, backCallback)
		var resetBT = cc.MenuItemImage(s_pPathR1,s_pPathR2, this, restartCallback)
		var nextBT = cc.MenuItemImage(s_pPathF1, s_pPathF2, this, nextCallback)
		
		var testmenu = cc.Menu(backBT, resetBT, nextBT)
		testmenu.position = cc.Point(0, 0)
		backBT.position = cc.Point(winSize.width / 2 - 100, 30)
		resetBT.position = cc.Point(winSize.width / 2, 30)
		nextBT.position = cc.Point(winSize.width / 2 + 100, 30)
		
		addChild(testmenu, 1)
	}
	
	function onExit()
	{
	}
	
	function restartCallback()
	{
		var s = CocosNodeTestScene()
		s.addChild(restartCocosNodeAction())
		
		cocos.director.replaceScene(s)	
	}

	function nextCallback()
	{
		var s = CocosNodeTestScene()
		s.addChild(nextCocosNodeAction())
		
		cocos.director.replaceScene(s)
	}
	
	function backCallback()
	{
		var s = CocosNodeTestScene()
		s.addChild(backCocosNodeAction())
		
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class CameraCenterTest : TestCocosNodeDemo
{
	constructor()
	{
		base.constructor()
		
		var winSize = cocos.director.winSize
		var sprite = null
		var ocam = null
		
		// left top
		sprite = cc.Sprite()
		addChild(sprite, 0)
		sprite.color = nit.Color.RED
		sprite.position = cc.Point(winSize.width / 5 * 1, winSize.height / 5 * 1)
		sprite.textureRect = cc.Rect(0, 0, 120, 50)
		ocam = cc.action.OrbitCamera(10, 1, 0, 0, 360, 0, 0)
		sprite.runAction(cc.action.RepeatForever(ocam))

		// left bottom
		sprite = cc.Sprite()
		addChild(sprite, 0, 40)
		sprite.color = nit.Color.BLUE
		sprite.position = cc.Point(winSize.width / 5 * 1, winSize.height / 5 * 4)
		sprite.textureRect = cc.Rect(0, 0, 120, 50)
		ocam = cc.action.OrbitCamera(10, 1, 0, 0, 360, 0, 0)
		sprite.runAction(cc.action.RepeatForever(ocam))

		// right top
		sprite = cc.Sprite()
		addChild(sprite, 0)
		sprite.color = nit.Color(1, 1, 0, 1)
		sprite.position = cc.Point(winSize.width / 5 * 4, winSize.height / 5 * 1)
		sprite.textureRect = cc.Rect(0, 0, 120, 50)
		ocam = cc.action.OrbitCamera(10, 1, 0, 0, 360, 0, 0)
		sprite.runAction(cc.action.RepeatForever(ocam))

		// right bottom
		sprite = cc.Sprite()
		addChild(sprite, 0, 40)
		sprite.color = nit.Color.GREEN
		sprite.position = cc.Point(winSize.width / 5 * 4, winSize.height / 5 * 4)
		sprite.textureRect = cc.Rect(0, 0, 120, 50)
		ocam = cc.action.OrbitCamera(10, 1, 0, 0, 360, 0, 0)
		sprite.runAction(cc.action.RepeatForever(ocam))
		
		// center
		sprite = cc.Sprite()
		addChild(sprite, 0, 40)
		sprite.color = nit.Color.WHITE
		sprite.position = cc.Point(winSize.width / 2, winSize.height / 2)
		sprite.textureRect = cc.Rect(0, 0, 120, 50)
		ocam = cc.action.OrbitCamera(10, 1, 0, 0, 360, 0, 0)
		sprite.runAction(cc.action.RepeatForever(ocam))		
	}
	
	function title()		{ return "Camera Center test"	}
	function subtitle()		{ return "Sprites should rotate at the same speed"	}	
}

////////////////////////////////////////////////////////////////////////////////

class TestTest2 : TestCocosNodeDemo
{
	function onEnter()
	{
		base.onEnter()
		
		var s = cocos.director.winSize
		var sp1 = cc.Sprite(s_pPathSister1)
		var sp2 = cc.Sprite(s_pPathSister2)
		var sp3 = cc.Sprite(s_pPathSister1)
		var sp4 = cc.Sprite(s_pPathSister2)
		
		sp1.position = cc.Point(100, s.height / 2)
		sp2.position = cc.Point(300, s.height / 2)
		
		addChild(sp1)
		addChild(sp2)
		
		sp3.scale(0.25)
		sp4.scale(0.25)
		
		sp1.addChild(sp3)
		sp2.addChild(sp4)
		
		var a1 = cc.action.RotateBy(2, 360)
		var a2 = cc.action.ScaleBy(2, 2)
		var a3 = cc.action.RotateBy(2, 360)
		var a4 = cc.action.ScaleBy(2, 2)
		
		var action1 = cc.action.RepeatForever(cc.action.Sequence(a1, a2, a2.reverse()))
		var action2 = cc.action.RepeatForever(cc.action.Sequence(a3, a4, a4.reverse()))
		
		sp2.anchorPoint = cc.Point(0, 0)
		
		sp1.runAction(action1)
		sp2.runAction(action2)
	}
	
	function title()		{ return "anchorPoint and children"	}
}

////////////////////////////////////////////////////////////////////////////////

class TestTest4 : TestCocosNodeDemo
{
	_scheduler = null
	
	constructor()
	{
		base.constructor()
		
		var sp1 = cc.Sprite(s_pPathSister1)
		var sp2 = cc.Sprite(s_pPathSister2)		
		
		sp1.position = cc.Point(100, 160)
		sp2.position = cc.Point(380, 160)	

		addChild(sp1, 0, 2)
		addChild(sp2, 0, 3)	

		cocos.director.scheduler.repeat(this, delay2, 2)
		_scheduler = cocos.director.scheduler.once(this, delay4, 4)
	}
	
	function delay2(evt)
	{
		print("++ delay2")

		var node = getChildByTag(2)
		var action1 = cc.action.RotateBy(1, 360)
		node.runAction(action1)
	}
	
	function delay4(evt)
	{
		print("++ delay4")

		cocos.director.scheduler.unbind(_scheduler)
		removeChildByTag(3, false)
	}
	
	function title()		{ return "tags"	}
}

////////////////////////////////////////////////////////////////////////////////

class TestTest5 : TestCocosNodeDemo
{
	_scheduler = null
	
	constructor()
	{
		base.constructor()
		
		var sp1 = cc.Sprite(s_pPathSister1)
		var sp2 = cc.Sprite(s_pPathSister2)		
		
		sp1.position = cc.Point(100, 160)
		sp2.position = cc.Point(380, 160)	

		var rot = cc.action.RotateBy(2, 360)
		var rot_back = rot.reverse()
		var rot1 = cc.action.RotateBy(2, 360)
		var rot_back1 = rot1.reverse()
		var forever1 = cc.action.RepeatForever(cc.action.Sequence(rot, rot_back))
		var forever2 = cc.action.RepeatForever(cc.action.Sequence(rot1, rot_back1))
		forever1.tag = 101
		forever2.tag = 102
		
		addChild(sp1, 0, 1)
		addChild(sp2, 0, 2)	
		
		sp1.runAction(forever1)
		sp2.runAction(forever2)

		cocos.director.scheduler.once(this, addAndRemove, 2)
	}
	
	function addAndRemove()
	{
		print("++ add and remove")

		var sp1 = getChildByTag(1)
		var sp2 = getChildByTag(2)
		
		removeChild(sp1, false)
		removeChild(sp2, true)
		
		addChild(sp1, 0, 1)
		addChild(sp2, 0, 2)
	}

	function title()		{ return "remove and cleanup" }
}

////////////////////////////////////////////////////////////////////////////////

class TestTest6 : TestCocosNodeDemo
{
	_scheduler = null
	
	constructor()
	{
		base.constructor()
		
		var sp1 = cc.Sprite(s_pPathSister1)
		var sp11 = cc.Sprite(s_pPathSister1)
		
		var sp2 = cc.Sprite(s_pPathSister2)		
		var sp21 = cc.Sprite(s_pPathSister2)		
		
		sp1.position = cc.Point(100, 160)
		sp2.position = cc.Point(380, 160)	

		var rot1 = cc.action.RotateBy(2, 360)
		var rot2 = cc.action.RotateBy(2, 360)
		var rot3 = cc.action.RotateBy(2, 360)
		var rot4 = cc.action.RotateBy(2, 360)

		var forever1 = cc.action.RepeatForever(cc.action.Sequence(rot1, rot1.reverse()))
		var forever11 = cc.action.RepeatForever(cc.action.Sequence(rot2, rot2.reverse()))
		var forever2 = cc.action.RepeatForever(cc.action.Sequence(rot3, rot3.reverse()))
		var forever21 = cc.action.RepeatForever(cc.action.Sequence(rot4, rot4.reverse()))
		
		addChild(sp1, 0, 1)
		sp1.addChild(sp11)

		addChild(sp2, 0, 2)
		sp2.addChild(sp21)
		
		sp1.runAction(forever1)
		sp11.runAction(forever11)
		sp2.runAction(forever2)
		sp21.runAction(forever21)
		
		cocos.director.scheduler.once(this, addAndRemove, 2)
	}
	
	function addAndRemove()
	{
		print("++ add and remove")

		var sp1 = getChildByTag(1)
		var sp2 = getChildByTag(2)
		
		removeChild(sp1, false)
		removeChild(sp2, true)
		
		addChild(sp1, 0, 1)
		addChild(sp2, 0, 2)
	}

	function title()		{ return "remove/cleanup with children"	}
}

////////////////////////////////////////////////////////////////////////////////

class StressTest1 : TestCocosNodeDemo
{
	_scheduler = null
	
	constructor()
	{
		base.constructor()
	
		var s = cocos.director.winSize
		var sp1 = cc.Sprite(s_pPathSister1)
		addChild(sp1, 0, TAG.SPRITE1)
		
		sp1.position = cc.Point(s.width/2, s.height/2)
		
		cocos.director.scheduler.once(this, shouldNotCrash, 1)
	}
	
	function shouldNotCrash()
	{
		var s = cocos.director.winSize;
		var explosion = cc.particle.Sun()
		explosion.texture = cocos.textureCache.addImage(pack.locate("fire.png"));
		explosion.position = cc.Point(s.width/2, s.height/2)
		
		anchorPoint = cc.Point(0,0)
		runAction(cc.action.Sequence(cc.action.RotateBy(2, 360), 
			cc.action.EventCall(null, this, @removeMe(this))))
		
		addChild(explosion)
	}
	
	function removeMe(node)
	{
		parent.removeChild(node, true)
		nextCallback()
	}

	function title()		{ return "stress test #1: no crashes";}
}

////////////////////////////////////////////////////////////////////////////////

class StressTest2 : TestCocosNodeDemo
{
	_schedule = null
	
	constructor()
	{
		base.constructor()
	
		var s = cocos.director.winSize	
		var sublayer = cc.ScriptLayer()

		var sp1 = cc.Sprite(s_pPathSister1)
		sp1.position = cc.Point(80, s.height / 2)
		
		var move = cc.action.MoveBy(3, cc.Point(350, 0))
		var move_ease_inout3 = cc.action.EaseInOut(cc.action.MoveBy(3, cc.Point(350, 0)), 2.0)
		var move_ease_inout_back3 = move_ease_inout3.reverse()
		var seq3 = cc.action.Sequence(move_ease_inout3, move_ease_inout_back3)
		sp1.runAction(cc.action.RepeatForever(seq3))
		sublayer.addChild(sp1, 1)
		
		var fire = cc.particle.Fire()
		fire.texture = cocos.textureCache.addImage(pack.locate("fire.png"));
		fire.position = cc.Point(80, s.height/2 - 50) 
		fire.runAction(cc.action.RepeatForever(seq3.reverse()))
		sublayer.addChild(fire, 2)
		
		_schedule = cocos.director.scheduler.once(this, shouldNotLeak, 6)
		
		addChild(sublayer, 0, TAG.SPRITE1);
	}
	
	function shouldNotLeak(dt)
	{
		cocos.director.scheduler.unbind(_schedule)
		var sublayer = getChildByTag(TAG.SPRITE1)
		sublayer.removeAllChildren(true);
	}

	function title()		{ return "stress test #2: no leaks";	}
}

////////////////////////////////////////////////////////////////////////////////

class SchedulerTest1 : TestCocosNodeDemo
{
	_scheduler = null
	
	constructor()
	{
		base.constructor()
		
		var layer = cc.Node()
		addChild(layer, 0)
		
		var handler = cocos.director.scheduler.once(this, doSomething, 0)
		
		cocos.director.scheduler.unbind(handler)
	}
	
	function doSomething(dt)
	{
		print("should not call")
		
	}
	
	function title()
	{
		return "cocosnode scheduler test #1"
	}
	
	function subtitle()
	{
		return "nothing should appear on console"
	}
}

////////////////////////////////////////////////////////////////////////////////

class NodeToWorld : TestCocosNodeDemo
{
	constructor()
	{
		base.constructor()
		
		var back = cc.Sprite(s_back3)
		addChild(back, -10)
		back.anchorPoint = cc.Point(0,0)
		var backSize = back.contentSize
		
		var item = cc.MenuItemImage(s_PlayNormal, s_PlaySelect)
		var menu = cc.Menu(item)
		menu.alignItemsVertically()
		menu.position = cc.Point(backSize.width/2, backSize.height/2)
		back.addChild(menu)
		
		var rot = cc.action.RotateBy(5, 360)
		var fe = cc.action.RepeatForever(rot)
		item.runAction(fe)
		
		var move = cc.action.MoveBy(3, cc.Point(200, 0))
		var move_back = move.reverse()
		var seq = cc.action.Sequence(move, move_back)
		var fe2 = cc.action.RepeatForever(seq)
		back.runAction(fe2)
	}

	function title()
	{
		return "nodeToParent transform"
	}
}

////////////////////////////////////////////////////////////////////////////////

class CameraOrbitTest : TestCocosNodeDemo
{

	function onEnter()
	{
		try base.onEnter()
		cocos.director.projection = cocos.director.PROJ_3D
	}
	
	function onExit()
	{
		cocos.director.projection = cocos.director.PROJ_2D
		try base.onExit()
	}
	
	constructor()
	{
		base.constructor()
		var winSize = cocos.director.winSize
		var sprite = null
		var cam = null
		var ss = null
		
		var p = cc.Sprite(s_back3)
		addChild(p, 0)
		p.position = cc.Point(winSize.width/2, winSize.height/2)
		p.opacity = 128
		
		// left 
		var s = p.contentSize
		sprite = cc.Sprite(pack.locate("grossini.png", "*Image*"))
		sprite.scale(0.5)
		p.addChild(sprite, 0)
		sprite.position = cc.Point(s.width/4, s.height/2)
		cam = sprite.camera
		var orbit1 = cc.action.OrbitCamera(2, 1, 0, 0, 360, 0, 0)
		sprite.runAction(cc.action.RepeatForever(orbit1))

		// center
		sprite = cc.Sprite(pack.locate("grossini.png", "*Image*"))
		sprite.scale(1.0)
		p.addChild(sprite, 0)
		sprite.position = cc.Point(s.width/4*2, s.height/2)
		var orbit2 = cc.action.OrbitCamera(2, 1, 0, 0, 360, 45, 0)
		sprite.runAction(cc.action.RepeatForever(orbit2))

		// right
		sprite = cc.Sprite(pack.locate("grossini.png", "*Image*"))
		sprite.scale(2.0)
		p.addChild(sprite, 0)
		sprite.position = cc.Point(s.width/4*3, s.height/2)
		ss = sprite.contentSize
		var orbit3 = cc.action.OrbitCamera(2, 1, 0, 0, 360, 90, -45)
		sprite.runAction(cc.action.RepeatForever(orbit3))
		
		// PARENT
		var orbit4 = cc.action.OrbitCamera(10, 1, 0, 0, 360, 0, 90)
		p.runAction(cc.action.RepeatForever(orbit4))
	}
	
	function title()		{ return  "Camera Orbit test";	}
}

////////////////////////////////////////////////////////////////////////////////

class CameraZoomTest : TestCocosNodeDemo
{
	m_z = null
	
	function onEnter()
	{
		try base.onEnter()
		cocos.director.projection = cocos.director.PROJ_3D
	}

	function onExit()
	{
		cocos.director.projection = cocos.director.PROJ_2D
		try base.onExit()
	}
	
	constructor()
	{
		base.constructor()
		var s = cocos.director.winSize
		
		var sprite = null
		var cam = null
		
		// left 
		sprite = cc.Sprite(pack.locate("grossini.png", "*Image*"))
		addChild(sprite, 0)
		sprite.position = cc.Point(s.width/4, s.height/2)
		cam = sprite.camera
		cam.eye = nit.Vector3(0, 0, 415)

		// center
		sprite = cc.Sprite(pack.locate("grossini.png", "*Image*"))
		addChild(sprite, 0, 40)
		sprite.position = cc.Point(s.width/4*2, s.height/2)

		// right
		sprite = cc.Sprite(pack.locate("grossini.png", "*Image*"))
		addChild(sprite, 0, 20)
		sprite.position = cc.Point(s.width/4*3, s.height/2)
		
		m_z = 0
		
		cocos.director.scheduler.repeat(this, update, 0.05)
	}
	
	function update(evt: TickEvent)
	{
		var sprite = null
		var cam = null
		
		m_z += evt.delta * 100
		
		sprite = getChildByTag(20)
		cam = sprite.camera
		cam.eye = nit.Vector3(0, 0, m_z)
		
		sprite = getChildByTag(40)
		cam = sprite.camera
		cam.eye = nit.Vector3(0, 0, m_z)
	}
	
	function title()		{ return  "Camera Orbit test";	}
}

////////////////////////////////////////////////////////////////////////////////

class ConvertToNode : TestCocosNodeDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var rotate = cc.action.RotateBy(10, -360)
		var action = cc.action.RepeatForever(rotate)

		for (var i=0; i<3; i++)
		{
			var sprite = cc.Sprite(pack.locate("grossini.png", "*Image*"))
			sprite.position = cc.Point(s.width/4 *(i+1), s.height/2)
			
			var point = cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 10, 100+i)
			
			switch (i)
			{
				case 0:
					sprite.anchorPoint=cc.Point(0,0)
					break;
				case 1:
					sprite.anchorPoint=cc.Point(0.5, 0.5)
					break;
				case 2:
					sprite.anchorPoint=cc.Point(1,1)
					break;
			}
			point.position = sprite.position
			var copy = action.reverse()
			sprite.runAction(copy)
			addChild(sprite, i)
		}
		
		touchEnabled = true
		channel().bind(Events.OnCCTouchEnded, this, onTouchEnded)
	}

	function onTouchEnded(evt: cc.TouchEvent)
	{
		foreach (touch in evt.touches)
		{
			var location = touch.locationInView(touch.view)
			location = cocos.director.toGl(location);
			
			for (var i = 0; i<3; i++)
			{
				var node = getChildByTag(100+i)
				var p1, p2 = null
				p1 = node.toNodeAR(location)
				p2 = node.toNode(location)
				
				print("AR: x="+p1.x+", y="+p1.y+" -- Not AR: x="+p2.x+", y="+p2.y)
			}
		}
	}

	function title()		{ return "Convert To Node Space";}
	function subtitle()		{ return "testing convertToNodeSPace / AR. Touch and see console"}
}

////////////////////////////////////////////////////////////////////////////////

class CocosNodeTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		addChild(nextCocosNodeAction())
		cocos.director.replaceScene(this)
	}
}

return CocosNodeTestScene()