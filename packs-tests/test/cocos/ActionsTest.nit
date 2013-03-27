var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

class ActionsTest
{
	var _layerIndex = 0
	
	var _layers =
	[
		ActionManual,
		ActionMove,
		ActionScale,
		ActionRotate,
		ActionSkew,
		ActionSkewRotateScale,
		ActionJump,
		ActionBezier,
		ActionBlink,
		ActionFade,
		ActionTint,
		ActionAnimate,
		ActionSequence,
		ActionSequence2,
		ActionSpawn,
		ActionReverse,
		ActionDelayTime,
		ActionRepeat,
		ActionRepeatForever,
		ActionRotateToRepeat,
		ActionRotateJerk,
		ActionCallFunc,
		ActionCallFuncND,
		ActionReverseSequence,
		ActionReverseSequence2,
		ActionOrbit,
		ActionFollow,
	]
	
	function next()
	{
		_layerIndex = (_layerIndex + 1) % _layers.len()
		
		return _layers[_layerIndex] ()
	}
	
	function back()
	{
		_layerIndex--
		if (_layerIndex < 0)
			_layerIndex = _layers.len() - 1
			
		return _layers[_layerIndex] ()
	}
	
	function restart()
	{
		return _layers[_layerIndex] ()
	}
}

var theTest

////////////////////////////////////////////////////////////////////////////////

class ActionsDemo : cc.ScriptLayer
{
	var _grossini 	= null
    var _tamara 	= null
    var _kathia		= null

	constructor()
	{
		base.constructor()
		
		printf("ActionsTest - (%s)", subtitle())
		
		_grossini = cc.Sprite(pack.locate("grossini.png", "*Images*"))
		
		_tamara = cc.Sprite(pack.locate("grossinis_sister1.png"))
		
		_kathia = cc.Sprite(pack.locate("grossinis_sister2.png"))

		addChild(_grossini, 1)
		addChild(_tamara, 2)
		addChild(_kathia, 3)

		 var s = cocos.director.winSize;

		_grossini.position = cc.Point(s.width/2, s.height/3)
		_tamara.position = cc.Point(s.width/2, 2*s.height/3)
		_kathia.position = cc.Point(s.width/2, s.height/2)

		var label  = cc.LabelTTF(title(), "Arial", 18)
		addChild(label, 1)
		label.position = cc.Point(s.width/2, s.height - 30)

		if (subtitle() != "") 
		{
			var l = cc.LabelTTF(subtitle(), "Arial", 22)
			addChild(l, 1)
			l.position = cc.Point(s.width/2, s.height - 60)
		}	

		var item1 = cc.MenuItemImage(pack.locate("b1.png"), pack.locate("b2.png"), this, onBack)
		var item2 = cc.MenuItemImage(pack.locate("r1.png"), pack.locate("r2.png"), this, onRestart)
		var item3 = cc.MenuItemImage(pack.locate("f1.png"), pack.locate("f2.png"), this, onNext)

		var menu = cc.Menu(item1, item2, item3)

		menu.position = cc.Point.ZERO
		item1.position = cc.Point(s.width/2 - 100,30)
		item2.position = cc.Point(s.width/2, 30)
		item3.position = cc.Point(s.width/2 + 100,30)

		addChild(menu, 1)
	}

    function centerSprites(numberOfSprites)
	{
		var s = cocos.director.winSize;

		if (numberOfSprites == 1) 
		{
			_tamara.visible = false
			_kathia.visible = false
			_grossini.position = cc.Point(s.width/2, s.height/2)
		}
		else if (numberOfSprites == 2) 
		{		
			_kathia.position = cc.Point(s.width/3, s.height/2)
			_tamara.position = cc.Point(2*s.width/3, s.height/2)
			_grossini.visible = false
		} 
		else if (numberOfSprites == 3) 
		{
			_grossini.position = cc.Point(s.width/2, s.height/2)
			_tamara.position = cc.Point(s.width/4, s.height/2)
			_kathia.position = cc.Point(3 * s.width/4, s.height/2)
		}
	}
    
	function alignSpritesLeft(numberOfSprites)
	{
		 var s = cocos.director.winSize;

		if (numberOfSprites == 1) 
		{
			_tamara.visible = false
			_kathia.visible = false
			_grossini.position = cc.Point(60, s.height/2)
		} 
		else if (numberOfSprites == 2) 
		{		
			_kathia.position = cc.Point(60, s.height/3)
			_tamara.position = cc.Point(60, 2*s.height/3)
			_grossini.visible = false 
		} 
		else if (numberOfSprites == 3) 
		{
			_grossini.position = cc.Point(60, s.height/2)
			_tamara.position = cc.Point(60, 2*s.height/3)
			_kathia.position = cc.Point(60, s.height/3)
		}
	}
	
     function title()
	 {
		return "ActionsTest"
	 }
	
    function subtitle()
	{
		return ""
	}

    function onRestart(evt: cc.MenuItemEvent)
	{
		var s = ActionTestScene()
		s.addChild(theTest.restart())
		cocos.director.replaceScene(s)
	}
	
    function onNext(evt: cc.MenuItemEvent)
	{
		var s = ActionTestScene()
		s.addChild(theTest.next())
		cocos.director.replaceScene(s)
	}
	
    function onBack(evt: cc.MenuItemEvent)
	{
		var s = ActionTestScene()
		s.addChild(theTest.back())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionMove : ActionsDemo
{
	constructor()
	{
		base.constructor()
	
		centerSprites(3);

		var s = cocos.director.winSize

		 var actionTo = cc.action.MoveTo(2, cc.Point(s.width-40, s.height-40))
		 var actionBy = cc.action.MoveBy(2, cc.Point(80,80))
		 var actionByBack = actionBy.reverse()

		_tamara.runAction(actionTo)
		_grossini.runAction(cc.action.Sequence(actionBy, actionByBack))
		_kathia.runAction(cc.action.MoveTo(1, cc.Point(40,40)))
	}
	
	function subtitle()
	{
		return "MoveTo / MoveBy"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionManual : ActionsDemo
{
	constructor()
	{
		base.constructor()
	
		var s = cocos.director.winSize

		_tamara.scaleX = 2.5
		_tamara.scaleY = -1.0
		_tamara.position = cc.Point(100,70)
		_tamara.opacity = 128

		_grossini.rotation = 120
		_grossini.position = cc.Point(s.width/2, s.height/2)
		_grossini.color = nit.Color(1,0,0, 1)

		_kathia.position = cc.Point(s.width -100 , s.height/2)
		_kathia.color = nit.Color.BLUE
	}
	
	function subtitle()
	{
		return "Manual Transformation"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionScale : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(3);

		var actionTo = cc.action.ScaleTo(2, 0.5)
		var actionBy = cc.action.ScaleBy(2 ,  2)
		var actionBy2 = cc.action.ScaleBy(2, 0.25, 4.5)
		var actionByBack = actionBy.reverse()

		_tamara.runAction(actionTo);
		_grossini.runAction(cc.action.Sequence(actionBy, actionByBack))
		_kathia.runAction(cc.action.Sequence(actionBy2, actionBy2.reverse()))
	}
	
	function subtitle()
	{
		return "ScaleTo / ScaleBy"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionRotate : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(3)

		var actionTo = cc.action.RotateTo(2, 45)
		var actionTo2 = cc.action.RotateTo(2, -45)
		
		var actionTo0 = cc.action.RotateTo(2 , 0)
		var actionTo1 = cc.action.RotateTo(2 , 0)
		_tamara.runAction(cc.action.Sequence(actionTo, actionTo0))

		var actionBy = cc.action.RotateBy(2 ,  360);
		var actionByBack = actionBy.reverse()
		_grossini.runAction(cc.action.Sequence(actionBy, actionByBack))

		_kathia.runAction(cc.action.Sequence(actionTo2, actionTo1))
	}
	
	function subtitle()
	{
		return "RotateTo / RotateBy"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionSkew : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(3)

		var actionTo = cc.action.SkewTo(2, 37.2, -37.2)
		var actionToBack = cc.action.SkewTo(2, 0, 0)
		var actionBy = cc.action.SkewBy(2, 0, -90)
		var actionBy2 = cc.action.SkewBy(2, 45, 45)
		var actionByBack = actionBy.reverse()

		_tamara.runAction(cc.action.Sequence(actionTo, actionToBack))
		_grossini.runAction(cc.action.Sequence(actionBy, actionByBack))
		_kathia.runAction(cc.action.Sequence(actionBy2, actionBy2.reverse()))
	}
	
	function subtitle()
	{
		return "SkewTo / SkewBy"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionSkewRotateScale : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		_tamara.removeFromParent(true)
		_grossini.removeFromParent(true)
		_kathia.removeFromParent(true)

		var boxSize = cc.Size(100.0, 100.0)

		var box = cc.LayerColor(nit.Color(1, 1, 0, 1))
		box.anchorPoint = cc.Point(0, 0)
		box.position = cc.Point(190, 110)
		box.contentSize = boxSize

		var markrside = 10.0
		var uL = cc.LayerColor(nit.Color(1, 0, 0, 1))
		box.addChild(uL)
		uL.contentSize = cc.Size(markrside, markrside)
		uL.position = cc.Point(0, boxSize.height - markrside)
		uL.anchorPoint = cc.Point(0, 0)

		var uR = cc.LayerColor(nit.Color(0, 0, 1, 1))
		box.addChild(uR);
		uR.contentSize = cc.Size(markrside, markrside)
		uR.position = cc.Point(boxSize.width - markrside, boxSize.height - markrside)
		uR.anchorPoint = cc.Point(0, 0)
		addChild(box)

		var actionTo = cc.action.SkewTo(2, 0, 2)
		var rotateTo = cc.action.RotateTo(2, 61)
		var actionScaleTo = cc.action.ScaleTo(2, -0.44, 0.47)

		var actionScaleToBack = cc.action.ScaleTo(2, 1.0, 1.0)
		var rotateToBack = cc.action.RotateTo(2, 0)
		var actionToBack = cc.action.SkewTo(2, 0, 0)

		box.runAction(cc.action.Sequence(actionTo, actionToBack))
		box.runAction(cc.action.Sequence(rotateTo, rotateToBack))
		box.runAction(cc.action.Sequence(actionScaleTo, actionScaleToBack))
	}
	
	function subtitle()
	{
		return "Skew + Rotate + Scale"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionJump : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(3)
		
		var actionTo = cc.action.JumpTo(2, cc.Point(300,300), 50, 4)
		var actionBy = cc.action.JumpBy(2, cc.Point(300,0), 50, 4)
		var actionUp = cc.action.JumpBy(2, cc.Point(0,0), 80, 4)
		var actionByBack = actionBy.reverse()

		_tamara.runAction(actionTo)
		_grossini.runAction(cc.action.Sequence(actionBy, actionByBack))
		_kathia.runAction(cc.action.RepeatForever(actionUp))
	}

	function subtitle()
	{
		return "JumpTo / JumpBy"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionBezier : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(3)
	
		var s = cocos.director.winSize
		
		var controlPoint_1 = cc.Point(0, s.height/2)
		var controlPoint_2 = cc.Point(300, -s.height/2)
		var endPosition = cc.Point(300,100)

		var bezierForward = cc.action.BezierBy(3, endPosition, controlPoint_1, controlPoint_2)
		var bezierBack = bezierForward.reverse()	
		var rep = cc.action.RepeatForever(cc.action.Sequence(bezierForward, bezierBack))

		//sprite 2
		_tamara.position = cc.Point(80,160)
		var controlPoint_1_1 = cc.Point(100, s.height/2)
		var controlPoint_1_2 = cc.Point(200, -s.height/2)
		var endPosition_1 = cc.Point(240,160)

		var bezierTo1 = cc.action.BezierTo(2, endPosition_1, controlPoint_1_1, controlPoint_1_2)

		//sprite 3
		_kathia.position = cc.Point(400,160)
		var bezierTo2 = cc.action.BezierTo(2, endPosition_1, controlPoint_1_1, controlPoint_1_2);

		_grossini.runAction(rep)
		_tamara.runAction(bezierTo1)
		_kathia.runAction(bezierTo2)
	}
	
	function subtitle()
	{
		return "BezierBy / BezierTo"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionBlink : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(2)

		var action1 = cc.action.Blink(2, 10)
		var action2 = cc.action.Blink(2, 5)

		_tamara.runAction(action1)
		_kathia.runAction(action2)
	}
	
	function subtitle()
	{
		return "Blink"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionFade : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(2)
		
		_tamara.opacity = 0 
		var action1 = cc.action.FadeIn(1.0)
		var action1Back = action1.reverse()

		var action2 = cc.action.FadeOut(1.0)
		var action2Back = action2.reverse()

		_tamara.runAction(cc.action.Sequence(action1, action1Back))
		_kathia.runAction(cc.action.Sequence(action2, action2Back))
	}
	
	function subtitle()
	{
		return "FadeIn / FadeOut"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionTint : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(2)
		
		var action1 = cc.action.TintTo(2, 255, 0, 255)
		var action2 = cc.action.TintBy(2, -127, -255, -127)
		var action2Back = action2.reverse();

		_tamara.runAction(action1)
		_kathia.runAction(cc.action.Sequence(action2, action2Back))
	}
	
	function subtitle()
	{
		return "TintTo / TintBy"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionAnimate : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(1)

		var animation = cc.Animation()
		var frameName = array(100, 0)
		for (var i=1; i <= 14; i++)
		{
			var framename = format("grossini_dance_%02d.png", i)
			
			animation.addFrameWithFileName(pack.locate(framename))
		}

		var action = cc.action.Animate(3, animation, false)
		var action_back = action.reverse()

		_grossini.runAction(cc.action.Sequence(action, action_back))
	}
	
	function subtitle()
	{
		return "Animation"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionSequence : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		alignSpritesLeft(1)

		var action = cc.action.Sequence(
			cc.action.MoveBy(2, cc.Point(240,0)),
			cc.action.RotateBy(2,  540) 
		)

		_grossini.runAction(action)
	}
	
	function subtitle()
	{
		return "Sequence: Move + Rotate"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionSequence2 : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		alignSpritesLeft(1)

		 _grossini.visible = false
		 
		var data = 0xbab0bab0

		var action = cc.action.Sequence(
			cc.action.Place(cc.Point(200,200)),
			cc.action.Show(),
			cc.action.MoveBy(1, cc.Point(100,0)),
			cc.action.EventCall(null, this, callback1),
			cc.action.EventCall(null, this, callback2),
			cc.action.EventCall(null, this, @callback3(data))
		)

		_grossini.runAction(action)
	}
	
	function callback1()
	{
		var s = cocos.director.winSize
		var label = cc.LabelTTF("callback 1 called", "Arial", 16);
		label.position = cc.Point(s.width/4*1,s.height/2)

		addChild(label)
	}

	function callback2()
	{
		var s = cocos.director.winSize
		var label = cc.LabelTTF("callback 2 called", "Arial", 16);
		label.position = cc.Point(s.width/4*2,s.height/2)

		addChild(label)
	}

	function callback3(data)
	{
		print(data)
		var s = cocos.director.winSize
		var label = cc.LabelTTF("callback 3 called", "Arial", 16)
		label.position = cc.Point(s.width/4*3,s.height/2)

		addChild(label)
	}
	
	function subtitle()
	{
		return "Sequence of InstantActions"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionSpawn : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		alignSpritesLeft(1)
		
		var action = cc.action.Spawn(
			cc.action.JumpBy(2, cc.Point(300,0), 50, 4),
			cc.action.RotateBy(2,  720)
		)

		_grossini.runAction(action)
	}
	
	function subtitle()
	{
		return "Spawn: Jump + Rotate"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionReverse : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		alignSpritesLeft(1)
		
		var jump = cc.action.JumpBy(2, cc.Point(300,0), 50, 4)
		var action = cc.action.Sequence(jump, jump.reverse())

		_grossini.runAction(action)
	}
	
	function subtitle()
	{
		return "Reverse an action"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionDelayTime : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		alignSpritesLeft(1)
		
		var move = cc.action.MoveBy(1, cc.Point(150,0))
		var action = cc.action.Sequence(move, cc.action.DelayTime(2), move)

		_grossini.runAction(action)
	}
	
	function subtitle()
	{
		return "DelayTime: m + delay + m"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionRepeat : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		alignSpritesLeft(2)
		
		var a1 = cc.action.MoveBy(1, cc.Point(150,0))
		var a2 = cc.action.MoveBy(1, cc.Point(150,0))
		
		var action1 = cc.action.Repeat(cc.action.Sequence(cc.action.Place(cc.Point(60,60)), a1) ,  3) 
		var action2 = cc.action.RepeatForever(cc.action.Sequence(a2,   a2.reverse()))

		_kathia.runAction(action1)
		_tamara.runAction(action2)
	}
	
	function subtitle()
	{
		return "Repeat / RepeatForever actions"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionRepeatForever : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(1)
		
		var action = cc.action.Sequence(cc.action.DelayTime(1),
        cc.action.EventCall(null, this, repeatForever))

		_grossini.runAction(action)
	}
	
	function repeatForever()
	{
		var repeat = cc.action.RepeatForever(cc.action.RotateBy(1, 360))

		_grossini.runAction(repeat)
	}
	
	function subtitle()
	{
		return "CallFuncN + RepeatForever"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionRotateToRepeat : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(2)
		var act1 = cc.action.RotateTo(1, 90)
		var act2 = cc.action.RotateTo(1, 0)
		
		var act1_1 = cc.action.RotateTo(1, 90)
		var act1_2 = cc.action.RotateTo(1, 0)
		
		var seq = cc.action.Sequence(act1, act2)
		var seq2 = cc.action.Sequence(act1_1, act1_2)
		var rep1 = cc.action.RepeatForever(seq)
		var rep2 = cc.action.Repeat(seq2, 10);

		_tamara.runAction(rep1)
		_kathia.runAction(rep2)
	}
	
	function subtitle()
	{
		return "Repeat/RepeatForever + RotateTo"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionRotateJerk : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(2)
		
		var seq = cc.action.Sequence(cc.action.RotateTo(0.5, -20), cc.action.RotateTo(0.5, 20))
		var seq2 = cc.action.Sequence(cc.action.RotateTo(0.5, -20), cc.action.RotateTo(0.5, 20))

		var rep1 = cc.action.Repeat(seq, 10)
		var rep2 = cc.action.RepeatForever(seq2)

		_tamara.runAction(rep1)
		_kathia.runAction(rep2)
	}
	
	function subtitle()
	{
		return "RepeatForever / Repeat + Rotate"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionCallFunc : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(3)

		var action = cc.action.Sequence(
        cc.action.MoveBy(2, cc.Point(200,0)),
        cc.action.EventCall(null, this, callback1))

		var action2 = cc.action.Sequence(
        cc.action.ScaleBy(2 ,  2),
        cc.action.FadeOut(2),
        cc.action.EventCall(null, this, callback2))

		var data = 0xbebabeba
		
		var action3 = cc.action.Sequence(
			cc.action.RotateBy(3 , 360),
			cc.action.FadeOut(2),
			cc.action.EventCall(null, this, @callback3(data))
		)

		_grossini.runAction(action)
		_tamara.runAction(action2)
		_kathia.runAction(action3)
	}
	
	function callback1()
	{
		var s = cocos.director.winSize
		var label = cc.LabelTTF("callback 1 called", "Arial", 16)
		label.position = cc.Point(s.width/4*1,s.height/2)

		addChild(label)
	}

	function callback2()
	{
		var s = cocos.director.winSize
		var label = cc.LabelTTF("callback 2 called", "Arial", 16)
		label.position = cc.Point(s.width/4*2,s.height/2)

		addChild(label)
	}

	function callback3(data)
	{
		var s = cocos.director.winSize
		var label = cc.LabelTTF("callback 3 called", "Arial", 16);
		label.position = cc.Point(s.width/4*3,s.height/2)
		addChild(label)
	}
	
	function subtitle()
	{
		return "Callbacks: CallFunc and friends"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionCallFuncND : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(1)
		
		var action = cc.action.Sequence(cc.action.MoveBy(2, cc.Point(200,0)),  cc.action.EventCall(null, this, @callback1(false)))
		_grossini.runAction(action)
	}
	
	function callback1(data)
	{
		_grossini.removeFromParent(true)
	}
	
	function title()
	{
		return "CallFuncND + auto remove";
	}
	
	function subtitle()
	{
		return "CallFuncND + removeFromParentAndCleanup\n. Grossini dissapears in 2s";
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionReverseSequence : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		alignSpritesLeft(1)
		
		var move1 = cc.action.MoveBy(1, cc.Point(250,0))
		var move2 = cc.action.MoveBy(1, cc.Point(0,50));
		var seq = cc.action.Sequence(move1, move2, move1.reverse())
		var action = cc.action.Sequence(seq, seq.reverse())

		_grossini.runAction(action)
	}
	
	function subtitle()
	{
		return "Reverse a sequence"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionReverseSequence2 : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		alignSpritesLeft(2)
		
		var move1 = cc.action.MoveBy(1, cc.Point(250,0))
		var move2 = cc.action.MoveBy(1, cc.Point(0,50))
		var tog1 = cc.action.ToggleVisibility();
		var tog2 = cc.action.ToggleVisibility();
		
		var seq = cc.action.Sequence(move1, tog1, move2, tog2, move1.reverse())
		var action = cc.action.Repeat(cc.action.Sequence(seq, seq.reverse()), 3)

		 _kathia.runAction(action);

		var move_tamara = cc.action.MoveBy(1, cc.Point(100,0))
		var move_tamara2 = cc.action.MoveBy(1, cc.Point(50,0))
		var hide = cc.action.Hide();
		var seq_tamara = cc.action.Sequence(move_tamara, hide, move_tamara2)
		var seq_back = seq_tamara.reverse()
		_tamara.runAction(cc.action.Sequence(seq_tamara, seq_back))
	}
	
	function subtitle()
	{
		return "Reverse sequence 2"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionOrbit : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		 centerSprites(3);

		var orbit1 = cc.action.OrbitCamera(2,1, 0, 0, 180, 0, 0)
		var action1 = cc.action.Sequence(orbit1, orbit1.reverse())

		var orbit2 = cc.action.OrbitCamera(2,1, 0, 0, 180, -45, 0)
		var action2 = cc.action.Sequence(orbit2, orbit2.reverse())

		var orbit3 = cc.action.OrbitCamera(2,1, 0, 0, 180, 90, 0)
		var action3 = cc.action.Sequence(orbit3, orbit3.reverse())

		_kathia.runAction(cc.action.RepeatForever(action1))
		_tamara.runAction(cc.action.RepeatForever(action2))
		_grossini.runAction(cc.action.RepeatForever(action3))

		var move = cc.action.MoveBy(3, cc.Point(100,-100))
		var move_back = move.reverse()
		var seq = cc.action.Sequence(move, move_back)
		
		var move2 = cc.action.MoveBy(3, cc.Point(100,-100))
		var move_back2 = move.reverse()
		var seq2 = cc.action.Sequence(move2, move_back2)
		
		var move3 = cc.action.MoveBy(3, cc.Point(100,-100))
		var move_back3 = move.reverse()
		var seq3 = cc.action.Sequence(move3, move_back3)
		
		var rfe = cc.action.RepeatForever(seq)
		var rfe2 = cc.action.RepeatForever(seq2)
		var rfe3 = cc.action.RepeatForever(seq3)
		
		_kathia.runAction(rfe)
		_tamara.runAction(rfe2)
		_grossini.runAction(rfe3)
	}
	
	function subtitle()
	{
		return "OrbitCamera action"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionFollow : ActionsDemo
{
	constructor()
	{
		base.constructor()
		
		centerSprites(1);
		var s = cocos.director.winSize

		_grossini.position = cc.Point(-200, s.height / 2)
		var move  = cc.action.MoveBy(2, cc.Point(s.width * 3, 0));
		var move_back = move.reverse()
		var seq = cc.action.Sequence(move, move_back)
		var rep = cc.action.RepeatForever(seq);

		_grossini.runAction(rep)

		runAction(cc.action.Follow(_grossini, cc.Rect(0, 0, s.width * 2 - 100, s.height)))
	}
	
	function subtitle()
	{
		return "Follow action"
	}
}

////////////////////////////////////////////////////////////////////////////////

class ActionTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		addChild(theTest.restart())
		cocos.director.replaceScene(this)
	}
}

theTest = ActionsTest()

return ActionTestScene()