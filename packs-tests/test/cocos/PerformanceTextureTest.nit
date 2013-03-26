var pack = script.locator

TEST_COUNT := 1
s_nTexCurCase := 0

function calculateDeltaTime( lastUpdate)
{
	var now = system.clock()
	var dt = now - lastUpdate
	return dt
}

function runTextureTest()
{
    s_nTexCurCase = 0;
    var pScene = TextureTest.newScene()
	cocos.director.replaceScene(pScene);
}
////////////////////////////////////////////////////////
//
// TextureMenuLayer
//
////////////////////////////////////////////////////////

class TextureMenuLayer : PerformBasicLayer
{

	constructor(bControlMenuVisible, nMaxCases, nCurCase)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
	}
	
	function showCurrentTest()
	{
		var pScene = null
		switch(m_nCurCase)
		{
			 case 0:
				pScene = TextureTest.scene()
			break;
		}
		s_nTexCurCase = m_nCurCase;
		
		if (pScene)
		{
			cocos.director.replaceScene(pScene)
		}
	}
	
	function onEnter()
	{
		base.onEnter()
		
		var s = cocos.director.winSize
		// title
		var label = cc.LabelTTF(title(), "Arial", 32)
		addChild(label, 1)
		label.position = cc.Point(s.width/2, s.height - 32)
		label.color = nit.Color(255/255, 255/255, 40/255, 0)
		
		// Subtitle
		var strSubTitle = subtitle()
		if(strSubTitle.len()!=0)
		{
			var l = cc.LabelTTF(strSubTitle, "Thonburi", 16);
			addChild(l, 1);
			l.position=cc.Point(s.width/2, s.height-80)
		}
		performTests();
	}
	
	function title()
	{
		return "No title"
	}
	
	function subtitle()
	{
		return "no subtitle"
	}
}
////////////////////////////////////////////////////////
//
// TouchesPerformTest1
//
////////////////////////////////////////////////////////
class TextureTest : TextureMenuLayer
{

	constructor(bControlMenuVisible, nMaxCases = 0, nCurCase = 0)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
	}
	
	function onEnter()
	{
		base.onEnter()
		
		
	}
	
	function performTestsPNG( filename)
	{
		var now = system.clock()
		var texture = null
		
		print("++ RGBA 8888")
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA8888)
		now = system.clock()
		texture = cocos.textureCache.addImage(pack.locate(filename))
		if (texture)
			print("   ms: "+calculateDeltaTime(now) )
		else
			print("*** ERROR")
		cocos.textureCache.removeTexture(texture)
		
		
		print("++ RGBA 4444")
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444)
		now = system.clock()
		texture = cocos.textureCache.addImage(pack.locate(filename))
		if (texture)
			print("   ms: "+calculateDeltaTime(now) )
		else
			print("*** ERROR")
		cocos.textureCache.removeTexture(texture)
		
		
		print("++ RGBA 5551")
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGB5A1)
		now = system.clock()
		texture = cocos.textureCache.addImage(pack.locate(filename))
		if (texture)
			print("   ms: "+calculateDeltaTime(now) )
		else
			print("*** ERROR")
		cocos.textureCache.removeTexture(texture)
		
		
		print("++ RGBA 565")
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGB565)
		now = system.clock()
		texture = cocos.textureCache.addImage(pack.locate(filename))
		if (texture)
			print("   ms: "+calculateDeltaTime(now) )
		else
			print("*** ERROR")
		cocos.textureCache.removeTexture(texture)
	}
	
	function performTests()
	{
		print("\n\n--------\n\n")
		print("--- PNG 128x128 ---\n")
		performTestsPNG("test_image.png")
		
		print("\n--- PNG 512x512 ---\n")
		performTestsPNG("texture512x512.png")
		
		print("\n\nEMPTY IMAGE\n\n")
		print("--- PNG 1024x1024 ---\n")
		performTestsPNG("texture1024x1024.png")
		
		print("\n\nSPRITESHEET IMAGE\n\n")
		print("--- PNG 1024x1024 ---\n")
		performTestsPNG("PlanetCute-1024x1024.png")
		
		print("\n\nLANDSCAPE IMAGE\n\n")
		print("--- PNG 1024x1024 ---\n")
		performTestsPNG("landscape-1024x1024.png")
	}
	
	function title()
	{
		return "Texture Performance Test";
	}
	
	function subtitle()
	{
		return "See console for results";
	}
	
	function newScene()
	{
		var pScene = cc.Scene()
		var layer = TextureTest(false, TEST_COUNT, s_nTexCurCase)
		pScene.addChild(layer)
		
		return pScene
	}
}