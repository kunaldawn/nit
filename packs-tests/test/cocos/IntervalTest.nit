var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var SID_STEP1 = 100
var SID_STEP1 = 101
var SID_STEP1 = 102

var IDC_PAUSE = 200

////////////////////////////////////////////////////////////////////////////////

class IntervalLayer : cc.ScriptLayer 
{
	var _time0 = 0;
	var _time1 = 0;
	var _time2 = 0;
	var _time3 = 0;
	var _time4 = 0;
	
	var _label0 = null;
	var _label1 = null;
	var _label2 = null;
	var _label3 = null;
	var _label4 = null;
	
	constructor()
	{
		
		base.constructor()
		
		var s = cocos.director.winSize;
		var sun = cc.particle.Sun() with
		{
			texture = cocos.textureCache.addImage(pack.locate("fire.png"))
			position = cc.Point(s.width - 32, s.height - 32) ;
			totalParticles = 130;
			life = 0.6;
		}
		addChild(sun);
	
		_label0 = cc.LabelBMFont("0", pack.locate("bitmapFontTest4.fnt"))
		_label1 = cc.LabelBMFont("0", pack.locate("bitmapFontTest4.fnt"))
		_label2 = cc.LabelBMFont("0", pack.locate("bitmapFontTest4.fnt"))	
		_label3 = cc.LabelBMFont("0", pack.locate("bitmapFontTest4.fnt"))
		_label4 = cc.LabelBMFont("0", pack.locate("bitmapFontTest4.fnt"))
		
		cocos.director.timer.channel().bind(Events.OnTick, this, step0)
		cocos.director.scheduler.repeat(this, step1, 0.01)
		cocos.director.scheduler.repeat(this, step2, 0.01)
		cocos.director.scheduler.repeat(this, step3, 1)
		cocos.director.scheduler.repeat(this, step4, 2)
		
		_label0.position = cc.Point(s.width * 1/6, s.height/2)
		_label1.position = cc.Point(s.width * 2/6, s.height/2)
		_label2.position = cc.Point(s.width * 3/6, s.height/2)
		_label3.position = cc.Point(s.width * 4/6, s.height/2)
		_label4.position = cc.Point(s.width * 5/6, s.height/2)
		
		addChild(_label0)
		addChild(_label1)
		addChild(_label2)
		addChild(_label3)
		addChild(_label4)
		
		// Sprite
		var sprite = cc.Sprite(pack.locate("grossini.png", "*Image*"))
		addChild(sprite);
		sprite.position = cc.Point(40, 50);
		
		var jump = cc.action.JumpBy(3, cc.Point(s.width-80, 0), 50, 4)
		sprite.runAction(cc.action.JumpTo(4, cc.Point(300, 48), 100, 4));
		
		sprite.runAction(
			cc.action.RepeatForever(
				cc.action.Sequence(jump, jump.reverse())
			) 
		)
		
		// pause button
		var item = cc.MenuItemFont("Pause", this, onPause);
		var menu = cc.Menu(item)
		menu.position = cc.Point(s.width/2, s.height-50)
		addChild(menu);
	}
	
	function onPause()
	{
		if (cocos.director.paused)
			cocos.director.resume()
		else
			cocos.director.pause()
	}
	
	function step0(evt: TimeEvent)
	{
		_time0 += evt.delta
		
		var time = math.floor(_time0 * 10) / 10
		_label0.string = time.tostring()
	}
	
	function step1(evt: TimeEvent)
	{
		_time1 += evt.delta
		
		var time = math.floor(_time1 * 10) / 10
		_label1.string = time.tostring()
	}
	
	function step2(evt: TimeEvent)
	{
		_time2 += evt.delta
		
		var time = math.floor(_time2 * 10) / 10
		_label2.string = time.tostring()
	}
	
	function step3(evt: TimeEvent)
	{
		_time3 += evt.delta
		var time = math.floor(_time3 * 100) / 100
		_label3.string = time.tostring()
	}
	
	function step4(evt: TimeEvent)
	{
		_time4 += evt.delta
		var time = math.floor(_time4 * 100) / 100
		_label4.string = time.tostring()
	}
}

////////////////////////////////////////////////////////////////////////////////

class IntervalTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		addChild(IntervalLayer());
		cocos.director.replaceScene(this)
	}
}

return IntervalTestScene()