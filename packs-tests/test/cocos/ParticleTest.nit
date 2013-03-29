var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

class ParticleTest
{
	var _sceneIndex = 0
	var _sceneMax = 33
	
	constructor()
	{
	}
	
	function _createParticleTest(index)
	{
		var layer = null
		
		switch (index)
		{
		case 0: return DemoFlower()
		case 1: return DemoGalaxy()
		case 2: return DemoFirework()
		case 3: return DemoSpiral()
		case 4: return DemoSun()
		case 5: return DemoMeteor()
		case 6: return DemoFire()
		case 7: return DemoSmoke()
		case 8: return DemoExplosion()
		case 9: return DemoSnow()
		case 10: return DemoRain()
		case 11: return DemoBigFlower()
		case 12: return DemoRotFlower()
		case 13: return DemoModernArt()
		case 14: return DemoRing()
		case 15: return ParallaxParticle()
		case 16: return DemoParticleFromFile("BoilingFoam") 
		case 17: return DemoParticleFromFile("BurstPipe") 
		case 18: return DemoParticleFromFile("Comet") 
		case 19: return DemoParticleFromFile("debian")
		case 20: return DemoParticleFromFile("ExplodingRing")
		case 21: return DemoParticleFromFile("LavaFlow") 
		case 22: return DemoParticleFromFile("SpinningPeas")
		case 23: return DemoParticleFromFile("SpookyPeas")
		case 24: return DemoParticleFromFile("Upsidedown") 
		case 25: return DemoParticleFromFile("Flower")
		case 26: return DemoParticleFromFile("Spiral")
		case 27: return DemoParticleFromFile("Galaxy")
		case 28: return RadiusMode1()
		case 29: return RadiusMode2()
		case 30: return Issue704()
		case 31: return Issue870()
		case 32: return DemoParticleFromFile("Phoenix")
		}
	}
	
	function nextParticleTest()
	{
		_sceneIndex++
		_sceneIndex = _sceneIndex % _sceneMax
		
		return _createParticleTest(_sceneIndex)
	}
	
	function backParticleTest()
	{
		_sceneIndex--
		if (_sceneIndex < 0)
			_sceneIndex = _sceneMax -1
			
		return _createParticleTest(_sceneIndex)
	}
	
	function restartParticleTest()
	{
		return _createParticleTest(_sceneIndex)
	}
}

var pTest = ParticleTest()

////////////////////////////////////////////////////////////////////////////////

class ParticleDemo : cc.ScriptLayer
{
	var _emitter = null
	var _background = null
	var _colorLayer = null
	
	constructor()
	{
		base.constructor()
		
		_colorLayer = cc.LayerColor(nit.Color(127/255, 127/255, 127/255, 255/255))
		addChild(_colorLayer, -100)
		
		var label = cc.LabelTTF(title(), "Arial", 28)
		var size = cocos.director.winSize
		label.position = cc.Point(size.width / 2, size.height - 50)
		addChild(label, 100, 1000)
		
		var tapScreen = cc.LabelTTF("(Tap the Screen)", "Arial", 20)
		tapScreen.position = cc.Point(size.width / 2, size.height - 80)
		addChild(tapScreen, 100)
	
		_background = cc.Sprite(pack.locate("background3.png"))
		_background.position = cc.Point(size.width / 2, size.height - 180)
		addChild(_background, 5)
		
		var labelAtlas = cc.LabelAtlas("0000", pack.locate("fps_images.png", "*fonts"), 16, 24, $'.')
		addChild(labelAtlas, 100, 1)
		labelAtlas.position = cc.Point(size.width - 66, 50)
		
		var move = cc.action.MoveBy(4, cc.Point(300,0))
		var move_back = move.reverse()
		var seq = cc.action.Sequence(move, move_back)
		_background.runAction(cc.action.RepeatForever(seq))
		
		var item1 = cc.MenuItemImage(pack.locate("b1.png"), pack.locate("b2.png"), this,backCallBack)
		var item2 = cc.MenuItemImage(pack.locate("r1.png"), pack.locate("r2.png"), this,restartCallBack)		
		var item3 = cc.MenuItemImage(pack.locate("f1.png"), pack.locate("f2.png"), this,nextCallBack)
		var item4 = cc.MenuItemToggle(this, toggleCallback,
			cc.MenuItemFont("Free Movement"),
			cc.MenuItemFont("Relative Movement"),
			cc.MenuItemFont("Grouped Movement"))
		
		var menu = cc.Menu(item1, item2, item3, item4)
		menu.position = cc.Point(0,0)
		item1.position = cc.Point(size.width/2 - 100,30) 
		item2.position = cc.Point(size.width/2, 30) 
		item3.position = cc.Point(size.width/2 + 100,30) 
		item4.position = cc.Point(0, 100) 
		item4.anchorPoint =  cc.Point(0,0)
		addChild(menu, 100)	
		
		touchEnabled = true
		
		cocos.director.timer.channel().bind(EVT.TICK, this, onTick)
		channel().bind(EVT.CC_TOUCH_MOVED, this, onTouchMove)
		channel().bind(EVT.CC_TOUCH_ENDED, this, onTouchEnded)
		
	}
	
	function onTick(evt: TimeEvent)
	{
		if (_emitter)
		{
			var atlas = getChildByTag(1)
			var str = _emitter.particleCount
			atlas.string = str.tostring()
		}
	}
	
	function onTouchMove(evt: cc.TouchEvent)
	{
		return onTouchEnded(evt)
	}
	
	function onTouchEnded(evt: cc.TouchEvent)
	{
		var touch = evt.touch0.locationInView(evt.touch0.view)
		touch = cocos.director.toGl(touch)

		var pos =  cc.Point(0,0)
		if (_background)
		{
			pos = _background.toWorld(pos)
		}
		var newPos = cc.Point(0,0)
		newPos.x = touch.x - pos.x
		newPos.y = touch.y - pos.y
		_emitter.position = newPos	
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
		_colorLayer.color = nit.Color(0xFF808080)
		s.addChild(pTest.backParticleTest())
		cocos.director.replaceScene(s)
	}
	
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		var s = CocosParticleTestScene()
		_colorLayer.color = nit.Color(0xFF808080)
		s.addChild(pTest.restartParticleTest())
		cocos.director.replaceScene(s)
	}

	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = CocosParticleTestScene()
		_colorLayer.color = nit.Color(0xFF808080)
		s.addChild(pTest.nextParticleTest())
		cocos.director.replaceScene(s)
	}

	function toggleCallback()
	{
		if (_emitter.positionType == 2)
			_emitter.positionType = 0
		else if (_emitter.positionType == 0)
			_emitter.positionType = 1
		else if (_emitter.positionType == 1)
			_emitter.positionType = 2
	}

	function setEmitterPosition()
	{
		var s = cocos.director.winSize
		_emitter.position = cc.Point(s.width/2, s.height/2)
	}
}

////////////////////////////////////////////////////////////////////////////////

class DemoFirework : ParticleDemo
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		_emitter = cc.particle.Fireworks()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)

		_background.addChild(_emitter, 10)
		
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
		_emitter = cc.particle.Fire()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Sun()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Galaxy()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Flower()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("stars.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Flower()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("stars.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)
		
		_emitter with
		{
			duration = -1
			gravity = cc.Point(0,0)
			angle = 90
			angleVar = 360
			speed = 160
			speedVar = 20
			radialAccel = -120
			radialAccelVar = 0
			tangentialAccel = 30
			tangentialAccelVar = 0
			posVar = cc.Point(0,0)
			life = 4
			lifeVar = 1
			startSpin = 0
			startSizeVar = 0
			endSpin = 0
			endSpinVar= 0
			
			startSize = 80
			startSizeVar = 40
			endSize = -1
			blendAdditive = true

			emissionRate = totalParticles / life / 5
		}

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Flower()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("stars2.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)
		
		_emitter with
		{
			duration = -1
			gravity = cc.Point(0,0)
			angle = 90
			angleVar = 360
			speed = 160
			speedVar = 20
			radialAccel = -120
			radialAccelVar = 0
			tangentialAccel = 30
			tangentialAccelVar = 0
			posVar = cc.Point(0,0)
			life = 3
			lifeVar = 1
			startSpin = 0
			startSizeVar = 0
			endSpin = 0
			endSpinVar = 2000
			
			startSize = 30
			startSizeVar = 0
			endSize = -1 
			blendAdditive = false
		}

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Meteor()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Spiral()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Explosion()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("stars.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Smoke()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Snow()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("snow.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)
		
		_emitter with
		{
			life = 3
			lifeVar = 1
			gravity = cc.Point(0,-10)
			speed = 130
			speedVar = 30
			
			startColor = startColor with { r = 0.9; g = 0.9; b = 0.9 }
			startColorVar = startColorVar with { b = 0.1 }
			
			emissionRate = totalParticles / life
		}
		
		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Rain()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		_emitter.position = cc.Point(size.width / 3, size.height / 2)
		_emitter.life = 4

		_background.addChild(_emitter, 10)
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
		_emitter = cc.particle.Flower()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("stars.png"))
		_emitter.position = cc.Point(size.width / 2, size.height / 2)
		
		_emitter with
		{
			life = 10
			lifeVar = 0
			speed = 100
			speedVar = 0
			emissionRate = 10000
		}

		_background.addChild(_emitter, 10)
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
		
		_background.parent.removeChild(_background, true)
		_background = null
		
		var p = cc.ParallaxNode()
		addChild(p, 5)
		
		var p1 = cc.Sprite(pack.locate("background3.png"))
		var p2 = cc.Sprite(pack.locate("background3.png"))
		
		p.addChild(p1, 1, cc.Point(0.5, 1), cc.Point(0, 250))
		p.addChild(p2, 2, cc.Point(1.5, 1), cc.Point(0, 50))
		
		var size = cocos.director.winSize
		_emitter = cc.particle.Flower()
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		
		p1.addChild(_emitter, 10)
		_emitter.position = cc.Point(250,200)
		
		var par = cc.particle.Sun()
		p2.addChild(par, 10)
		par.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		
		var move = cc.action.MoveBy(4, cc.Point(300,0))
		var move_back = move.reverse()
		var seq = cc.action.Sequence(move, move_back)
		p.runAction(cc.action.RepeatForever(seq))
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
		_emitter = cc.ParticleSystemPoint(1000)
		_background.addChild(_emitter, 10)
			
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		
		_emitter with
		{
			duration = -1
			gravity = cc.Point(0,0)
			angle = 0
			angleVar = 360
			radialAccel = 70
			radialAccelVar = 10
			tangentialAccel = 80
			tangentialAccelVar = 0
			speed = 50
			speedVar = 10
			position = cc.Point(size.width / 2, size.height / 2)
			posVar = cc.Point(0,0)
			life = 2
			lifeVar = 0.3
			emissionRate = totalParticles / life

			startColor = startColor with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			startColorVar = startColorVar with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
			endColor = endColor with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
			endColorVar = endColorVar with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
			
			startSize = 1
			startSizeVar = 1
			endSize = 32 
			endSizeVar = 8 
			blendAdditive = false
		}
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
		
		removeChild(_background, true)
		_background = null
		
		var size = cocos.director.winSize
		_emitter = cc.ParticleSystemQuad(200)
		addChild(_emitter, 10)
		_emitter.texture = cocos.textureCache.addImage(pack.locate("stars-grayscale.png"))
		
		_emitter with
		{
			duration = -1
			emitterMode = 1
			startRadius = 0
			startRadiusVar = 0
			endRadius = 160
			endRadiusVar = 0
			
			rotatePerSecond = 180
			rotatePerSecondVar = 0
			
			angle = 90
			angleVar = 0

			startSpin = 0
			startSizeVar = 0
			endSpin = 0
			endSpinVar= 0
			
			position = cc.Point(size.width / 2, size.height / 2)
			posVar = cc.Point(0,0)
			life = 5
			lifeVar = 0
			emissionRate  = totalParticles / life
			
			startColor = startColor with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			startColorVar = startColorVar with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			endColor = endColor with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
			endColorVar = endColorVar with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
		
			startSize = 32
			startSizeVar = 0
			endSize = -1 
			
			blendAdditive = false
		}

		_colorLayer.color = nit.Color(0)
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
		
		removeChild(_background, true)
		_background = null
		
		var size = cocos.director.winSize
		_emitter = cc.ParticleSystemQuad(200)
		addChild(_emitter, 10)
			
		_emitter.texture = cocos.textureCache.addImage(pack.locate("stars-grayscale.png"))
		
		_emitter with
		{
			duration = -1
			emitterMode = 1
			startRadius = 100
			startRadiusVar = 0
			endRadius = -1
			endRadiusVar = 0
			rotatePerSecond = 45
			rotatePerSecondVar = 0
			angle = 90
			angleVar = 0
			position = cc.Point(size.width / 2, size.height / 2)
			posVar = cc.Point(0,0)
			life = 4
			lifeVar = 0
			startSpin = 0
			startSizeVar = 0
			endSpin = 0
			endSpinVar= 0
		
			startColor = startColor with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			startColorVar = startColorVar with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			endColor = endColor with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
			endColorVar = endColorVar with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
	
			emissionRate  = totalParticles / life
			
			startSize = 32
			startSizeVar = 0
			endSize = -1 
			blendAdditive = false
		}
		
		_colorLayer.color = nit.Color(0,0,0,0)
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
		
		removeChild(_background, true)
		_background = null
		
		var size = cocos.director.winSize
		_emitter = cc.ParticleSystemQuad(100)
		addChild(_emitter, 10)
			
		_emitter.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		
		_emitter with
		{
			duration = -1
			emitterMode = 1
			startRadius = 50
			startRadiusVar = 0
			endRadius = -1
			endRadiusVar = 0
			rotatePerSecond = 0
			rotatePerSecondVar = 0
			angle = 0
			angleVar = 0
			position = cc.Point(size.width / 2, size.height / 2)
			posVar = cc.Point(0,0)
			life = 5
			lifeVar = 0
			startSpin = 0
			startSizeVar = 0
			endSpin = 0
			endSpinVar= 0
		
			startColor = startColor with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			startColorVar = startColorVar with { r = 0.5; g = 0.5; b = 0.5; a = 1.0 }
			endColor = endColor with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
			endColorVar = endColorVar with { r = 0.1; g = 0.1; b = 0.1; a = 0.2 }
	
			emissionRate  = totalParticles / life
			
			startSize=16
			startSizeVar=0
			endSize=-1 
			blendAdditive = false
		}
		
		var rot = cc.action.RotateBy(16,360)
		_emitter.runAction(cc.action.RepeatForever(rot))
		
		_colorLayer.color = nit.Color(0,0,0,0)
	}
	
	function title()
	{
		return "Issue704. Free + Rot"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Issue870 : ParticleDemo
{
	_nIndex = 0
	constructor()
	{
		base.constructor()
		
		removeChild(_background, true)
		_background = null
		
		var size = cocos.director.winSize
		var system = cc.ParticleSystemQuad(pack.locate("SpinningPeas.plist"))
		system.setTextureWithRect(cocos.textureCache.addImage(pack.locate("particles.png")), cc.Rect(0,0,32,32))
		addChild(system, 10)
		_emitter = system
		
		_colorLayer.color = nit.Color(0,0,0,0)
		cocos.director.scheduler.repeat(this, updateQuads, 2.0)
	}
	
	function title()
	{
		return "Issue 870. SubRect"
	}
	
	function updateQuads(dt)
	{
		_nIndex = (_nIndex+1)%4
		var rect = cc.Rect(_nIndex*32, 0, 32, 32)
		var system = _emitter
		system.setTextureWithRect(_emitter.texture, rect)
	}
}

////////////////////////////////////////////////////////////////////////////////

class DemoParticleFromFile : ParticleDemo
{
	_title = null
	constructor(file)
	{
		_title = file
		base.constructor()
		
		removeChild(_background, true)
		_background = null
		
		var size = cocos.director.winSize
		var filename = file + ".plist"
		_emitter = cc.ParticleSystemQuad(pack.locate(filename))
		setEmitterPosition()
		addChild(_emitter, 10)
		
		_colorLayer.color = nit.Color(0,0,0,0)
	}
	
	function title()
	{
		return _title
	}
}

////////////////////////////////////////////////////////////////////////////////

class CocosParticleTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		addChild(pTest.restartParticleTest())
		cocos.director.replaceScene(this)
	}
}

return CocosParticleTestScene()