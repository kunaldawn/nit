var pack = script.locator

class CocosParticleTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		addChild(pTest.restartParticleTest());
		cocos.director.replaceScene(this)
	}
}

class ParticleTest
{
	sceneindex = 0
	scenemaxcnt = 33
	
	constructor()
	{
	}
	
	function _createParticleTest(index)
	{
		var layer = null
		
		switch (index)
		{
		case 0:
			layer = DemoFlower()
			break
		case 1:
			layer = DemoGalaxy()
			break
		case 2:
			layer = DemoFirework()
			break
		case 3:
			layer = DemoSpiral()
			break
		case 4:
			layer = DemoSun()
			break
		case 5:
			layer = DemoMeteor()
			break
		case 6:
			layer = DemoFire()
			break
		case 7:
			layer = DemoSmoke()
			break
		case 8:
			layer = DemoExplosion()
			break
		case 9:
			layer = DemoSnow()
			break
		case 10:
			layer = DemoRain()
			break
		case 11:
			layer = DemoBigFlower()
			break
		case 12:
			layer = DemoRotFlower()
			break
		case 13:
			layer = DemoModernArt()
			break
		case 14:
			layer = DemoRing()
			break
		case 15:
			layer = ParallaxParticle()
			break
		case 16:
			layer = DemoParticleFromFile("BoilingFoam") 
			break
		case 17:
			layer =  DemoParticleFromFile("BurstPipe") 
			break
		case 18:
			layer = DemoParticleFromFile("Comet") 
			break
		case 19:
			layer = DemoParticleFromFile("debian")
			break
		case 20:
			layer = DemoParticleFromFile("ExplodingRing")
			break
		case 21:
			layer = DemoParticleFromFile("LavaFlow") 
			break
		case 22:
			layer = DemoParticleFromFile("SpinningPeas")
			break
		case 23:
			layer = DemoParticleFromFile("SpookyPeas")
			break
		case 24:
			layer = DemoParticleFromFile("Upsidedown") 
			break
		case 25:
			layer = DemoParticleFromFile("Flower")
			break
		case 26:
			layer = DemoParticleFromFile("Spiral")
			break
		case 27:
			layer = DemoParticleFromFile("Galaxy")
			break
		case 28:
			layer = RadiusMode1()
			break
		case 29:
			layer = RadiusMode2()
			break
		case 30:
			layer = Issue704()
			break
		case 31:
			layer = Issue870()
			break
		case 32:
			layer = DemoParticleFromFile("Phoenix")
			break
		
		}
		
		return layer
	}
	
	function nextParticleTest()
	{
		sceneindex++
		sceneindex = sceneindex % scenemaxcnt
		
		return _createParticleTest(sceneindex)
	}
	
	function backParticleTest()
	{
		sceneindex--
		if (sceneindex < 0)
			sceneindex = scenemaxcnt -1
			
		return _createParticleTest(sceneindex)
	}
	
	function restartParticleTest()
	{
		return _createParticleTest(sceneindex)
	}
}

pTest := ParticleTest()

class ParticleDemo : cc.ScriptLayer
{
	m_emitter = null
	m_background = null
	m_colorLayer = null
	
	constructor()
	{
		base.constructor()
		
		m_colorLayer = cc.LayerColor(nit.Color(127/255, 127/255, 127/255, 255/255))
		addChild( m_colorLayer, -100 )
		
		var label = cc.LabelTTF(title(), "Arial", 28)
		var size = cocos.director.winSize
		label.position = cc.Point(size.width / 2, size.height - 50)
		this.addChild(label, 100, 1000)
		
		var tapScreen = cc.LabelTTF("(Tap the Screen)", "Arial", 20)
		tapScreen.position = cc.Point(size.width / 2, size.height - 80)
		addChild(tapScreen, 100);
	
		m_background = cc.Sprite(pack.locate("background3.png"))
		m_background.position = cc.Point(size.width / 2, size.height - 180)
		this.addChild(m_background, 5)
		
		var labelAtlas = cc.LabelAtlas("0000", pack.locate("fps_images.png", "*fonts"), 16, 24, char('.'))
		this.addChild(labelAtlas, 100, 1)
		labelAtlas.position = cc.Point(size.width - 66, 50)
		
		var move = cc.action.MoveBy(4, cc.Point(300,0) );
		var move_back = move.reverse();
		var seq = cc.action.Sequence( move, move_back);
		m_background.runAction( cc.action.RepeatForever(seq) );
		
		var item1 = cc.MenuItemImage(pack.locate("b1.png"), pack.locate("b2.png"), this,backCallBack);
		var item2 = cc.MenuItemImage(pack.locate("r1.png"), pack.locate("r2.png"), this,restartCallBack);		
		var item3 = cc.MenuItemImage(pack.locate("f1.png"), pack.locate("f2.png"), this,nextCallBack);
		var item4 = cc.MenuItemToggle(this, toggleCallback,
			cc.MenuItemFont( "Free Movement" ),
			cc.MenuItemFont( "Relative Movement" ),
			cc.MenuItemFont( "Grouped Movement" ));
		
		var menu = cc.Menu(item1, item2, item3, item4);
		menu.position= cc.Point(0,0);
		item1.position= cc.Point( size.width/2 - 100,30) ;
		item2.position= cc.Point( size.width/2, 30) ;
		item3.position= cc.Point( size.width/2 + 100,30) ;
		item4.position= cc.Point( 0, 100) ;
		item4.anchorPoint =  cc.Point(0,0);
		addChild( menu, 100 );	
		
		touchEnabled = true
		cocos.director.timer.channel().bind(Events.OnTick, this, onTick );
		channel().bind(Events.OnCCTouchMoved, this, onTouchMove)
		channel().bind(Events.OnCCTouchEnded, this, onTouchEnded)
		
	}
	
	function onTick(dt)
	{
		if(m_emitter)
		{
			var atlas = this.getChildByTag(1);
			var str = m_emitter.particleCount;
			atlas.string = str.tostring();
		}
	}
	
	function onTouchMove(evt: cc.TouchEvent)
	{
		return onTouchEnded(evt);
	}
	
	function onTouchEnded(evt: cc.TouchEvent)
	{
		var touch = evt.touch0.locationInView(evt.touch0.view)
		touch = cocos.director.toGl(touch)

		var pos =  cc.Point(0,0);
		if (m_background)
		{
			pos = m_background.toWorld(pos);
		}
		var newPos = cc.Point(0,0);
		newPos.x = touch.x - pos.x;
		newPos.y = touch.y - pos.y;
		m_emitter.position = newPos;	
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
		var s = CocosParticleTestScene()
		m_colorLayer.color = nit.Color(0xFF808080)
		s.addChild(pTest.backParticleTest())
		cocos.director.replaceScene(s)
	}
	
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		var s = CocosParticleTestScene()
		m_colorLayer.color = nit.Color(0xFF808080)
		s.addChild(pTest.restartParticleTest())
		cocos.director.replaceScene(s)
	}

	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = CocosParticleTestScene()
		m_colorLayer.color = nit.Color(0xFF808080)
		s.addChild(pTest.nextParticleTest())
		cocos.director.replaceScene(s)
	}

	function toggleCallback()
	{
		if ( m_emitter.positionType == 2)
			m_emitter.positionType = 0;
		else if (m_emitter.positionType == 0)
			m_emitter.positionType = 1;
		else if (m_emitter.positionType == 1)
			m_emitter.positionType = 2;
	}

	function setEmitterPosition()
	{
		var s = cocos.director.winSize;
		m_emitter.position = cc.Point(s.width/2, s.height/2)
	}
}

////////////////////////////////////////////////////////////////////////////////

class DemoFirework : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Fireworks()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)

		m_background.addChild(m_emitter, 10);
		
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle FireWorks"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoFire : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Fire()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle Fire"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoSun : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Sun()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle Sun"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoGalaxy : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Galaxy()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle Galaxy"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoFlower : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Flower()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("stars.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle Flower"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoBigFlower : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Flower()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("stars.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)
		
		m_emitter.duration = -1;
		m_emitter.gravity = cc.Point(0,0);
		m_emitter.angle = 90;
		m_emitter.angleVar = 360;
		m_emitter.speed = 160;
		m_emitter.speedVar = 20;
		m_emitter.radialAccel = -120;
		m_emitter.radialAccelVar = 0;
		m_emitter.tangentialAccel = 30;
		m_emitter.tangentialAccelVar = 0;
		m_emitter.posVar = cc.Point(0,0);
		m_emitter.life = 4;
		m_emitter.lifeVar = 1;
		m_emitter.startSpin = 0;
		m_emitter.startSizeVar = 0;
		m_emitter.endSpin = 0;
		m_emitter.endSpinVar= 0;
		
		m_emitter.startSize=80;
		m_emitter.startSizeVar=40;
		m_emitter.endSize=-1;
		m_emitter.blendAdditive = true;

		m_emitter.emissionRate = m_emitter.totalParticles / m_emitter.life / 5

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle BIgFlower"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoRotFlower : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Flower()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("stars2.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)
		
		m_emitter.duration = -1;
		m_emitter.gravity = cc.Point(0,0);
		m_emitter.angle = 90;
		m_emitter.angleVar = 360;
		m_emitter.speed = 160;
		m_emitter.speedVar = 20;
		m_emitter.radialAccel = -120;
		m_emitter.radialAccelVar = 0;
		m_emitter.tangentialAccel = 30;
		m_emitter.tangentialAccelVar = 0;
		m_emitter.posVar = cc.Point(0,0);
		m_emitter.life = 3;
		m_emitter.lifeVar = 1;
		m_emitter.startSpin = 0;
		m_emitter.startSizeVar = 0;
		m_emitter.endSpin = 0;
		m_emitter.endSpinVar= 2000;
		
		m_emitter.startSize=30;
		m_emitter.startSizeVar=0;
		m_emitter.endSize=-1 ;
		m_emitter.blendAdditive = false;

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle RotFlower"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoMeteor : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Meteor()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle Meteor"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoSpiral : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Spiral()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle Spiral"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoExplosion : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Explosion()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("stars.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle Explosion"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoSmoke : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Smoke()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle Smoke"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoSnow : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Snow()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("snow.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)
		m_emitter.life = 3;
		m_emitter.lifeVar = 1;
		m_emitter.gravity = cc.Point(0,-10)
		m_emitter.speed = 130;
		m_emitter.speedVar = 30;
		
		m_emitter.startColor = m_emitter.startColor with { r = 0.9; g = 0.9; b = 0.9 }
		m_emitter.startColorVar = m_emitter.startColorVar with { b = 0.1 }
		
		m_emitter.emissionRate = m_emitter.totalParticles / m_emitter.life
		
		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle Snow"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoRain : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Rain()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		m_emitter.position = cc.Point(size.width / 3, size.height / 2)
		m_emitter.life = 4;

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Particle Rain"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoRing : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Flower()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("stars.png"))
		m_emitter.position = cc.Point(size.width / 2, size.height / 2)
		m_emitter.life = 10;
		m_emitter.lifeVar = 0;
		m_emitter.speed = 100;
		m_emitter.speedVar = 0;
		m_emitter.emissionRate = 10000;

		m_background.addChild(m_emitter, 10);
		setEmitterPosition()
	}
	
	function title()
	{
		return "Ring Demo"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class ParallaxParticle : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		m_background.parent.removeChild( m_background, true);
		m_background = null;
		
		var p = cc.ParallaxNode();
		this.addChild(p, 5);
		
		var p1 = cc.Sprite( pack.locate("background3.png") );
		var p2 = cc.Sprite( pack.locate("background3.png") );
		
		p.addChild( p1, 1, cc.Point(0.5, 1), cc.Point(0, 250) );
		p.addChild( p2, 2, cc.Point(1.5, 1), cc.Point(0, 50) );
		
		var size = cocos.director.winSize
		m_emitter = cc.particle.Flower()
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		
		p1.addChild(m_emitter, 10);
		m_emitter.position = cc.Point(250,200)
		
		var par = cc.particle.Sun();
		p2.addChild(par, 10);
		par.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		
		var move = cc.action.MoveBy(4, cc.Point(300,0) );
		var move_back = move.reverse();
		var seq = cc.action.Sequence( move, move_back);
		p.runAction( cc.action.RepeatForever( seq ) );
	}
	
	function title()
	{
		return "Parallax + Particle"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoModernArt : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		m_emitter = cc.ParticleSystemPoint(1000);
		m_background.addChild(m_emitter, 10);
			
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		
		m_emitter.duration = -1;
		m_emitter.gravity = cc.Point(0,0);
		m_emitter.angle = 0;
		m_emitter.angleVar = 360;
		m_emitter.radialAccel = 70;
		m_emitter.radialAccelVar = 10;
		m_emitter.tangentialAccel = 80;
		m_emitter.tangentialAccelVar = 0;
		m_emitter.speed = 50;
		m_emitter.speedVar = 10;
		m_emitter.position = cc.Point(size.width / 2, size.height / 2)
		m_emitter.posVar = cc.Point(0,0);
		m_emitter.life = 2;
		m_emitter.lifeVar = 0.3;
		m_emitter.emissionRate  = m_emitter.totalParticles/m_emitter.life;

		m_emitter.startColor = m_emitter.startColor with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
		m_emitter.startColorVar = m_emitter.startColorVar with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
		m_emitter.endColor = m_emitter.endColor with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
		m_emitter.endColorVar = m_emitter.endColorVar with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
		
		m_emitter.startSize=1;
		m_emitter.startSizeVar=1;
		m_emitter.endSize=32 ;
		m_emitter.endSizeVar=8 ;
		m_emitter.blendAdditive = false;
		
	}
	
	function title()
	{
		return "Varying size"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class RadiusMode1 : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		this.removeChild(m_background, true);
		m_background = null;
		
		var size = cocos.director.winSize
		m_emitter = cc.ParticleSystemQuad(200);
		this.addChild(m_emitter, 10);
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("stars-grayscale.png"))
		
		m_emitter.duration = -1;
		m_emitter.emitterMode = 1;
		m_emitter.startRadius = 0;
		m_emitter.startRadiusVar = 0;
		m_emitter.endRadius = 160;
		m_emitter.endRadiusVar = 0;
		
		m_emitter.rotatePerSecond = 180;
		m_emitter.rotatePerSecondVar = 0;
		
		m_emitter.angle = 90;
		m_emitter.angleVar = 0;

		m_emitter.startSpin = 0;
		m_emitter.startSizeVar = 0;
		m_emitter.endSpin = 0;
		m_emitter.endSpinVar= 0;
		
		m_emitter.position = cc.Point(size.width / 2, size.height / 2)
		m_emitter.posVar = cc.Point(0,0);
		m_emitter.life = 5;
		m_emitter.lifeVar = 0;
		m_emitter.emissionRate  = m_emitter.totalParticles/m_emitter.life;
		
		m_emitter with
		{
			startColor = startColor with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			startColorVar = startColorVar with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			endColor = endColor with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
			endColorVar = endColorVar with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
		}
		
		m_emitter.startSize=32;
		m_emitter.startSizeVar=0;
		m_emitter.endSize=-1 ;
		
		m_emitter.blendAdditive = false;
		m_colorLayer.color = nit.Color(0)
	}
	
	function title()
	{
		return "Radius Mode: Spiral"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class RadiusMode2 : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		this.removeChild(m_background, true);
		m_background = null;
		
		var size = cocos.director.winSize
		m_emitter = cc.ParticleSystemQuad(200);
		this.addChild(m_emitter, 10);
			
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("stars-grayscale.png"))
		
		m_emitter.duration = -1;
		m_emitter.emitterMode = 1;
		m_emitter.startRadius = 100;
		m_emitter.startRadiusVar = 0;
		m_emitter.endRadius = -1;
		m_emitter.endRadiusVar = 0;
		m_emitter.rotatePerSecond = 45;
		m_emitter.rotatePerSecondVar = 0;
		m_emitter.angle = 90;
		m_emitter.angleVar = 0;
		m_emitter.position = cc.Point(size.width / 2, size.height / 2)
		m_emitter.posVar = cc.Point(0,0);
		m_emitter.life = 4;
		m_emitter.lifeVar = 0;
		m_emitter.startSpin = 0;
		m_emitter.startSizeVar = 0;
		m_emitter.endSpin = 0;
		m_emitter.endSpinVar= 0;
		
		m_emitter with
		{
			startColor = startColor with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			startColorVar = startColorVar with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			endColor = endColor with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
			endColorVar = endColorVar with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
		}
	
		m_emitter.emissionRate  = m_emitter.totalParticles/m_emitter.life;
		
		m_emitter.startSize=32;
		m_emitter.startSizeVar=0;
		m_emitter.endSize=-1 ;
		m_emitter.blendAdditive = false;
		
		m_colorLayer.color = nit.Color(0,0,0,0)
	}
	
	function title()
	{
		return "Radius Mode: Semi Circle"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class Issue704 : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		this.removeChild(m_background, true);
		m_background = null;
		
		var size = cocos.director.winSize
		m_emitter = cc.ParticleSystemQuad(100);
		this.addChild(m_emitter, 10);
			
		m_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		
		m_emitter.duration = -1;
		m_emitter.emitterMode = 1;
		m_emitter.startRadius = 50;
		m_emitter.startRadiusVar = 0;
		m_emitter.endRadius = -1;
		m_emitter.endRadiusVar = 0;
		m_emitter.rotatePerSecond = 0;
		m_emitter.rotatePerSecondVar = 0;
		m_emitter.angle = 0;
		m_emitter.angleVar = 0;
		m_emitter.position = cc.Point(size.width / 2, size.height / 2)
		m_emitter.posVar = cc.Point(0,0);
		m_emitter.life = 5;
		m_emitter.lifeVar = 0;
		m_emitter.startSpin = 0;
		m_emitter.startSizeVar = 0;
		m_emitter.endSpin = 0;
		m_emitter.endSpinVar= 0;
		
		m_emitter with
		{
			startColor = startColor with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			startColorVar = startColorVar with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			endColor = endColor with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
			endColorVar = endColorVar with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
		}
	
		m_emitter.emissionRate  = m_emitter.totalParticles / m_emitter.life;
		
		m_emitter.startSize=16;
		m_emitter.startSizeVar=0;
		m_emitter.endSize=-1 ;
		m_emitter.blendAdditive = false;
		
		var rot = cc.action.RotateBy(16,360);
		m_emitter.runAction( cc.action.RepeatForever(rot) );
		
		m_colorLayer.color = nit.Color(0,0,0,0)
	}
	
	function title()
	{
		return "Issue704. Free + Rot"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class Issue870 : ParticleDemo
{
	m_nIndex = 0;
	constructor()
	{
		base.constructor()
		
		this.removeChild(m_background, true);
		m_background = null;
		
		var size = cocos.director.winSize
		var system = cc.ParticleSystemQuad(pack.locate("SpinningPeas.plist"));
		system.setTextureWithRect(cocos.textureCache.addImage(pack.locate("particles.png")), cc.Rect(0,0,32,32))
		this.addChild(system, 10);
		m_emitter = system;
		
		m_colorLayer.color = nit.Color(0,0,0,0)
		cocos.director.scheduler.repeat( this, updateQuads, 2.0);
	}
	
	function title()
	{
		return "Issue 870. SubRect"
	}
	
	function updateQuads(dt)
	{
		m_nIndex = (m_nIndex+1)%4;
		var rect = cc.Rect(m_nIndex*32, 0, 32, 32);
		var system = m_emitter;
		system.setTextureWithRect(m_emitter.texture, rect)
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class DemoParticleFromFile : ParticleDemo
{
	m_title = null;
	constructor(file)
	{
		m_title = file;
		base.constructor()
		
		this.removeChild(m_background, true);
		m_background = null;
		
		var size = cocos.director.winSize
		var filename = file + ".plist";
		m_emitter = cc.ParticleSystemQuad(pack.locate( filename ));
		setEmitterPosition()
		this.addChild(m_emitter, 10);
		
		m_colorLayer.color = nit.Color(0,0,0,0)
	}
	
	function title()
	{
		return m_title
	}
	
}

////////////////////////////////////////////////////////////////////////////////

return CocosParticleTestScene()