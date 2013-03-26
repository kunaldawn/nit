var pack = script.locator

SID_STEP1 := 100
SID_STEP1 := 101
SID_STEP1 := 102

IDC_PAUSE := 200


class IntervalTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		addChild(IntervalLayer() );
		cocos.director.replaceScene(this)
	}
}

class IntervalLayer : cc.ScriptLayer 
{
	m_time0 = 0;
	m_time1 = 0;
	m_time2 = 0;
	m_time3 = 0;
	m_time4 = 0;
	
	m_label0 = null;
	m_label1 = null;
	m_label2 = null;
	m_label3 = null;
	m_label4 = null;
	
	constructor()
	{
		
		base.constructor()
		
		var s = cocos.director.winSize;
		var sun = cc.particle.Sun();
		sun.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		sun.position= cc.Point(s.width - 32, s.height - 32) ;
		sun.totalParticles=130;
		sun.life= 0.6;
		this.addChild( sun );
	
		m_label0 =cc.LabelBMFont("0", pack.locate("bitmapFontTest4.fnt"))
		m_label1 =cc.LabelBMFont("0", pack.locate("bitmapFontTest4.fnt"))
		m_label2 =cc.LabelBMFont("0", pack.locate("bitmapFontTest4.fnt"))	
		m_label3 =cc.LabelBMFont("0", pack.locate("bitmapFontTest4.fnt"))
		m_label4 =cc.LabelBMFont("0", pack.locate("bitmapFontTest4.fnt"))
		
		session.timer.channel().bind(Events.OnTick, this, step0)
		session.scheduler.repeat(this, step1, 0.01)
		session.scheduler.repeat(this, step2, 0.01)
		session.scheduler.repeat(this, step3, 1)
		session.scheduler.repeat(this, step4, 2)
		
		m_label0.position = cc.Point(s.width * 1/6, s.height/2)
		m_label1.position = cc.Point(s.width * 2/6, s.height/2)
		m_label2.position = cc.Point(s.width * 3/6, s.height/2)
		m_label3.position = cc.Point(s.width * 4/6, s.height/2)
		m_label4.position = cc.Point(s.width * 5/6, s.height/2)
		
		addChild(m_label0)
		addChild(m_label1)
		addChild(m_label2)
		addChild(m_label3)
		addChild(m_label4)
		// Sprite
		var sprite = cc.Sprite(pack.locate("grossini.png", "*Image*"))
		this.addChild(sprite);
		sprite.position = cc.Point(40, 50);
		
		var jump = cc.action.JumpBy(3, cc.Point(s.width-80,0), 50, 4)
		sprite.runAction( cc.action.JumpTo(4,cc.Point(300,48), 100, 4) );
		
		sprite.runAction( 
			cc.action.RepeatForever( 
				cc.action.Sequence( jump,jump.reverse() )
			) 
		)
		// pause button
		var item = cc.MenuItemFont("Pause", this, onPause );
		var menu = cc.Menu( item)
		menu.position = cc.Point(s.width/2, s.height-50)
		this.addChild( menu );
	}
	
	function onPause()
	{
		if(cocos.director.paused)
			cocos.director.resume()
		else
			cocos.director.pause()
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	function step0(evt: TimeEvent)
	{
		m_time0 += evt.delta
		
		var time = math.floor(m_time0*10)/10
		m_label0.string = time.tostring()
	}
	
	function step1(evt: TimeEvent)
	{
		m_time1 += evt.delta
		
		var time = math.floor(m_time1*10)/10
		m_label1.string = time.tostring()
	}
	
	function step2(evt: TimeEvent)
	{
		m_time2 += evt.delta
		
		var time = math.floor(m_time2*10)/10
		m_label2.string = time.tostring()
	}
	
	function step3(evt: TimeEvent)
	{
		m_time3 += evt.delta
		var time = math.floor(m_time3*100)/100
		m_label3.string = time.tostring()
	}
	
	function step4(evt: TimeEvent)
	{
		m_time4 += evt.delta
		var time = math.floor(m_time4*100)/100
		m_label4.string = time.tostring()
	}
	
}