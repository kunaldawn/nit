var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var TAG =
{
	TAG.INFO_LAYER 		= 1
	TAG.MAIN_LAYER 		= 2
	TAG.PARTILE_SYSTEM 	= 3
	TAG.LABEL_ATLAS 	= 4
	TAG.MENU_LAYER 		= 1000
}

var TEST_COUNT			= 4
	
var MAX_PARTICLES 		= 14000
var NODES_INCREASE 		= 100
	
var curIdx 				= 0

var function runParticleTest()
{
    var pScene = ParticlePerformTest1()
    pScene.initWithSubTest(1, NODES_INCREASE);
	cocos.director.replaceScene(pScene);
}

////////////////////////////////////////////////////////////////////////////////

class ParticleMenuLayer : PerformBasicLayer
{	
	constructor(bControlMenuVisible, nMaxCases, nCurCase)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
	}
	
	function showCurrentTest()
	{
		var pScene = parent
		var SubTest = pScene.getSubTestNum()
		var parNum = pScene.getParticlesNum()
		
		var pNewScene = null
		
		switch (_curCase)
		{
			case 0:
				pNewScene = ParticlePerformTest1()
				break;
			case 1:
				pNewScene = ParticlePerformTest2()
				break;
			case 2:
				pNewScene = ParticlePerformTest3()
				break;
			case 3:
				pNewScene = ParticlePerformTest4()
				break;
		}
		curIdx = _curCase

		if (pNewScene)
		{
			pNewScene.initWithSubTest(SubTest, parNum)
			cocos.director.replaceScene(pNewScene)
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class ParticleMainScene : cc.ScriptScene
{
	var _lastRenderedCount = null
    var _quantityParticles = null
    var _subtestNumber = null
	
	function getSubTestNum()
	{
		return _subtestNumber;
	}
	function getParticlesNum()
	{
		return _quantityParticles;
	}
	
	function initWithSubTest(asubtest, particles)
	{
		_subtestNumber = asubtest;		
		var s = cocos.director.winSize
		
		_lastRenderedCount = 0
		_quantityParticles   = particles
		
		cc.MenuItemFont.setDefaultFontSize(65)
		var decrease = cc.MenuItemFont("-", this, onDecrease)
		decrease.color = nit.Color(0, 200/255, 20/255, 1)
		var increase = cc.MenuItemFont("+", this, onIncrease)
		increase.color = nit.Color(0, 200/255, 20/255, 1)
		
		var menu = cc.Menu(decrease, increase)
		menu.alignItemsHorizontally()
		menu.position = cc.Point(s.width/2, s.height/2+15)
		addChild(menu, 1)
		
		var infoLabel = cc.LabelTTF("0 nodes", "Arial", 30)
		infoLabel.color = nit.Color(0, 200/255, 20/255, 0)
		infoLabel.position = cc.Point(s.width/2, s.height - 90)
		addChild(infoLabel, 1, TAG.INFO_LAYER)
		
	    // particles on stage
		var labelAtlas = cc.LabelAtlas("0000", pack.locate("fps_images.png"), 16, 24, char('.'))
		addChild(labelAtlas, 0, TAG.LABEL_ATLAS)
		labelAtlas.position = cc.Point(s.width - 80, 50)
		
		//Next Prev Test
		var pMenu = ParticleMenuLayer(true, TEST_COUNT, curIdx)
		addChild(pMenu, 1, TAG.MENU_LAYER)
		
		//Sub Tests
		cc.MenuItemFont.setDefaultFontSize(40)
		var pSubMenu = cc.Menu()
		for (var i = 1; i <= 6; i++)
		{
			var str = ""+i
			var itemFont = null
			itemFont = cc.MenuItemFont(str, this, @testNCallback(itemFont))
			itemFont.tag = i
			pSubMenu.addChild(itemFont, 10)
			
			if (i<=3)
				itemFont.color = nit.Color(200/255, 20/255, 20/255, 1)
			else
				itemFont.color = nit.Color(0, 200/255, 20/255, 1)
		}
		pSubMenu.alignItemsHorizontally()
		pSubMenu.position = cc.Point(s.width/2, 80)
		addChild(pSubMenu, 2)
		
		//Add title label
		var label = cc.LabelTTF(title(), "Arial", 40)
		addChild(label, 1)
		label.position = cc.Point(s.width/2, s.height- 32)
		label.color = nit.Color(255/255, 255/255, 40/255, 0)
		
		updateQuantityLabel()
		createParticleSystem()

		cocos.director.scheduler.repeat(this, step, 0.05)
	}
	
	function title()
	{
		return "No title"
	}
	
	function step(dt)
	{
		var atlas = getChildByTag(TAG.LABEL_ATLAS)
		var emitter = getChildByTag(TAG.PARTILE_SYSTEM)
		
		var str = ""
		var count = emitter.particleCount
		if (count < 10)
			str = "0000" + count
		else if (count < 100) 
			str = "000" + count
		else if (count < 1000) 
			str = "00" + count
		else if (count < 10000) 
			str = "0" + count
		else 
			str = ""+ count
		atlas.string = str
	}
	
	function createParticleSystem()
	{
		var particleSystem = null
		 /*
		* Tests:
		* 1: Point Particle System using 32-bit textures (PNG)
		* 2: Point Particle System using 16-bit textures (PNG)
		* 3: Point Particle System using 8-bit textures (PNG)
		* 4: Point Particle System using 4-bit textures (PVRTC)

		* 5: Quad Particle System using 32-bit textures (PNG)
		* 6: Quad Particle System using 16-bit textures (PNG)
		* 7: Quad Particle System using 8-bit textures (PNG)
		* 8: Quad Particle System using 4-bit textures (PVRTC)
		*/
		removeChildByTag(TAG.PARTILE_SYSTEM, true)
		// remove the "fire.png" from the textureCache cache. 
		var texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		cocos.textureCache.removeTexture(texture)
		
		if (_subtestNumber <= 3)
		{
			particleSystem = cc.ParticleSystemPoint(_quantityParticles)
		}
		else
		{
			particleSystem = cc.ParticleSystemQuad(_quantityParticles)
		}
		
		switch (_subtestNumber)
		{
			case 1:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
				particleSystem.texture=cocos.textureCache.addImage(pack.locate("fire.png"))
				break;
			case 2:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444)
				particleSystem.texture=cocos.textureCache.addImage(pack.locate("fire.png"))
				break;
			case 3:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_A8)
				particleSystem.texture=cocos.textureCache.addImage(pack.locate("fire.png"))
				break;
				
			case 4:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
				particleSystem.texture=cocos.textureCache.addImage(pack.locate("fire.png"))
				break;
			case 5:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444)
				particleSystem.texture=cocos.textureCache.addImage(pack.locate("fire.png"))
				break;
			case 6:
				cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_A8)
				particleSystem.texture=cocos.textureCache.addImage(pack.locate("fire.png"))
				break;
			default:
				particleSystem = null
				print("** Shall not happne!")
				break;
		}
		addChild(particleSystem, 0, TAG.PARTILE_SYSTEM)
		
		doTest()
		// restore the default pixel format
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
	}
	
	function testNCallback(pSender)
	{
		_subtestNumber = pSender.tag
		var pMenu = getChildByTag(TAG.MENU_LAYER)
		pMenu.restartCallback(pSender)
	}
	
	function onIncrease(pSender)
	{
		_quantityParticles += NODES_INCREASE
		if (_quantityParticles > MAX_PARTICLES)
			_quantityParticles = MAX_PARTICLES

		updateQuantityLabel()
		createParticleSystem()
	}

	function onDecrease(pSender)
	{
		_quantityParticles -= NODES_INCREASE
		if (_quantityParticles < 0)
			_quantityParticles = 0

		updateQuantityLabel()
		createParticleSystem()
	}
	
	function updateQuantityLabel()
	{
		if (_quantityParticles != _lastRenderedCount)
		{
			var infoLabel = getChildByTag(TAG.INFO_LAYER)
			var str = "" + _quantityParticles+ " particles"
			infoLabel.string = str

			_lastRenderedCount = _quantityParticles;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class ParticlePerformTest1 : ParticleMainScene
{
	function title()
	{
		var str = "A ("+_subtestNumber+") Size=4"
		return str;
	}

	function doTest()
	{
		var s = cocos.director.winSize
		var particleSystem = getChildByTag(TAG.PARTILE_SYSTEM)
		
		particleSystem with
		{
			// duration
			duration = -1

			// gravity
			gravity = cc.Point(0,-90)

			// angle
			angle = 90
			angleVar = 0

			// radial
			radialAccel = 0
			radialAccelVar = 0

			// speed of particles
			speed = 180
			speedVar = 50

			// emitter position
			position = cc.Point(s.width/2, 100)
			posVar = cc.Point(s.width/2,0)

			// life of particles
			life = 2.0
			lifeVar = 1

			// emits per frame
			emissionRate = particleSystem.totalParticles/particleSystem.life

			// color of particles
			startColor = nit.Color(0.5, 0.5, 0.5, 1.0)
			startColorVar = nit.Color(0.5, 0.5, 0.5, 1.0)
			endColor = nit.Color(0.1, 0.1, 0.1, 0.2)
			endColorVar = nit.Color(0.1, 0.1, 0.1, 0.2)	

			// size, in pixels
			endSize = 4.0
			startSize = 4.0
			endSizeVar = 0
			startSizeVar = 0

			// additive
			blendAdditive = false
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class ParticlePerformTest2 : ParticleMainScene
{
	function title()
	{
		var str = "B ("+_subtestNumber+") Size=8"
		return str;
	}

	function doTest()
	{
		var s = cocos.director.winSize
		var particleSystem = getChildByTag(TAG.PARTILE_SYSTEM)
		
		particleSystem with
		{
			// duration
			duration = -1

			// gravity
			gravity = cc.Point(0,-90)

			// angle
			angle = 90
			angleVar = 0

			// radial
			radialAccel = 0
			radialAccelVar = 0

			// speed of particles
			speed = 180
			speedVar = 50

			// emitter position
			position = cc.Point(s.width/2, 100)
			posVar = cc.Point(s.width/2,0)

			// life of particles
			life = 2.0
			lifeVar = 1

			// emits per frame
			emissionRate = particleSystem.totalParticles/particleSystem.life

			// color of particles
			startColor = nit.Color(0.5, 0.5, 0.5, 1.0)
			startColorVar = nit.Color(0.5, 0.5, 0.5, 1.0)
			endColor = nit.Color(0.1, 0.1, 0.1, 0.2)
			endColorVar = nit.Color(0.1, 0.1, 0.1, 0.2)	

			// size, in pixels
			endSize = 8.0
			startSize = 8.0
			endSizeVar = 0
			startSizeVar = 0

			// additive
			blendAdditive = false
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class ParticlePerformTest3 : ParticleMainScene
{
	function title()
	{
		var str = "C ("+_subtestNumber+") Size=32"
		return str;
	}

	function doTest()
	{
		var s = cocos.director.winSize
		var particleSystem = getChildByTag(TAG.PARTILE_SYSTEM)
		
		particleSystem with
		{
			// duration
			duration = -1

			// gravity
			gravity = cc.Point(0,-90)

			// angle
			angle = 90
			angleVar = 0

			// radial
			radialAccel = 0
			radialAccelVar = 0

			// speed of particles
			speed = 180
			speedVar = 50

			// emitter position
			position = cc.Point(s.width/2, 100)
			posVar = cc.Point(s.width/2,0)

			// life of particles
			life = 2.0
			lifeVar = 1

			// emits per frame
			emissionRate = totalParticles / life

			// color of particles
			startColor = nit.Color(0.5, 0.5, 0.5, 1.0)
			startColorVar = nit.Color(0.5, 0.5, 0.5, 1.0)
			endColor = nit.Color(0.1, 0.1, 0.1, 0.2)
			endColorVar = nit.Color(0.1, 0.1, 0.1, 0.2)	

			// size, in pixels
			endSize = 32.0
			startSize = 32.0
			endSizeVar = 0
			startSizeVar = 0

			// additive
			blendAdditive = false
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class ParticlePerformTest4 : ParticleMainScene
{
	function title()
	{
		var str = "D ("+_subtestNumber+") Size=64"
		return str;
	}

	function doTest()
	{
		var s = cocos.director.winSize
		var particleSystem = getChildByTag(TAG.PARTILE_SYSTEM)
		
		particleSystem with
		{
			// duration
			duration = -1

			// gravity
			gravity = cc.Point(0,-90)

			// angle
			angle = 90
			angleVar = 0

			// radial
			radialAccel = 0
			radialAccelVar = 0

			// speed of particles
			speed = 180
			speedVar = 50

			// emitter position
			position = cc.Point(s.width/2, 100)
			posVar = cc.Point(s.width/2,0)

			// life of particles
			life = 2.0
			lifeVar = 1

			// emits per frame
			emissionRate = totalParticles / life

			// color of particles
			startColor = nit.Color(0.5, 0.5, 0.5, 1.0)
			startColorVar = nit.Color(0.5, 0.5, 0.5, 1.0)
			endColor = nit.Color(0.1, 0.1, 0.1, 0.2)
			endColorVar = nit.Color(0.1, 0.1, 0.1, 0.2)	

			// size, in pixels
			endSize = 64.0
			startSize = 64.0
			endSizeVar = 0
			startSizeVar = 0

			// additive
			blendAdditive = false
		}
	}
}
