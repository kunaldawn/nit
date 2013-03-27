var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

class EaseActionTest
{
	_sceneIndex = 0
	_sceneMax = 14
	
	constructor()
	{
	}
	
	function _createEaseActionTest(index)
	{
		var layer = null
		
		switch (index)
		{
		case 0: return SpriteEase()
		case 1: return SpriteEaseInOut()
		case 2: return SpriteEaseExponential()
		case 3: return SpriteEaseExponentialInOut()
		case 4: return SpriteEaseSine()
		case 5: return SpriteEaseSineInOut()
		case 6: return SpriteEaseElastic()
		case 7: return SpriteEaseElasticInOut()
		case 8: return SpriteEaseBounce()
		case 9: return SpriteEaseBounceInOut()
		case 10: return SpriteEaseBack()
		case 11: return SpriteEaseBackInOut()
		case 12: return SpeedTest()
		case 13: return SchedulerTest()
		}
	}
	
	function nextEaseActionTest()
	{
		_sceneIndex++
		_sceneIndex = _sceneIndex % _sceneMax
		
		return _createEaseActionTest(_sceneIndex)
	}
	function backEaseActionTest()
	{
		_sceneIndex--
		if (_sceneIndex < 0)
			_sceneIndex = _sceneMax -1
			
		return _createEaseActionTest(_sceneIndex)
	}
	function restartEaseActionTest()
	{
		return _createEaseActionTest(_sceneIndex)
	}
}

var eaTest = EaseActionTest()

////////////////////////////////////////////////////////////////////////////////

class EaseSpriteDemo : cc.ScriptLayer
{
	_grossini = null
	_tamara = null
	_kathia = null
	
	constructor()
	{
		base.constructor()
		
		_grossini = cc.Sprite(pack.locate("grossini.png", "*Images*"))
		_tamara = cc.Sprite(pack.locate("grossinis_sister1.png"))
		_kathia = cc.Sprite(pack.locate("grossinis_sister2.png"))
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		
		_grossini.position = cc.Point(60, 50);
		_kathia.position = cc.Point(60, 150);
		_tamara.position = cc.Point(60, 250);
		
		var rot = cc.action.RotateBy(16, -3600)
		
		addChild(_grossini, 3);
		addChild(_tamara, 2);
		addChild(_kathia, 1);
		
		
		var label = cc.LabelTTF(title(), "Arial", 32)
		var size = cocos.director.winSize
		label.position = cc.Point(x / 2, y - 50)
		addChild(label)
		
		var item1 = cc.MenuItemImage(s_pPathB1, s_pPathB2, this, onBack);
		var item2 = cc.MenuItemImage(s_pPathR1, s_pPathR2, this, onRestart);		
		var item3 = cc.MenuItemImage(s_pPathF1, s_pPathF2, this, onNext);
		
		var menu = cc.Menu(item1, item2, item3);
		menu.position = cc.Point(0, 0);
		item1.position = cc.Point(size.width/2 - 100, 30) ;
		item2.position = cc.Point(size.width/2, 30) ;
		item3.position = cc.Point(size.width/2 + 100, 30) ;
		addChild(menu, 1);	
	}
	
	function positionForTwo()
	{
		_grossini.position = cc.Point(60, 120)
		_tamara.position = cc.Point(60, 220)
		_kathia.visible = false;
	}
	
	function onBack(evt: cc.MenuItemEvent)
	{
		var s = CocosEaseActionTestScene()
		s.addChild(eaTest.backEaseActionTest())
		cocos.director.replaceScene(s)
	}
	
	function onRestart(evt: cc.MenuItemEvent)
	{
		var s = CocosEaseActionTestScene()
		s.addChild(eaTest.restartEaseActionTest())
		cocos.director.replaceScene(s)
	}
	
	function onNext(evt: cc.MenuItemEvent)
	{
		var s = CocosEaseActionTestScene()
		s.addChild(eaTest.nextEaseActionTest())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEase : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move3 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseIn(move1, 3.0);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseOut(move2, 3.0);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence(move3, move_back)
		var seq2 = cc.action.Sequence(move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence(move_ease_out, move_ease_out_back)
		
		var a2 = _grossini.runAction(cc.action.RepeatForever(seq1))
		a2.tag = 1;
		var a1 = _tamara.runAction(cc.action.RepeatForever(seq2))
		a1.tag = 1;
		var a = _kathia.runAction(cc.action.RepeatForever(seq3))
		a.tag = 1;
		
		cocos.director.scheduler.once(this, testStopAction, 6)
	}
	
	function title()
	{
		return "EaseIn - EaseOut - Stop"
	}
	
	function testStopAction()
	{
		_tamara.stopActionByTag(1);
		_kathia.stopActionByTag(1);
		_grossini.stopActionByTag(1);
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseInOut : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move3 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_ease_inout1 = cc.action.EaseInOut(move1, 2.0);
		var move_ease_inout_back1 = move_ease_inout1.reverse();
		var move_ease_inout2 = cc.action.EaseInOut(move2, 3.0);
		var move_ease_inout_back2 = move_ease_inout2.reverse();
		var move_ease_inout3 = cc.action.EaseInOut(move3, 4.0);
		var move_ease_inout_back3 = move_ease_inout3.reverse();
		
		var seq1 = cc.action.Sequence(move_ease_inout1, move_ease_inout_back1)
		var seq2 = cc.action.Sequence(move_ease_inout2, move_ease_inout_back2)
		var seq3 = cc.action.Sequence(move_ease_inout3, move_ease_inout_back3)
		
		var a2 = _tamara.runAction(cc.action.RepeatForever(seq1))
		a2.tag = 1;
		var a1 = _kathia.runAction(cc.action.RepeatForever(seq2))
		a1.tag = 1;
		var a = _grossini.runAction(cc.action.RepeatForever(seq3))
		a.tag = 1;
	}
	
	function title()
	{
		return "EaseInOut and rates";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseExponential : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move3 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseExponentialIn(move1, 3.0);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseExponentialOut(move2, 3.0);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence(move3, move_back)
		var seq2 = cc.action.Sequence(move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence(move_ease_out, move_ease_out_back)
		
		var a2 = _grossini.runAction(cc.action.RepeatForever(seq1))
		a2.tag = 1;
		var a1 = _tamara.runAction(cc.action.RepeatForever(seq2))
		a1.tag = 1;
		var a = _kathia.runAction(cc.action.RepeatForever(seq3))
		a.tag = 1;
	}
	
	function title()
	{
		return "ExpIn - ExpOut actions";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseExponentialInOut : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_back = move1.reverse()
		
		var move_ease = cc.action.EaseExponentialInOut(move1, 3.0);
		var move_ease_back = move_ease.reverse();
		
		var seq1 = cc.action.Sequence(move2, move_back)
		var seq2 = cc.action.Sequence(move_ease, move_ease_back)
		
		positionForTwo()
		
		var a2 = _grossini.runAction(cc.action.RepeatForever(seq1))
		var a1 = _tamara.runAction(cc.action.RepeatForever(seq2))
	}
	
	function title()
	{
		return "EaseExponentialInOut action";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseSine : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move3 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseSineIn(move1, 3.0);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseSineOut(move2, 3.0);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence(move3, move_back)
		var seq2 = cc.action.Sequence(move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence(move_ease_out, move_ease_out_back)
		
		var a2 = _grossini.runAction(cc.action.RepeatForever(seq1))
		a2.tag = 1;
		var a1 = _tamara.runAction(cc.action.RepeatForever(seq2))
		a1.tag = 1;
		var a = _kathia.runAction(cc.action.RepeatForever(seq3))
		a.tag = 1;
	}
	
	function title()
	{
		return "EaseSineIn - EaseSineOut";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseSineInOut : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move_back = move1.reverse()
		
		var move_ease = cc.action.EaseSineInOut(move2);
		var move_ease_back = move_ease.reverse();
		
		var seq1 = cc.action.Sequence(move1, move_back)
		var seq2 = cc.action.Sequence(move_ease, move_ease_back)
	
		positionForTwo();
		
		var a2 = _grossini.runAction(cc.action.RepeatForever(seq1))
		var a1 = _tamara.runAction(cc.action.RepeatForever(seq2))
	}
	
	function title()
	{
		return "EaseSineInOut action";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseElastic : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move3 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseElasticIn(move2);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseElasticOut(move3);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence(move1, move_back)
		var seq2 = cc.action.Sequence(move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence(move_ease_out, move_ease_out_back)
		
		_grossini.runAction(cc.action.RepeatForever(seq1))
		_tamara.runAction(cc.action.RepeatForever(seq2))
		_kathia.runAction(cc.action.RepeatForever(seq3))
	}
	
	function title()
	{
		return "Elastic In - Out actions";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseElasticInOut : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move3 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move4 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_back = move1.reverse()
		
		var move_ease_inout1 = cc.action.EaseElasticInOut(move2, 0.3);
		var move_ease_inout_back1 = move_ease_inout1.reverse();
		
		var move_ease_inout2 = cc.action.EaseElasticInOut(move3, 0.45);
		var move_ease_inout_back2 = move_ease_inout1.reverse();
		
		var move_ease_inout3 = cc.action.EaseElasticInOut(move4, 0.6);
		var move_ease_inout_back3 = move_ease_inout3.reverse();
		
		var seq1 = cc.action.Sequence(move_ease_inout1, move_ease_inout_back1)
		var seq2 = cc.action.Sequence(move_ease_inout2, move_ease_inout_back2)
		var seq3 = cc.action.Sequence(move_ease_inout3, move_ease_inout_back3)
		
		_tamara.runAction(cc.action.RepeatForever(seq1))
		_kathia.runAction(cc.action.RepeatForever(seq2))
		_grossini.runAction(cc.action.RepeatForever(seq3))
	}
	
	function title()
	{
		return "EaseElasticInOut action";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseBounce : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move3 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseBounceIn(move1, 3.0);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseBounceOut(move2, 3.0);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence(move3, move_back)
		var seq2 = cc.action.Sequence(move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence(move_ease_out, move_ease_out_back)
		
		_grossini.runAction(cc.action.RepeatForever(seq1))
		_tamara.runAction(cc.action.RepeatForever(seq2))
		_kathia.runAction(cc.action.RepeatForever(seq3))
	}
	
	function title()
	{
		return "Bounce In - Out actions";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseBounceInOut : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move3 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_back = move1.reverse()
		
		var move_ease = cc.action.EaseBounceInOut(move1, 3.0);
		var move_ease_back = move_ease.reverse();
		
		var seq1 = cc.action.Sequence(move3, move_back)
		var seq2 = cc.action.Sequence(move_ease, move_ease_back)
		
		positionForTwo();
		_grossini.runAction(cc.action.RepeatForever(seq1))
		_tamara.runAction(cc.action.RepeatForever(seq2))
	}
	
	function title()
	{
		return "EaseBounceInOut action";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseBack : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move3 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseBackIn(move1, 3.0);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseBackOut(move2, 3.0);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence(move3, move_back)
		var seq2 = cc.action.Sequence(move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence(move_ease_out, move_ease_out_back)
		
		_grossini.runAction(cc.action.RepeatForever(seq1))
		_tamara.runAction(cc.action.RepeatForever(seq2))
		_kathia.runAction(cc.action.RepeatForever(seq3))
	}
	
	function title()
	{
		return "Back In - Out actions";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteEaseBackInOut : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move2 = cc.action.MoveBy(3, cc.Point(350, 0))
		var move3 = cc.action.MoveBy(3, cc.Point(350, 0))
		
		var move_back = move1.reverse()
		
		var move_ease = cc.action.EaseBackInOut(move1, 3.0);
		var move_ease_back = move_ease.reverse();
		
		var seq1 = cc.action.Sequence(move3, move_back)
		var seq2 = cc.action.Sequence(move_ease, move_ease_back)
		
		positionForTwo()
		
		_grossini.runAction(cc.action.RepeatForever(seq1))
		_tamara.runAction(cc.action.RepeatForever(seq2))
	}
	
	function title()
	{
		return "EaseBackInOut action";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpeedTest : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var jump1 = cc.action.JumpBy(4, cc.Point(-400, 0), 100, 4);
		var jump2 = jump1.reverse();
		var rot1 = cc.action.RotateBy(4, 360*2);
		var rot2 = rot1.reverse();
		
		var seq3_1 = cc.action.Sequence(jump2, jump1)
		var seq3_2 = cc.action.Sequence(rot1, rot2)
		var spawn = cc.action.Spawn(seq3_1, seq3_2)
		var action = cc.action.Speed(cc.action.RepeatForever(spawn),  1)
		action.tag = 1;
		
		var jump11 = cc.action.JumpBy(4, cc.Point(-400, 0), 100, 4);
		var jump21 = jump11.reverse();
		var rot11 = cc.action.RotateBy(4, 360*2);
		var rot21 = rot11.reverse();
		
		var seq3_11 = cc.action.Sequence(jump21, jump11)
		var seq3_21 = cc.action.Sequence(rot11, rot21)
		var spawn1 = cc.action.Spawn(seq3_11, seq3_21)
		var action1 = cc.action.Speed(cc.action.RepeatForever(spawn1), 2)
		action1.tag = 1;
		
		var jump12 = cc.action.JumpBy(4, cc.Point(-400, 0), 100, 4);
		var jump22 = jump12.reverse();
		var rot12 = cc.action.RotateBy(4, 360*2);
		var rot22 = rot12.reverse();
		
		var seq3_12 = cc.action.Sequence(jump22, jump12)
		var seq3_22 = cc.action.Sequence(rot12, rot22)
		var spawn2 = cc.action.Spawn(seq3_12, seq3_22)
		var action2 = cc.action.Speed(cc.action.RepeatForever(spawn2), 3)
		action2.tag = 1;
		
		_grossini.runAction(action)
		_tamara.runAction(action1)
		_kathia.runAction(action2)
		
		cocos.director.scheduler.repeat(this, altertime, 1)
	}

	function altertime(dt)
	{
		var action1 = _grossini.getActionByTag(1)
		var action2 = _tamara.getActionByTag(1)
		var action3 = _kathia.getActionByTag(1)
		
		action1.speed = math.random()*2
		action2.speed = math.random()*2
		action3.speed = math.random()*2
	}
	
	function title()
	{
		return  "Speed action";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SchedulerTest : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var jump1 = cc.action.JumpBy(4, cc.Point(-400, 0), 100, 4);
		var jump2 = jump1.reverse();
		var rot1 = cc.action.RotateBy(4, 360*2);
		var rot2 = rot1.reverse();
		
		var seq3_1 = cc.action.Sequence(jump2, jump1)
		var seq3_2 = cc.action.Sequence(rot1, rot2)
		var spawn = cc.action.Spawn(seq3_1, seq3_2)
		var action = cc.action.Speed(cc.action.RepeatForever(spawn),  0.5)
		action.tag = 1;
		
		var jump11 = cc.action.JumpBy(4, cc.Point(-400, 0), 100, 4);
		var jump21 = jump11.reverse();
		var rot11 = cc.action.RotateBy(4, 360*2);
		var rot21 = rot11.reverse();
		
		var seq3_11 = cc.action.Sequence(jump21, jump11)
		var seq3_21 = cc.action.Sequence(rot11, rot21)
		var spawn1 = cc.action.Spawn(seq3_11, seq3_21)
		var action1 = cc.action.Speed(cc.action.RepeatForever(spawn1), 1.5)
		action1.tag = 1;
		
		var jump12 = cc.action.JumpBy(4, cc.Point(-400, 0), 100, 4);
		var jump22 = jump12.reverse();
		var rot12 = cc.action.RotateBy(4, 360*2);
		var rot22 = rot12.reverse();
		
		var seq3_12 = cc.action.Sequence(jump22, jump12)
		var seq3_22 = cc.action.Sequence(rot12, rot22)
		var spawn2 = cc.action.Spawn(seq3_12, seq3_22)
		var action2 = cc.action.Speed(cc.action.RepeatForever(spawn2), 1.0)
		action2.tag = 1;
		
		_grossini.runAction(action)
		_tamara.runAction(action1)
		_kathia.runAction(action2)
		
		var emitter = cc.particle.Fireworks();
		emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		addChild(emitter)
	}
	
	function title()
	{
		return "SchedulerTest Demo"
	}
}

////////////////////////////////////////////////////////////////////////////////

class CocosEaseActionTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		addChild(eaTest.restartEaseActionTest());
		cocos.director.replaceScene(this)
	}
}

return CocosEaseActionTestScene()