var pack = script.locator

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


class EaseActionTest
{
	sceneindex = 0
	scenemaxcnt = 14
	
	constructor()
	{
	}
	
	function _createEaseActionTest(index)
	{
		var layer = null
		
		switch (index)
		{
		case 0:
			layer = SpriteEase()
			break
		case 1:
			layer = SpriteEaseInOut()
			break
		case 2:
			layer = SpriteEaseExponential()
			break
		case 3:
			layer = SpriteEaseExponentialInOut()
			break
		case 4:
			layer = SpriteEaseSine()
			break
		case 5:
			layer = SpriteEaseSineInOut()
			break
		case 6:
			layer = SpriteEaseElastic()
			break
		case 7:
			layer = SpriteEaseElasticInOut()
			break
		case 8:
			layer = SpriteEaseBounce()
			break
		case 9:
			layer = SpriteEaseBounceInOut()
			break
		case 10:
			layer = SpriteEaseBack()
			break
		case 11:
			layer = SpriteEaseBackInOut()
			break
		case 12:
			layer = SpeedTest()
			break
		case 13:
			layer = SchedulerTest()
			break	
		}
		
		return layer
	}
	
	function nextEaseActionTest()
	{
		sceneindex++
		sceneindex = sceneindex % scenemaxcnt
		
		return _createEaseActionTest(sceneindex)
	}
	function backEaseActionTest()
	{
		sceneindex--
		if (sceneindex < 0)
			sceneindex = scenemaxcnt -1
			
		return _createEaseActionTest(sceneindex)
	}
	function restartEaseActionTest()
	{
		return _createEaseActionTest(sceneindex)
	}
}

eaTest := EaseActionTest()

class EaseSpriteDemo : cc.ScriptLayer
{
	m_grossini = null
	m_tamara = null
	m_kathia = null
	
	constructor()
	{
		
		base.constructor()
		//TouchEnabled = true
		
		m_grossini = cc.Sprite(pack.locate("grossini.png", "*Images*"))
		m_tamara = cc.Sprite(pack.locate("grossinis_sister1.png"))
		m_kathia = cc.Sprite(pack.locate("grossinis_sister2.png"))
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		
		m_grossini.position = cc.Point(60, 50);
		m_kathia.position = cc.Point(60, 150);
		m_tamara.position = cc.Point(60, 250);
		
		var rot = cc.action.RotateBy(16, -3600)
		
		this.addChild(m_grossini, 3);
		this.addChild(m_tamara, 2);
		this.addChild(m_kathia, 1);
		
		
		var label = cc.LabelTTF(title(), "Arial", 32)
		var size = cocos.director.winSize
		label.position = cc.Point(x / 2, y - 50)
		this.addChild(label)
		
		var item1 = cc.MenuItemImage(s_pPathB1,s_pPathB2, this,backCallBack);
		var item2 = cc.MenuItemImage(s_pPathR1, s_pPathR2, this,restartCallBack);		
		var item3 = cc.MenuItemImage(s_pPathF1, s_pPathF2, this,nextCallBack);
		
		var menu = cc.Menu(item1, item2, item3);
		menu.position= cc.Point(0,0);
		item1.position= cc.Point( size.width/2 - 100,30) ;
		item2.position= cc.Point( size.width/2, 30) ;
		item3.position= cc.Point( size.width/2 + 100,30) ;
		this.addChild( menu, 1 );	
		
		//this.TouchEnabled = true
		//cocos.director.Timer.Channel().Bind(Events.OnTick, this, Tick );
		//Channel().Bind(Events.OnCCTouchBegin, this, OnTouchBegin)
		//Channel().Bind(Events.OnCCTouchMoved, this, OnTouchMove)
		//Channel().Bind(Events.OnCCTouchEnded, this, OnTouchEnded)
		
	}
	
	function positionForTwo()
	{
		m_grossini.position = cc.Point( 60, 120)
		m_tamara.position = cc.Point( 60, 220)
		m_kathia.visible = false;
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		var s = CocosEaseActionTestScene()
		s.addChild(eaTest.backEaseActionTest())
		cocos.director.replaceScene(s)
	}
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		
		var s = CocosEaseActionTestScene()
		s.addChild(eaTest.restartEaseActionTest())
		cocos.director.replaceScene(s)
	}
	function nextCallBack(evt: cc.MenuItemEvent)
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move3= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseIn(move1, 3.0);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseOut( move2, 3.0);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence( move3, move_back)
		var seq2 = cc.action.Sequence( move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence( move_ease_out, move_ease_out_back)
		
		var a2 = m_grossini.runAction( cc.action.RepeatForever(seq1))
		a2.tag = 1;
		var a1 = m_tamara.runAction( cc.action.RepeatForever(seq2))
		a1.tag = 1;
		var a = m_kathia.runAction( cc.action.RepeatForever(seq3))
		a.tag = 1;
		
		session.scheduler.once(this, testStopAction, 6)
	}
	
	function title()
	{
		return "EaseIn - EaseOut - Stop"
	}
	
	function testStopAction(dt)
	{
		m_tamara.stopActionByTag(1);
		m_kathia.stopActionByTag(1);
		m_grossini.stopActionByTag(1);
	}
	
}
////////////////////////////////////////////////////////////////////////////////
class SpriteEaseInOut : EaseSpriteDemo
{
	constructor()
	{
		base.constructor()
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move3= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_ease_inout1 = cc.action.EaseInOut(move1, 2.0);
		var move_ease_inout_back1 = move_ease_inout1.reverse();
		var move_ease_inout2 = cc.action.EaseInOut( move2, 3.0);
		var move_ease_inout_back2 = move_ease_inout2.reverse();
		var move_ease_inout3 = cc.action.EaseInOut( move3, 4.0);
		var move_ease_inout_back3 = move_ease_inout3.reverse();
		
		var seq1 = cc.action.Sequence( move_ease_inout1, move_ease_inout_back1)
		var seq2 = cc.action.Sequence( move_ease_inout2, move_ease_inout_back2)
		var seq3 = cc.action.Sequence( move_ease_inout3, move_ease_inout_back3)
		
		var a2 = m_tamara.runAction( cc.action.RepeatForever(seq1))
		a2.tag = 1;
		var a1 = m_kathia.runAction( cc.action.RepeatForever(seq2))
		a1.tag = 1;
		var a = m_grossini.runAction( cc.action.RepeatForever(seq3))
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move3= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseExponentialIn(move1, 3.0);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseExponentialOut( move2, 3.0);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence( move3, move_back)
		var seq2 = cc.action.Sequence( move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence( move_ease_out, move_ease_out_back)
		
		var a2 = m_grossini.runAction( cc.action.RepeatForever(seq1))
		a2.tag = 1;
		var a1 = m_tamara.runAction( cc.action.RepeatForever(seq2))
		a1.tag = 1;
		var a = m_kathia.runAction( cc.action.RepeatForever(seq3))
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_back = move1.reverse()
		
		var move_ease = cc.action.EaseExponentialInOut(move1, 3.0);
		var move_ease_back = move_ease.reverse();
		
		var seq1 = cc.action.Sequence( move2, move_back)
		var seq2 = cc.action.Sequence( move_ease, move_ease_back)
		
		this.positionForTwo()
		
		var a2 = m_grossini.runAction( cc.action.RepeatForever(seq1))
		var a1 = m_tamara.runAction( cc.action.RepeatForever(seq2))
		
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move3= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseSineIn(move1, 3.0);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseSineOut( move2, 3.0);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence( move3, move_back)
		var seq2 = cc.action.Sequence( move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence( move_ease_out, move_ease_out_back)
		
		var a2 = m_grossini.runAction( cc.action.RepeatForever(seq1))
		a2.tag = 1;
		var a1 = m_tamara.runAction( cc.action.RepeatForever(seq2))
		a1.tag = 1;
		var a = m_kathia.runAction( cc.action.RepeatForever(seq3))
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move_back = move1.reverse()
		
		var move_ease = cc.action.EaseSineInOut(move2);
		var move_ease_back = move_ease.reverse();
		
		var seq1 = cc.action.Sequence( move1, move_back)
		var seq2 = cc.action.Sequence( move_ease, move_ease_back)
	
		this.positionForTwo();
		
		var a2 = m_grossini.runAction( cc.action.RepeatForever(seq1))
		var a1 = m_tamara.runAction( cc.action.RepeatForever(seq2))
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move3= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseElasticIn(move2);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseElasticOut( move3);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence( move1, move_back)
		var seq2 = cc.action.Sequence( move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence( move_ease_out, move_ease_out_back)
		
		m_grossini.runAction( cc.action.RepeatForever(seq1))
		m_tamara.runAction( cc.action.RepeatForever(seq2))
		m_kathia.runAction( cc.action.RepeatForever(seq3))
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move3= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move4= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_back = move1.reverse()
		
		var move_ease_inout1 = cc.action.EaseElasticInOut(move2, 0.3);
		var move_ease_inout_back1 = move_ease_inout1.reverse();
		
		var move_ease_inout2 = cc.action.EaseElasticInOut(move3, 0.45);
		var move_ease_inout_back2 = move_ease_inout1.reverse();
		
		var move_ease_inout3 = cc.action.EaseElasticInOut(move4, 0.6);
		var move_ease_inout_back3 = move_ease_inout3.reverse();
		
		
		
		var seq1 = cc.action.Sequence( move_ease_inout1, move_ease_inout_back1)
		var seq2 = cc.action.Sequence( move_ease_inout2, move_ease_inout_back2)
		var seq3 = cc.action.Sequence( move_ease_inout3, move_ease_inout_back3)
		
		m_tamara.runAction( cc.action.RepeatForever(seq1))
		m_kathia.runAction( cc.action.RepeatForever(seq2))
		m_grossini.runAction( cc.action.RepeatForever(seq3))
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move3= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseBounceIn(move1, 3.0);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseBounceOut( move2, 3.0);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence( move3, move_back)
		var seq2 = cc.action.Sequence( move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence( move_ease_out, move_ease_out_back)
		
		m_grossini.runAction( cc.action.RepeatForever(seq1))
		m_tamara.runAction( cc.action.RepeatForever(seq2))
		m_kathia.runAction( cc.action.RepeatForever(seq3))
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move3= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_back = move1.reverse()
		
		var move_ease = cc.action.EaseBounceInOut(move1, 3.0);
		var move_ease_back = move_ease.reverse();
		
		var seq1 = cc.action.Sequence( move3, move_back)
		var seq2 = cc.action.Sequence( move_ease, move_ease_back)
		
		this.positionForTwo();
		m_grossini.runAction( cc.action.RepeatForever(seq1))
		m_tamara.runAction( cc.action.RepeatForever(seq2))
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move3= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_back = move1.reverse()
		
		var move_ease_in = cc.action.EaseBackIn(move1, 3.0);
		var move_ease_in_back = move_ease_in.reverse();
		var move_ease_out = cc.action.EaseBackOut( move2, 3.0);
		var move_ease_out_back = move_ease_out.reverse();
		
		var seq1 = cc.action.Sequence( move3, move_back)
		var seq2 = cc.action.Sequence( move_ease_in, move_ease_in_back)
		var seq3 = cc.action.Sequence( move_ease_out, move_ease_out_back)
		
		m_grossini.runAction( cc.action.RepeatForever(seq1))
		m_tamara.runAction( cc.action.RepeatForever(seq2))
		m_kathia.runAction( cc.action.RepeatForever(seq3))
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
		
		var move1= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move2= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		var move3= cc.action.MoveBy( 3, cc.Point( 350, 0) )
		
		var move_back = move1.reverse()
		
		var move_ease = cc.action.EaseBackInOut(move1, 3.0);
		var move_ease_back = move_ease.reverse();
		
		var seq1 = cc.action.Sequence( move3, move_back)
		var seq2 = cc.action.Sequence( move_ease, move_ease_back)
		
		this.positionForTwo()
		m_grossini.runAction( cc.action.RepeatForever(seq1))
		m_tamara.runAction( cc.action.RepeatForever(seq2))
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
		
		var jump1 = cc.action.JumpBy( 4, cc.Point(-400, 0), 100, 4);
		var jump2 = jump1.reverse();
		var rot1 = cc.action.RotateBy( 4, 360*2);
		var rot2 = rot1.reverse();
		
		var seq3_1 = cc.action.Sequence( jump2, jump1)
		var seq3_2 = cc.action.Sequence( rot1, rot2)
		var spawn = cc.action.Spawn( seq3_1, seq3_2 )
		var action = cc.action.Speed( cc.action.RepeatForever(spawn),  1)
		action.tag=1;
		
		var jump11 = cc.action.JumpBy( 4, cc.Point(-400, 0), 100, 4);
		var jump21 = jump11.reverse();
		var rot11 = cc.action.RotateBy( 4, 360*2);
		var rot21 = rot11.reverse();
		
		var seq3_11 = cc.action.Sequence( jump21, jump11)
		var seq3_21 = cc.action.Sequence( rot11, rot21)
		var spawn1 = cc.action.Spawn( seq3_11, seq3_21 )
		var action1 = cc.action.Speed( cc.action.RepeatForever(spawn1), 2)
		action1.tag=1;
		
		var jump12 = cc.action.JumpBy( 4, cc.Point(-400, 0), 100, 4);
		var jump22 = jump12.reverse();
		var rot12 = cc.action.RotateBy( 4, 360*2);
		var rot22 = rot12.reverse();
		
		var seq3_12 = cc.action.Sequence( jump22, jump12)
		var seq3_22 = cc.action.Sequence( rot12, rot22)
		var spawn2 = cc.action.Spawn( seq3_12, seq3_22 )
		var action2 = cc.action.Speed( cc.action.RepeatForever(spawn2),3 )
		action2.tag=1;
		
		m_grossini.runAction( action)
		m_tamara.runAction( action1)
		m_kathia.runAction( action2)
		
		session.scheduler.repeat(this, altertime, 1)
	}

	function altertime(dt)
	{
		var action1 = m_grossini.getActionByTag(1)
		var action2 = m_tamara.getActionByTag(1)
		var action3 = m_kathia.getActionByTag(1)
		
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
		
		var jump1 = cc.action.JumpBy( 4, cc.Point(-400, 0), 100, 4);
		var jump2 = jump1.reverse();
		var rot1 = cc.action.RotateBy( 4, 360*2);
		var rot2 = rot1.reverse();
		
		var seq3_1 = cc.action.Sequence( jump2, jump1)
		var seq3_2 = cc.action.Sequence( rot1, rot2)
		var spawn = cc.action.Spawn( seq3_1, seq3_2 )
		var action = cc.action.Speed( cc.action.RepeatForever(spawn),  0.5)
		action.tag=1;
		
		var jump11 = cc.action.JumpBy( 4, cc.Point(-400, 0), 100, 4);
		var jump21 = jump11.reverse();
		var rot11 = cc.action.RotateBy( 4, 360*2);
		var rot21 = rot11.reverse();
		
		var seq3_11 = cc.action.Sequence( jump21, jump11)
		var seq3_21 = cc.action.Sequence( rot11, rot21)
		var spawn1 = cc.action.Spawn( seq3_11, seq3_21 )
		var action1 = cc.action.Speed( cc.action.RepeatForever(spawn1), 1.5)
		action1.tag=1;
		
		var jump12 = cc.action.JumpBy( 4, cc.Point(-400, 0), 100, 4);
		var jump22 = jump12.reverse();
		var rot12 = cc.action.RotateBy( 4, 360*2);
		var rot22 = rot12.reverse();
		
		var seq3_12 = cc.action.Sequence( jump22, jump12)
		var seq3_22 = cc.action.Sequence( rot12, rot22)
		var spawn2 = cc.action.Spawn( seq3_12, seq3_22 )
		var action2 = cc.action.Speed( cc.action.RepeatForever(spawn2),1.0 )
		action2.tag=1;
		
		m_grossini.runAction( action)
		m_tamara.runAction( action1)
		m_kathia.runAction( action2)
		
		var emitter = cc.particle.Fireworks();
		emitter.texture = cocos.textureCache.addImage( pack.locate("fire.png") )
		addChild(emitter)
	}
	
	function title()
	{
		return "SchedulerTest Demo"
	}
	
}

