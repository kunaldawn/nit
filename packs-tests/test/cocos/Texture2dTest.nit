var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

class TextureTest
{
	sceneindex = -1
	scenemaxcnt = 30
	
	constructor()
	{
	}
	
	function _createTextureTest(index)
	{
		var layer = null
		
		switch (index)
		{
		case 0: return TextureAlias()
		case 1: return TextureMipMap()
		case 2: return TexturePVRMipMap()
		case 3: return TexturePVRMipMap2()
		case 4: return TexturePVRNonSquare()
		case 5: return TexturePVRNPOT4444()
		case 6: return TexturePVRNPOT8888()
		case 7: return TexturePVR2BPP()
		case 8: return TexturePVRRaw()
		case 9: return TexturePVR()
		case 10: return TexturePVR4BPP()
		case 11: return TexturePVRRGBA8888()
		case 12: return TexturePVRBGRA8888()
		case 13: return TexturePVRRGBA4444()
		case 14: return TexturePVRRGBA4444CCZ()
		case 15: return TexturePVRRGBA5551()
		case 16: return TexturePVRRGB565()
		case 17: return TexturePVRA8()
		case 18: return TexturePVRI8()
		case 19: return TexturePVRAI88()
		case 20: return TexturePVRBadEncoding()
		case 21: return TexturePNG()
		case 22: return TextureJPEG()
		case 23: return TextureGif ()
		case 24: return TexturePixelFormat()
		case 25: return TextureBlend()
		case 26: return TextureGlClamp()
		case 27: return TextureGlRepeat()
		case 28: return TextureSizeTest()
		case 29: return textureCache1()
		}
	}
	
	function nextTextureTest()
	{
		sceneindex++
		sceneindex = sceneindex % scenemaxcnt
		
		return _createTextureTest(sceneindex)
	}
	function backTextureTest()
	{
		sceneindex--
		if (sceneindex < 0)
			sceneindex = scenemaxcnt -1
			
		return _createTextureTest(sceneindex)
	}
	function restartTextureTest()
	{
		return _createTextureTest(sceneindex)
	}
}

var texTest = TextureTest()

////////////////////////////////////////////////////////////////////////////////

class TextureDemo : cc.ScriptLayer
{
	constructor()
	{
		base.constructor()
		
		cocos.textureCache.dumpCachedTextureInfo()
		var s = cocos.director.winSize
		
		var label = cc.LabelTTF(title(), "Arial", 26)
		addChild(label, 1, 1)
		label.position = cc.Point(s.width / 2, s.height - 50)
		
		var strSubtitle = subtitle()
		if (strSubtitle != "")
		{
			var l = cc.LabelTTF(strSubtitle, "Arial", 16)
			addChild(l, 1)
			l.position = cc.Point(s.width / 2, s.height - 80)
		}
		
		var item1 = cc.MenuItemImage(
			pack.locate("b1.png"),
			pack.locate("b2.png"),
			this, onBackMenu)
		var item2 = cc.MenuItemImage(
			pack.locate("r1.png"),
			pack.locate("r2.png"),
			this, onRestartMenu)
		var item3 = cc.MenuItemImage(
			pack.locate("f1.png"),
			pack.locate("f2.png"),
			this, onNextMenu)
			
		var menu = cc.Menu(item1, item2, item3)
		menu.position = cc.Point(0,0)
		item1.position = cc.Point(s.width / 2 - 100, 30)
		item2.position = cc.Point(s.width / 2, 30)
		item3.position = cc.Point(s.width / 2 + 100, 30)
		addChild(menu, 1)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}
	
	function title()
	{
		return "No title"
	}

	function subtitle()
	{
		return ""
	}
	
	function onBackMenu(evt: cc.MenyItemEvent)
	{
		var s =  TextureTestScene()
		s.addChild(texTest.backTextureTest())
		cocos.director.replaceScene(s)
	}

	function onRestartMenu(evt: cc.MenyItemEvent)
	{
		var s =  TextureTestScene()
		s.addChild(texTest.restartTextureTest())
		cocos.director.replaceScene(s)
	}

	function onNextMenu(evt: cc.MenyItemEvent)
	{
		var s =  TextureTestScene()
		s.addChild(texTest.nextTextureTest())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePNG : TextureDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image.png"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PNG Test"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TextureJPEG : TextureDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image.jpeg"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}
	
	function title()
	{
		return "JPEG Test"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TextureGif : TextureDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("Kraid_16-bit.gif"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}
	
	function title()
	{
		return "GIF Test"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TextureMipMap : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var texture0 = cocos.textureCache.addImage(
			pack.locate("grossini_dance_atlas.png"))
		texture0.generateMipmap()
		
		texture0.nitTexture with 
		{
			minFilter = MIN_FILTER.LINEAR_MIPMAP_LINEAR
			magFilter = MAG_FILTER.LINEAR
			wrapModeS = WRAP_MODE.CLAMP_TO_EDGE
			wrapModeT = WRAP_MODE.CLAMP_TO_EDGE
		}
		
		var texture1 = cocos.textureCache.addImage(
			pack.locate("grossini_dance_atlas_nomipmap.png"))
		
		var img0 = cc.Sprite(texture0)
		img0.textureRect = cc.Rect(85, 121, 85, 121)
		img0.position = cc.Point(s.width / 3, s.height / 2)
		addChild(img0)
		
		var img1 = cc.Sprite(texture1)
		img1.textureRect = cc.Rect(85, 121, 85, 121)
		img1.position = cc.Point(2 * s.width /3, s.height / 2)
		addChild(img1)
		
		var scale1 = cc.action.EaseOut(cc.action.ScaleBy(4, 0.01), 3)
		var sc_back = scale1.reverse()
		
		var scale2 = cc.action.EaseOut(cc.action.ScaleBy(4, 0.01), 3)
		var sc_back2 = scale2.reverse()
		
		img0.runAction(cc.action.RepeatForever(cc.action.Sequence(scale1, sc_back)))
		img1.runAction(cc.action.RepeatForever(cc.action.Sequence(scale2, sc_back2)))
			
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "Texture Mipmap"
	}
	
	function subtitle()
	{
		return "Left image uses mipmap. Right image doesn't"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRMipMap : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var imgMipMap = cc.Sprite(pack.locate("logo-mipmap.pvr"))
		if (imgMipMap)
		{
			imgMipMap.position = cc.Point(s.width / 2 - 100, s.height / 2)
			addChild(imgMipMap)

			imgMipMap.texture.nitTexture with 
			{
				minFilter = MIN_FILTER.LINEAR_MIPMAP_LINEAR
				magFilter = MAG_FILTER.LINEAR
				wrapModeS = WRAP_MODE.CLAMP_TO_EDGE
				wrapModeT = WRAP_MODE.CLAMP_TO_EDGE
			}
		}
	
		var img = cc.Sprite(pack.locate("logo-nomipmap.pvr"))
		if (img)
		{
			img.position = cc.Point(s.width / 2 + 100, s.height / 2)
			addChild(img)
			
			var scale1 = cc.action.EaseOut(cc.action.ScaleBy(4, 0.01), 3)
			var sc_back = scale1.reverse()
			
			var scale2 = cc.action.EaseOut(cc.action.ScaleBy(4, 0.01), 3)
			var sc_back2 = scale2.reverse()
			
			imgMipMap.runAction(cc.action.RepeatForever(cc.action.Sequence(scale1, sc_back)))
			img.runAction(cc.action.RepeatForever(cc.action.Sequence(scale2, sc_back2)))
		}
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVRTC MipMap Test"
	}
	
	function subtitle()
	{
		return "Left image uses mipmap. Right image doesn't"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRMipMap2 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var imgMipMap = cc.Sprite(pack.locate("test_image_rgba4444_mipmap.pvr"))
		imgMipMap.position = cc.Point(s.width / 2 - 100, s.height / 2)
		addChild(imgMipMap)
		
		imgMipMap.texture.nitTexture with
		{
			minFilter = MIN_FILTER.LINEAR_MIPMAP_LINEAR
			magFilter = MAG_FILTER.LINEAR
			wrapModeS = WRAP_MODE.CLAMP_TO_EDGE
			wrapModeT = WRAP_MODE.CLAMP_TO_EDGE
		}
	
		var img = cc.Sprite(pack.locate("test_image.png"))
	
		img.position = cc.Point(s.width / 2 + 100, s.height / 2)
		addChild(img)
		
		var scale1 = cc.action.EaseOut(cc.action.ScaleBy(4, 0.01), 3)
		var sc_back = scale1.reverse()
		
		var scale2 = cc.action.EaseOut(cc.action.ScaleBy(4, 0.01), 3)
		var sc_back2 = scale2.reverse()
		
		imgMipMap.runAction(cc.action.RepeatForever(cc.action.Sequence(scale1, sc_back)))
		img.runAction(cc.action.RepeatForever(cc.action.Sequence(scale2, sc_back2)))
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR MipMap Test #2"
	}
	
	function subtitle()
	{
		return "Left image uses mipmap. Right image doesn't"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVR2BPP : TextureDemo
{
	constructor()
	{
		base.constructor()	
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_pvrtc2bpp.pvr"))
		if (img)
		{
			img.position = cc.Point(s.width / 2, s.height / 2)
			addChild(img)
		}
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR TC 2bpp Test"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRRaw : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		// var tex = coocs.textureCache.AddPVRTCImage(pack.locate("test_image.pvrraw", 4, true, 128))
		// var img = cc.Sprite(tex)
		// img.position = cc.Point(s.width / 2, s.height / 2)
		// addChild(img)
		
		print("Not support PVRTC!");
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR TC 4bpp Test #1 (Raw)"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVR : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image.pvr"))
		if (img)
		{
			img.position = cc.Point(s.width / 2, s.height / 2)
			addChild(img)
		}
		else
		{
			print("This test is not supported.")
		}
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR TC 4bpp Test #2"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVR4BPP : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_pvrtc4bpp.pvr"))
		if (img)
		{
			img.position = cc.Point(s.width / 2, s.height / 2)
			addChild(img)
		}
		else
		{
			print("This test is not supported in cocos2d-mac")
		}
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR TC 4bpp Test #3"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRRGBA8888 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_rgba8888.pvr"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR + RGBA  8888 Test"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRBGRA8888 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_bgra8888.pvr"))
		if (img)
		{
			img.position = cc.Point(s.width / 2, s.height / 2)
			addChild(img)
		}
		else
		{
			print("BGRA8888 images are not supported")
		}
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR + BGRA 8888 Test"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRRGBA5551 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_rgba5551.pvr"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR + RGBA 5551 Test"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRRGBA4444 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_rgba4444.pvr"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR + RGBA 4444 Test"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRRGBA4444CCZ : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_rgba4444.pvr.ccz"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR + RGBA 4444 + CCZ Test"
	}
	
	function subtitle()
	{
		return "This is a ccz PVR image"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRRGB565 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_rgb565.pvr"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR + RGB 565 Test"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRA8 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_a8.pvr"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR + A8 Test"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRI8 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_i8.pvr"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR + I8 Test"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRAI88 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image_ai88.pvr"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR + AI88 Test"
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRBadEncoding : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("test_image-bad_encoding.pvr"))
		if (img)
		{
			img.position = cc.Point(s.width / 2, s.height / 2)
			addChild(img)
		}
	}

	function title()
	{
		return "PVR Unsupported encoding"
	}
	
	function subtitle()
	{
		return "You should not see any image"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRNonSquare : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("grossini_128x256_mipmap.pvr"))
		img.position = cc.Point(s.width / 2, s.height / 2)
		addChild(img)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR + Non square texture"
	}
	
	function subtitle()
	{
		return "Loading a 128x256 texture"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRNPOT4444 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("grossini_pvr_rgba4444.pvr"))
		if (img)
		{
			img.position = cc.Point(s.width / 2, s.height / 2)
			addChild(img)
		}
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR RGBA4 + NPOT texture"
	}
	
	function subtitle()
	{
		return "Loading a 81x121 RGBA4444 texture."
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePVRNPOT8888 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var img = cc.Sprite(pack.locate("grossini_pvr_rgba8888.pvr"))
		if (img)
		{
			img.position = cc.Point(s.width / 2, s.height / 2)
			addChild(img)
		}
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "PVR RGBA8 + NPOT texture"
	}
	
	function subtitle()
	{
		return "Loading a 81x121 RGBA8888 texture."
	}
}

////////////////////////////////////////////////////////////////////////////////

class TextureAlias : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		//
		// Sprite 1: GL_LINEAR
		//
		// Default filter is GL_LINEAR
		
		var sprite = cc.Sprite(pack.locate("grossinis_sister1.png"))
		sprite.position = cc.Point(s.width / 3, s.height / 2)
		addChild(sprite)
		
		// this is the default filterting
		sprite.texture.setAntiAliasTexParameters()
		
		//
		// Sprte 1: GL_NEAREST
		//
		
		var sprite2 = cc.Sprite(pack.locate("grossinis_sister2.png"))
		sprite2.position = cc.Point(2 * s.width / 3, s.height / 2)
		addChild(sprite2)
		
		// Use Nearest in this one
		sprite2.texture.setAliasTexParameters()
		
		// scale them to show
		var sc = cc.action.ScaleBy(3, 8)
		var sc_back = sc.reverse()
		var sc2 = cc.action.ScaleBy(3, 8)
		var sc_back2 = sc.reverse()
		var scaleforever = cc.action.RepeatForever(cc.action.Sequence(sc, sc_back))
		var scaleToo = cc.action.RepeatForever(cc.action.Sequence(sc2, sc_back2))
		
		sprite2.runAction(scaleforever)
		sprite.runAction(scaleToo)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "AntiAlias / Alias textures"
	}
	
	function subtitle()
	{
		return "Left image is antialiased. Right image is aliases"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TexturePixelFormat : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		// This example displays 1 png images 4 times.
		// Each time the image is generated using:
		// 1- 32-bit RGBA8
		// 2- 16-bit RGBA4
		// 3- 16-bit RGB5A1
		// 4- 16-bit RGB565
		
		var label = getChildByTag(1)
		label.color = nit.Color(16 / 255, 16 / 255, 1, 1)
		
		var background = cc.LayerColor(nit.Color(128/255, 128/255, 128/255, 255/255), s.width, s.height)
		addChild(background, -1)
		
		// RGBA 8888 image (32-bit)
		//print("DefaultAlphaPixelFormat :"+cc.Texture2D.DefaultAlphaPixelFormat())
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444)
		var sprite1 = cc.Sprite(pack.locate("test-rgba1.png"))
		sprite1.position = cc.Point(1 * s.width / 6, s.height / 2 + 32)
		addChild(sprite1, 0)
		
		// remove texture from texture manager
		cocos.textureCache.removeTexture(sprite1.texture)
		
		// RGB 4444 image (16-bit)
		//print("DefaultAlphaPixelFormat :"+cc.Texture2D.DefaultAlphaPixelFormat())
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGBA4444)
		var sprite2 = cc.Sprite(pack.locate("test-rgba1.png"))
		sprite2.position = cc.Point(2 * s.width / 6, s.height / 2 - 32)
		addChild(sprite2, 0)
		
		// remove texture from texture manager
		cocos.textureCache.removeTexture(sprite2.texture)
		
		// RGB5A1 image (16-bit)
		//print("DefaultAlphaPixelFormat :"+cc.Texture2D.DefaultAlphaPixelFormat())
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGB5A1)
		var sprite3 = cc.Sprite(pack.locate("test-rgba1.png"))
		sprite3.position = cc.Point(3 * s.width / 6, s.height / 2 + 32)
		addChild(sprite3, 0)
		
		// remove texture from texture manager
		cocos.textureCache.removeTexture(sprite3.texture)
		
		// RGB565 image (16-bit)
		//print("DefaultAlphaPixelFormat :"+cc.Texture2D.DefaultAlphaPixelFormat())
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_RGB565)
		var sprite4 = cc.Sprite(pack.locate("test-rgba1.png"))
		sprite4.position = cc.Point(4 * s.width / 6, s.height / 2 - 32)
		addChild(sprite4, 0)
		
		// remove texture from texture manager
		cocos.textureCache.removeTexture(sprite4.texture)
		
		// A8 image (8-bit)
		//print("DefaultAlphaPixelFormat :"+cc.Texture2D.DefaultAlphaPixelFormat())
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_A8)
		var sprite5 = cc.Sprite(pack.locate("test-rgba1.png"))
		sprite5.position = cc.Point(5 * s.width / 6, s.height / 2 + 32)
		addChild(sprite5, 0)
		
		// remove texture from texture manager
		cocos.textureCache.removeTexture(sprite5.texture)
		
		var fadeout = cc.action.FadeOut(2)
		var fadein = cc.action.FadeIn(2)
		var fadeout2 = cc.action.FadeOut(2)
		var fadein2 = cc.action.FadeIn(2)
		var fadeout3 = cc.action.FadeOut(2)
		var fadein3 = cc.action.FadeIn(2)
		var fadeout4 = cc.action.FadeOut(2)
		var fadein4 = cc.action.FadeIn(2)
		var fadeout5 = cc.action.FadeOut(2)
		var fadein5 = cc.action.FadeIn(2)

		var seq_4ever = cc.action.RepeatForever(cc.action.Sequence(cc.action.DelayTime(2), fadeout, fadein))
		var seq_4ever2 = cc.action.RepeatForever(cc.action.RepeatForever(cc.action.Sequence(cc.action.DelayTime(2), fadeout2, fadein2)))
		var seq_4ever3 = cc.action.RepeatForever(cc.action.RepeatForever(cc.action.Sequence(cc.action.DelayTime(2), fadeout3, fadein3)))
		var seq_4ever4 = cc.action.RepeatForever(cc.action.RepeatForever(cc.action.Sequence(cc.action.DelayTime(2), fadeout4, fadein4)))
		var seq_4ever5 = cc.action.RepeatForever(cc.action.RepeatForever(cc.action.Sequence(cc.action.DelayTime(2), fadeout5, fadein5)))
		
		sprite1.runAction(seq_4ever)
		sprite2.runAction(seq_4ever2)
		sprite3.runAction(seq_4ever3)
		sprite4.runAction(seq_4ever4)
		sprite5.runAction(seq_4ever5)
		
		cc.Texture2D.setDefaultAlphaPixelFormat(cc.Texture2D.FORMAT_DEFAULT)
		
		cocos.textureCache.dumpCachedTextureInfo()
	}

	function title()
	{
		return "Texture Pixel Formats"
	}
	
	function subtitle()
	{
		return "Textures: RGBA8888, RGBA4444, RGB5A1, RGB565, A8"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TextureBlend : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		for (var i = 0; i < 15; i++)
		{
			// BOTTOM sprites have alpha pre-multiplied
			// they use by default GL_ONE, GL_ONE_MINUS_SRC_ALPHA
			var cloud = cc.Sprite(pack.locate("test_blend.png"))
			addChild(cloud, i + 1, 100 + i)
			cloud.position = cc.Point(50 + 25 * i, 80)
			cloud.blendFuncSrc = cloud.SRC_BLEND_ONE
			cloud.blendFuncDst = cloud.DST_BLEND_ONE_MINUS_SRC_ALPHA
			
			// CENTER sprites have also alpha pre-multiplied
			// they use by default GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
			cloud = cc.Sprite(pack.locate("test_blend.png"))
			addChild(cloud, i + 1, 200 + i)
			cloud.position = cc.Point(50 + 25 * i, 160)
			cloud.blendFuncSrc = cloud.SRC_BLEND_ONE_MINUS_DST_COLOR
			cloud.blendFuncDst = cloud.DST_BLEND_ZERO
			
			// UPPER sprites are using custom blending function
			// You can set any blend function to your sprites
			cloud = cc.Sprite(pack.locate("test_blend.png"))
			addChild(cloud, i + 1, 200 + i)
			cloud.position = cc.Point(50 + 25 * i, 320 - 80)
			cloud.blendFuncSrc = cloud.SRC_BLEND_SRC_ALPHA
			cloud.blendFuncDst = cloud.DST_BLEND_ONE
		}
	}

	function title()
	{
		return "Texture Blending"
	}
	
	function subtitle()
	{
		return "Testing 3 different blending modes"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TextureGlClamp : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		// The .png image MUST be power of 2 in order to create a continue effect.
		// eg: 32x64, 512x128, 256x1024, 64x64, etc..
		var sprite = cc.Sprite(pack.locate("pattern1.png"), cc.Rect(0,0,512,256))
		addChild(sprite, -1, 2)
		sprite.position = cc.Point(s.width / 2, s.height / 2)
		
		sprite.texture.nitTexture with
		{
			minFilter = MIN_FILTER.LINEAR
			magFilter = MAG_FILTER.LINEAR
			wrapModeS = WRAP_MODE.CLAMP_TO_EDGE
			wrapModeT = WRAP_MODE.CLAMP_TO_EDGE
		}
		
		var rotate = cc.action.RotateBy(4, 360)
		sprite.runAction(rotate)
		var scale = cc.action.ScaleBy(2, 0.04)
		var scaleBack = scale.reverse()
		var seq = cc.action.Sequence(scale,scaleBack)
		sprite.runAction(seq)
	}

	function title()
	{
		return "Texture GL_CLAMP"
	}

	destructor()
	{
		cocos.textureCache.removeUnusedTextures()
	}
	
}

////////////////////////////////////////////////////////////////////////////////

class TextureGlRepeat : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		// The .png image MUST be power of 2 in order to create a continue effect.
		// eg: 32x64, 512x128, 256x1024, 64x64, etc..
		var sprite = cc.Sprite(pack.locate("pattern1.png"), cc.Rect(0,0,4096,4096))
		addChild(sprite, -1, 2)
		sprite.position = cc.Point(s.width / 2, s.height / 2)

		sprite.texture.nitTexture with
		{
			minFilter = MIN_FILTER.LINEAR
			magFilter = MAG_FILTER.LINEAR
			wrapModeS = WRAP_MODE.REPEAT
			wrapModeT = WRAP_MODE.REPEAT
		}
				
		var rotate = cc.action.RotateBy(4, 360)
		sprite.runAction(rotate)
		var scale = cc.action.ScaleBy(2, 0.04)
		var scaleBack = scale.reverse()
		var seq = cc.action.Sequence(scale,scaleBack)
		sprite.runAction(seq)
	}

	function title()
	{
		return "Texture GL_REPEAT"
	}

	destructor()
	{
		cocos.textureCache.removeUnusedTextures()
	}
}

////////////////////////////////////////////////////////////////////////////////

class TextureSizeTest : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var sprite = null
		
		print("Loading 512x512 image...")
		sprite = cc.Sprite(pack.locate("texture512x512.png"))
		if (sprite)
			print("OK")
		else
			print("Error")
			
		print("Loading 1024x1024 image...")
		sprite = cc.Sprite(pack.locate("texture1024x1024.png"))
		if (sprite)
			print("OK")
		else
			print("Error")
		
	}

	function title()
	{
		return "Different Texture Sizes"
	}

	function subtitle()
	{
		return "512x512, 1024x1024. See the console."
	}
}

////////////////////////////////////////////////////////////////////////////////

class textureCache1 : TextureDemo
{
	constructor()
	{
		base.constructor()
	}

	function onEnter()
	{
		var s = cocos.director.winSize
		
		var sprite = null
		
		sprite = cc.Sprite(pack.locate("grossinis_sister1.png"))
		sprite.position = cc.Point(s.width / 5 * 1, s.height / 2)
		sprite.texture.setAliasTexParameters()
		sprite.scale(2)
		addChild(sprite)
		
		cocos.textureCache.removeTexture(sprite.texture)
		
		sprite = cc.Sprite(pack.locate("grossinis_sister1.png"))
		sprite.position = cc.Point(s.width / 5 * 2, s.height / 2)
		sprite.texture.setAntiAliasTexParameters()
		sprite.scale(2)
		addChild(sprite)
		
		// 2nd set of sprites
		
		sprite = cc.Sprite(pack.locate("grossinis_sister2.png"))
		sprite.position = cc.Point(s.width / 5 * 3, s.height / 2)
		sprite.texture.setAliasTexParameters()
		sprite.scale(2)
		addChild(sprite)
		
		cocos.textureCache.removeTexture(sprite.texture)
		
		sprite = cc.Sprite(pack.locate("grossinis_sister1.png"))
		sprite.position = cc.Point(s.width / 5 * 4, s.height / 2)
		sprite.texture.setAntiAliasTexParameters()
		sprite.scale(2)
		addChild(sprite)
	}

	function title()
	{
		return "CCtextureCache: remove"
	}

	function subtitle()
	{
		return "4 images should appear: alias, antialias, alias, antilias"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TextureTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var layer = texTest.nextTextureTest()
		addChild(layer)
		cocos.director.replaceScene(this)
	}
}

return TextureTestScene()