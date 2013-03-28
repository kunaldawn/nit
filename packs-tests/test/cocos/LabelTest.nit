var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

class LabelTest
{
	_sceneIndex = -1
	_sceneMax = 17
	
	constructor()
	{
	}
	
	function _createAtlasTest(index)
	{
		var layer = null
		
		switch (index)
		{
		case 0:	return LabelAtlasTest()
		case 1: return LabelAtlasColorTest()
		case 2: return Atlas3()
		case 3: return Atlas4()
		case 4: return Atlas5()
		case 5: return Atlas6()
		case 6: return AtlasBitmapColor()
		case 7: return AtlasFastBitmap()
		case 8: return BitmapFontMultiLine()
		case 9: return LabelsEmpty()
		case 10: return LabelBMFontHD()
		case 11: return LabelAtlasHD()
		case 12: return LabelGlyphDesigner()
		case 13: return Atlas1()
		case 14: return LabelTTFTest()
		case 15: return LabelTTFMultiline()
		case 16: return LabelTTFInternational()
		}
	}
	
	function nextAtlasTest()
	{
		_sceneIndex++
		_sceneIndex = _sceneIndex % _sceneMax
		
		return _createAtlasTest(_sceneIndex)
	}
	
	function backAtlasTest()
	{
		_sceneIndex--
		if (_sceneIndex < 0)
			_sceneIndex = _sceneMax -1
			
		return _createAtlasTest(_sceneIndex)
	}
	
	function restartAtlasTest()
	{
		return _createAtlasTest(_sceneIndex)
	}
}

var labeltest = LabelTest()

////////////////////////////////////////////////////////////////////////////////

class AtlasDemo : cc.ScriptLayer
{
	var _textureAtlas = null
	var _time = 0 
	
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var label = cc.LabelTTF(title(), "Arial", 28)
		this.addChild(label, 1)
		label.position = cc.Point(s.width / 2, s.height - 50)
		
		var strSubtitle = subtitle()
		if (strSubtitle != "")
		{
			var l = cc.LabelTTF(strSubtitle, "Arial", 16)
			this.addChild(l, 1)
			l.position = cc.Point(s.width / 2, s.height - 80)
		}
		
		var item1 = cc.MenuItemImage(
			IMG.B1,
			IMG.B2,
			this, onBackMenu)
		var item2 = cc.MenuItemImage(
			IMG.R1,
			IMG.R2,
			this, onRestartMenu)
		var item3 = cc.MenuItemImage(
			IMG.F1,
			IMG.F2,
			this, onNextMenu)
			
		var menu = cc.Menu(item1, item2, item3)
		menu.position = cc.Point(0,0)
		item1.position = cc.Point(s.width / 2 - 100, 30)
		item2.position = cc.Point(s.width / 2, 30)
		item3.position = cc.Point(s.width / 2 + 100, 30)
		this.addChild(menu, 1)
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
		var s =  AtlasTestScene()
		s.addChild(labeltest.backAtlasTest())
		cocos.director.replaceScene(s)
	}
	
	function onRestartMenu(evt: cc.MenyItemEvent)
	{
		var s =  AtlasTestScene()
		s.addChild(labeltest.restartAtlasTest())
		cocos.director.replaceScene(s)
	}
	
	function onNextMenu(evt: cc.MenyItemEvent)
	{
		var s =  AtlasTestScene()
		s.addChild(labeltest.nextAtlasTest())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class Atlas1 : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		// TODO: not implemented
		return
		
		_textureAtlas = cc.TextureAtlas(IMG.ATLAS_TEST, 3)
		
		var s = cocos.director.winSize()
		
		//
		// Notice: u,v tex coordinates are inverted
		//
	}

	function title()
	{
		return "CCTextureAtlas"
	}

	function subtitle()
	{
		return "Manual creation of CCTextureAtlas"
	}
}

////////////////////////////////////////////////////////////////////////////////

class LabelAtlasTest : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		_time = 0
		
		var label1 = cc.LabelAtlas("123 Test", pack.locate("tuffy_bold_italic-charmap.png"), 48, 64, $' ')
		this.addChild(label1, 0, 0)
		label1.position = cc.Point(10, 100)
		label1.opacity = 200
		
		var label2 = cc.LabelAtlas("0123456789", pack.locate("tuffy_bold_italic-charmap.png"), 48, 64, $' ')
		this.addChild(label2, 0, 1)
		label2.position = cc.Point(10, 200)
		label2.opacity = 32
		
		cocos.director.timer.channel().bind(Events.OnTick, this, step)
	}
	
	function step(evt: TimeEvent)
	{
		_time += evt.delta
		var label1 = this.getChildByTag(0)
		var str = format("%.3f", _time)

		label1.string = str + " Test"
		
		var label2 = this.getChildByTag(1)
		label2.string = "" + _time.tointeger()
	}
	
	function title()
	{
		return "LabelAtlas"
	}
	
	function subtitle()
	{
		return "Updating label should be fast"
	}
}

////////////////////////////////////////////////////////////////////////////////

class LabelAtlasColorTest : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		_time = 0
		
		var label1 = cc.LabelAtlas("123 Test", pack.locate("tuffy_bold_italic-charmap.png"), 48, 64, $' ')
		this.addChild(label1, 0, 0)
		label1.position = cc.Point(10, 100)
		label1.opacity = 200
		
		var label2 = cc.LabelAtlas("0123456789", pack.locate("tuffy_bold_italic-charmap.png"), 48, 64, $' ')
		this.addChild(label2, 0, 1)
		label2.position = cc.Point(10, 200)
		label2.color = nit.Color.RED
		
		var fade = cc.action.FadeOut(1)
		var fade_in = fade.reverse()
		var seq = cc.action.Sequence(fade, fade_in)
		var repeat = cc.action.RepeatForever(seq)
		label2.runAction(repeat)
		
		cocos.director.timer.channel().bind(Events.OnTick, this, step)
	}
	
	function step(evt: TimeEvent)
	{
		_time += evt.delta
		var label1 = this.getChildByTag(0)

		var str = format("%.3f", _time)

		label1.string = str + " Test"
		
		var label2 = this.getChildByTag(1)
		label2.string = "" + _time.tointeger()
	}
	
	function title()
	{
		return "CCLabelAtlas"
	}
	
	function subtitle()
	{
		return "opacity + Color should work at the same time"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Atlas3 : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		_time = 0
		var col = cc.LayerColor(nit.Color(128 / 255, 128 / 255, 128 / 255, 1))
		this.addChild(col, -10)
		
		var label1 = cc.LabelBMFont("Test", pack.locate("bitmapFontTest2.fnt"))
		
		// testing anchors
		label1.anchorPoint = cc.Point(0,0)
		addChild(label1, 0, 1)
		var fade = cc.action.FadeOut(1)
		var fade_in = fade.reverse()
		var seq = cc.action.Sequence(fade, fade_in)
		var repeat = cc.action.RepeatForever(seq)
		label1.runAction(repeat)
		
		// VERY IMPORTANT
		// color and opacity work OK because bitmapFontAltas2 loads a BMP image (not a PNG image)
		// If you want to use both opacity and color, it is recommended to use NON premultiplied images like BMP images
		// Of course, you can also tell XCode not to compress PNG images, but I think it doesn't work as expected
		var label2 = cc.LabelBMFont("Test", pack.locate("bitmapFontTest2.fnt"))
		// testing anchors
		label2.anchorPoint = cc.Point(0.5, 0.5)
		label2.color = nit.Color.RED
		this.addChild(label2, 0, 2)
		var fade2 = cc.action.FadeOut(1)
		var fade_in2 = fade.reverse()
		var seq2 = cc.action.Sequence(fade2, fade_in2)
		var repeat2 = cc.action.RepeatForever(seq2)
		label2.runAction(repeat2)
		
		var label3 = cc.LabelBMFont("Test", pack.locate("bitmapFontTest2.fnt"))
		// testing anchors
		label3.anchorPoint = cc.Point(1, 1)
		this.addChild(label3, 0, 3)
		
		var s = cocos.director.winSize
		label1.position = cc.Point(0, 0)
		label2.position = cc.Point(s.width / 2, s.height / 2)
		label3.position = cc.Point(s.width, s.height)
		
		cocos.director.timer.channel().bind(Events.OnTick, this, step)
	}
	
	function step(evt: TimeEvent)
	{
		_time += evt.delta
		var str = format("%.3f", _time)

		str = str + " Test j"
		
		var label1 = this.getChildByTag(1)
		label1.string = str
		var label2 = this.getChildByTag(2)
		label2.string = str
		var label3 = this.getChildByTag(3)
		label3.string = str
	}
	
	function title()
	{
		return "CCLabelBMFont"
	}
	
	function subtitle()
	{
		return "Testing alignment. Testing opacity + tint"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Atlas4 : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		_time = 0
		var label = cc.LabelBMFont("Bitmap Font Atlas", pack.locate("bitmapFontTest.fnt"))
		addChild(label)
		
		var s = cocos.director.winSize
		
		label.position = cc.Point(s.width / 2, s.height / 2)
		label.anchorPoint = cc.Point(0.5, 0.5)
		
		var charB = label.getChildByTag(0)
		var charF = label.getChildByTag(7)
		var charA = label.getChildByTag(12)
		
		var rotate = cc.action.RotateBy(2, 360)
		var rot_4ever = cc.action.RepeatForever(rotate)
		
		var scale = cc.action.ScaleBy(2, 1.5)
		var scale_back = scale.reverse()
		var scale_seq = cc.action.Sequence(scale, scale_back)
		var scale_4ever = cc.action.RepeatForever(scale_seq)
		
		var jump = cc.action.JumpBy(0.5, cc.Point(0, 0), 60, 1)
		var jump_4ever = cc.action.RepeatForever(jump)
		
		var fade_out = cc.action.FadeOut(1)
		var fade_in = cc.action.FadeIn(1)
		var seq = cc.action.Sequence(fade_out, fade_in)
		var fade_4ever = cc.action.RepeatForever(seq)
		
		charB.runAction(rot_4ever)
		charB.runAction(scale_4ever)
		charF.runAction(jump_4ever)
		charA.runAction(fade_4ever)
		
		// Bottom Label
		var label2 = cc.LabelBMFont("00.0", pack.locate("bitmapFontTest.fnt"))
		this.addChild(label2, 0, 2)
		label2.position = cc.Point(s.width / 2, 80)
		
		var lastChar = label2.getChildByTag(3)
		var rotate2 = cc.action.RotateBy(2, 360)
		var rot_4ever2 = cc.action.RepeatForever(rotate2)
		lastChar.runAction(rot_4ever2)
		
		cocos.director.timer.channel().bind(Events.OnTick, this, step)
	}

	function step(evt: TimeEvent)
	{
		_time += evt.delta
		var str = format("%.3f", _time)
		
		var label1 = this.getChildByTag(2)
		label1.string = str
	}
	
	function title()
	{
		return "CCLabelBMFont"
	}
	
	function subtitle()
	{
		return "Using fonts as CCSprite objects. Some characters should rotate."
	}
}

////////////////////////////////////////////////////////////////////////////////

class Atlas5 : AtlasDemo
{
	constructor()
	{
		base.constructor()

		var label = cc.LabelBMFont("abcdefg", pack.locate("bitmapFontTest4.fnt"))
		addChild(label)
		
		var s = cocos.director.winSize
		
		label.position = cc.Point(s.width / 2, s.height / 2)
		label.anchorPoint = cc.Point(0.5, 0.5)
	}
	
	function onEnter()
	{
	
	}
	
	function title()
	{
		return "CCLabelBMFont"
	}
	
	function subtitle()
	{
		return "Testing padding"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Atlas6 : AtlasDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var label = cc.LabelBMFont("FaFeFiFoFu", pack.locate("bitmapFontTest5.fnt"))
		addChild(label)
		label.position = cc.Point(s.width/2, s.height/2 + 50)
		label.anchorPoint = cc.Point(0.5, 0.5)
		
		label = cc.LabelBMFont("fafefifofu", pack.locate("bitmapFontTest5.fnt"))
		addChild(label)
		label.position = cc.Point(s.width/2, s.height/2)
		label.anchorPoint = cc.Point(0.5, 0.5)
		
		label = cc.LabelBMFont("aeiou", pack.locate("BitmapFontTest5.fnt"))
		addChild(label)
		label.position = cc.Point(s.width/2, s.height/2-50)
		label.anchorPoint = cc.Point(0.5, 0.5)
	}
	
	function onEnter()
	{
		_time = 0
	}

	function title()
	{
		return "CCLabelBMFont"
	}

	function subtitle()
	{
		return "Rendering should be OK. Testing offset"
	}
}

////////////////////////////////////////////////////////////////////////////////

class AtlasBitmapColor : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		_time = 0
		
		var s = cocos.director.winSize
		
		var label = cc.LabelBMFont("Blue", pack.locate("bitmapFontTest5.fnt"))
		addChild(label)
		label.position = cc.Point(s.width/2, s.height/2 - 50)
		label.anchorPoint = cc.Point(0.5, 0.5)
		label.color = nit.Color.BLUE
		
		label = cc.LabelBMFont("Red", pack.locate("bitmapFontTest5.fnt"))
		addChild(label)
		label.position = cc.Point(s.width/2, s.height/2)
		label.anchorPoint = cc.Point(0.5, 0.5)
		label.color = nit.Color.RED
		
		label = cc.LabelBMFont("Green", pack.locate("bitmapFontTest5.fnt"))
		addChild(label)
		label.position = cc.Point(s.width/2, s.height/2 + 50)
		label.anchorPoint = cc.Point(0.5, 0.5)
		label.color = nit.Color.GREEN
	}

	function title()
	{
		return "CCLabelBMFont"
	}

	function subtitle()
	{
		return "Testing color"
	}
}

////////////////////////////////////////////////////////////////////////////////

class AtlasFastBitmap : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		for (var i=0; i<100; ++i)
		{
			var str = format("-%d-", i)
			var label = cc.LabelBMFont(str, pack.locate("bitmapFontTest.fnt"))
			addChild(label)
			
			var s = cocos.director.winSize
			
			var p = cc.Point(math.random() * s.width, math.random() * s.height)
			label.position = p
			label.anchorPoint = cc.Point(0.5, 0.5)
		}
	}
	
	function title()
	{
		return "CCLabelBMFont"
	}
	
	function subtitle()
	{
		return "Creating several CCLabelBMFont with the same .fnt file should be fast"
	}
}

////////////////////////////////////////////////////////////////////////////////

class BitmapFontMultiLine : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		// left
		var label1 = cc.LabelBMFont("Multi line\nLeft", pack.locate("bitmapFontTest3.fnt"))
		label1.anchorPoint = cc.Point(0, 0)
		addChild(label1)
		
		var s = label1.contentSize
		printf("content size: %.2f x %.2f", s.width, s.height)
		
		// center
		var label2 = cc.LabelBMFont("Multi line\nCenter", pack.locate("bitmapFontTest3.fnt"))
		label2.anchorPoint = cc.Point(0.5, 0.5)
		addChild(label2)
		
		s = label2.contentSize
		printf("content size: %.2f x %.2f", s.width, s.height)
		
		// right
		var label3 = cc.LabelBMFont("Multi line\nRight\nThree lines Three", pack.locate("bitmapFontTest3.fnt"))
		label3.anchorPoint = cc.Point(1, 1)
		addChild(label3)
		
		s = label3.contentSize
		printf("content size: %.2f x %.2f", s.width, s.height)
	
		s = cocos.director.winSize
		label1.position = cc.Point(0, 0)
		label2.position = cc.Point(s.width / 2, s.height / 2)
		label3.position = cc.Point(s.width, s.height)
	}
	
	function title()
	{
		return "CCLabelBMFont"
	}
	
	function subtitle()
	{
		return "Multiline + anchor point"
	}
}

////////////////////////////////////////////////////////////////////////////////

class LabelsEmpty : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	var _empty: bool
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var label1 = cc.LabelBMFont("", pack.locate("bitmapFontTest3.fnt"))
		addChild(label1, 0, 1)
		label1.position = cc.Point(s.width / 2, s.height - 100)
		
		var label2 = cc.LabelTTF("", "Arial", 24)
		addChild(label2, 0, 2)
		label2.position = cc.Point(s.width / 2, s.height / 2)
		
		var label3 = cc.LabelAtlas("", pack.locate("tuffy_bold_italic-charmap.png"), 48, 64, $' ')
		addChild(label3, 0, 3)
		label3.position = cc.Point(s.width / 2, 0 + 100)
		
		cocos.director.scheduler.repeat(this, updateStrings, 1.0)
		
		_empty = false
	}
	
	function updateStrings()
	{
		var label1 = getChildByTag(1)
		var label2 = getChildByTag(2)
		var label3 = getChildByTag(3)
		
		if (!_empty)
		{
			label1.string = "not empty"
			label2.string = "not empty"
			label3.string = "hi"
			
			_empty = true
		}
		else
		{
			label1.string = ""
			label2.string = ""
			label3.string = ""
			
			_empty = false
		}
	}
	
	function title()
	{
		return "Testing empty labels"
	}
	
	function subtitle()
	{
		return "3 empty labels: LabelAtlas, LabelTTF and LabelBMFont"
	}
}

////////////////////////////////////////////////////////////////////////////////

class LabelBMFontHD : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var label1 = cc.LabelBMFont("TESTING RETINA DISPLAY", pack.locate("konqa32.fnt"))
		addChild(label1)
		label1.position = cc.Point(s.width / 2, s.height / 2)
	}
	
	function title()
	{
		return "Testing Retina Display BMFont"
	}
	
	function subtitle()
	{
		return "loading konqa32 or konqa32-hd"
	}
}

////////////////////////////////////////////////////////////////////////////////

class LabelAtlasHD : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var label1 = cc.LabelAtlas("TESTING RETINA DISPLAY", pack.locate("larabie-16.png"), 10, 20, $'A')
		label1.anchorPoint = cc.Point(0.5, 0.5)
		
		this.addChild(label1)
		label1.position = cc.Point(s.width / 2, s.height / 2)
	}
	
	function title()
	{
		return "LabelAtlas with Retina Display"
	}
	
	function subtitle()
	{
		return "loading larabie-16 / larabie-16-hd"
	}
}

////////////////////////////////////////////////////////////////////////////////

class LabelGlyphDesigner : AtlasDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var layer = cc.LayerColor(nit.Color(0.5, 0.5, 0.5, 1))
		addChild(layer, -10)
		
		var label1 = cc.LabelBMFont("Testing Glyph Designer", pack.locate("futura-48.fnt"))
		addChild(label1)
		label1.position = cc.Point(s.width / 2, s.height / 2)
	}
	
	function onEnter()
	{
		_time = 0
		
	}
	
	function title()
	{
		return "Testing Glyph Designer"
	}
	
	function subtitle()
	{
		return "You should see a font with shawdows and outline"
	}
}

////////////////////////////////////////////////////////////////////////////////

class LabelTTFTest : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var left = cc.LabelTTF("align left", cc.Size(s.width, 50), cc.LabelTTF.ALIGN_LEFT, "Arial", 32)
		var center = cc.LabelTTF("align center", cc.Size(s.width, 50), cc.LabelTTF.ALIGN_CENTER, "Arial", 32)
		var right = cc.LabelTTF("align right", cc.Size(s.width, 50), cc.LabelTTF.ALIGN_RIGHT, "Arial", 32)
		
		left.position = cc.Point(s.width / 2, 200)
		center.position = cc.Point(s.width / 2, 150)
		right.position = cc.Point(s.width / 2, 100)
		
		this.addChild(left)
		this.addChild(center)
		this.addChild(right)
		
		print("ok")
		
	}
	
	function title()
	{
		return "Testing CCLabelTTF"
	}
	
	function subtitle()
	{
		return "You should see 3 labels aligned left, center and right"
	}
}

////////////////////////////////////////////////////////////////////////////////

class LabelTTFMultiline : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		var center = cc.LabelTTF("word wrap \"testing\" (bla0)   bla1  'bla2' [bla3] (bla4) {bla5} {bla6} [bla7] (bla8) [bla9] 'bla0' \"bla1\"",
								cc.Size(s.width / 2, 200), cc.LabelTTF.ALIGN_CENTER, "MarkerFelt.ttc", 32)
		
		center.position = cc.Point(s.width / 2, 130)
		
		::lbl := center
		
		this.addChild(center)
		
	}
	
	function title()
	{
		return "Testing CCLabelTTF Word Wrap"
	}
	
	function subtitle()
	{
		return "Word wrap using CCLabelTTF"
	}
}

////////////////////////////////////////////////////////////////////////////////

class LabelTTFInternational : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		var str = 
		"격려문자에 \"각하 만세\" 답신 파문\n" +
		"損保ジャパンと興亜 新会社へ\n" +
		"प्रज्ञापारमिताहृदय Prajñāpāramitā Hṛdaya\n" + 
		"般若波羅蜜多心經\n" +
		"Bōrěbōluómìduō Xīnjīng\n" +
		"音乐 都要好好的小沈阳\n" +
		"Laura Smet arrêtée en tenue d'Eve\n" +
		"Westerwelle gegen größeres Hilfspaket für...\n" +
		"¿Seguro que está bien puesto, Eva?"
		
		var label = cc.LabelTTF(str, "산돌고딕Neo1 Bold")
		
		::lbl := label
		
		label.position = cc.Point(s.width / 2, s.height / 2)
		
		this.addChild(label)
		
	}
	
	function title()
	{
		return "UTF-8 International"
	}
}

////////////////////////////////////////////////////////////////////////////////

class AtlasTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var layer = labeltest.nextAtlasTest()
		this.addChild(layer)
		cocos.director.replaceScene(this)
	}
}

return AtlasTestScene()