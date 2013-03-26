var pack = script.locator

////////////////////////////////////////////////////////////////////////////////
class LabelTest
{
	sceneindex = -1
	scenemaxcnt = 17
	
	constructor()
	{
	}
	
	function _createAtlasTest(index)
	{
		var layer = null
		
		switch (index)
		{
		case 0:
			layer = LabelAtlasTest()
			break
		case 1:
			layer = LabelAtlasColorTest()
			break
		case 2:
			layer = Atlas3()
			break
		case 3:
			layer = Atlas4()
			break
		case 4:
			layer = Atlas5()
			break
		case 5:
			layer = Atlas6()
			break
		case 6:
			layer = AtlasBitmapColor()
			break
		case 7:
			layer = AtlasFastBitmap()
			break
		case 8:
			layer = BitmapFontMultiLine()
			break
		case 9:
			layer = LabelsEmpty()
			break
		case 10:
			layer = LabelBMFontHD()
			break
		case 11:
			layer = LabelAtlasHD()
			break
		case 12:
			layer = LabelGlyphDesigner()
			break
		case 13:
			layer = Atlas1()
			break
		case 14:
			layer = LabelTTFTest()
			break
		case 15:
			layer = LabelTTFMultiline()
			break
		case 16:
			layer = LabelTTFInternational()
			break
		default:
			break
		}
		
		print(layer._classname)
		
		return layer
	}
	
	function nextAtlasTest()
	{
		sceneindex++
		sceneindex = sceneindex % scenemaxcnt
		
		return _createAtlasTest(sceneindex)
	}
	function backAtlasTest()
	{
		sceneindex--
		if (sceneindex < 0)
			sceneindex = scenemaxcnt -1
			
		return _createAtlasTest(sceneindex)
	}
	function restartAtlasTest()
	{
		return _createAtlasTest(sceneindex)
	}
}
labeltest := LabelTest()
////////////////////////////////////////////////////////////////////////////////

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
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// AtlasDemo
//
class AtlasDemo : cc.ScriptLayer
{
	textureAtlas 						= null
	time     							= 0 
	
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
			var l = cc.LabelTTF(strSubtitle, "Thonburi", 16)
			this.addChild(l, 1)
			l.position = cc.Point(s.width / 2, s.height - 80)
		}
		
		var item1 = cc.MenuItemImage(
			s_pPathB1,
			s_pPathB2,
			this, onBackMenu)
		var item2 = cc.MenuItemImage(
			s_pPathR1,
			s_pPathR2,
			this, onRestartMenu)
		var item3 = cc.MenuItemImage(
			s_pPathF1,
			s_pPathF2,
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


////////////////////////////////////////////////////////////////////////////////
//
// Atlas1
//
class Atlas1 : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		return
		
		
		textureAtlas = cc.TextureAtlas(s_AtlasTest, 3)
		
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

////////////////////////////////////////////////////////////////////////////////
//
// LabelAtlasTest
//
class LabelAtlasTest : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		time = 0
		
		var label1 = cc.LabelAtlas("123 Test", pack.locate("tuffy_bold_italic-charmap.png"), 48, 64, char(' '))
		this.addChild(label1, 0, 0)
		label1.position = cc.Point(10, 100)
		label1.opacity = 200
		
		var label2 = cc.LabelAtlas("0123456789", pack.locate("tuffy_bold_italic-charmap.png"), 48, 64, char(' '))
		this.addChild(label2, 0, 1)
		label2.position = cc.Point(10, 200)
		label2.opacity = 32
		
		cocos.director.timer.channel().bind(Events.OnTick, this, step)
	}
	function step(evt: TimeEvent)
	{
		time += evt.delta
		var label1 = this.getChildByTag(0)
		// dump(label1)
		var str = format("%.3f", time)

		label1.string = str + " Test"
		
		var label2 = this.getChildByTag(1)
		label2.string = "" + time.tointeger()
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

////////////////////////////////////////////////////////////////////////////////
//
// LabelAtlasColorTest
//
class LabelAtlasColorTest : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		time = 0
		
		var label1 = cc.LabelAtlas("123 Test", pack.locate("tuffy_bold_italic-charmap.png"), 48, 64, char(' '))
		this.addChild(label1, 0, 0)
		label1.position = cc.Point(10, 100)
		label1.opacity = 200
		
		var label2 = cc.LabelAtlas("0123456789", pack.locate("tuffy_bold_italic-charmap.png"), 48, 64, char(' '))
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
		time += evt.delta
		var label1 = this.getChildByTag(0)

		var str = format("%.3f", time)

		label1.string = str + " Test"
		
		var label2 = this.getChildByTag(1)
		label2.string = "" + time.tointeger()
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

////////////////////////////////////////////////////////////////////////////////
//
// Atlas3
//
class Atlas3 : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		time = 0
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
		time += evt.delta
		var str = format("%.3f", time)

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

////////////////////////////////////////////////////////////////////////////////
//
// Atlas4
//
class Atlas4 : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		time = 0
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
		time += evt.delta
		var str = format("%.3f", time)
		
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

////////////////////////////////////////////////////////////////////////////////
//
// Atlas5
//
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

////////////////////////////////////////////////////////////////////////////////
//
// Atlas6
//
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
		time = 0
		
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

////////////////////////////////////////////////////////////////////////////////
//
// AtlasBitmapColor
//
class AtlasBitmapColor : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		time = 0

		
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

////////////////////////////////////////////////////////////////////////////////
//
// AtlasFastBitmap
//
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

////////////////////////////////////////////////////////////////////////////////
//
// BitmapFontMultiLine
//
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
//
// LabelsEmpty
//
class LabelsEmpty : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	var setEmpty: bool
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var label1 = cc.LabelBMFont("", pack.locate("bitmapFontTest3.fnt"))
		addChild(label1, 0, 1)
		label1.position = cc.Point(s.width / 2, s.height - 100)
		
		var label2 = cc.LabelTTF("", "Arial", 24)
		addChild(label2, 0, 2)
		label2.position = cc.Point(s.width / 2, s.height / 2)
		
		var label3 = cc.LabelAtlas("", pack.locate("tuffy_bold_italic-charmap.png"), 48, 64, char(' '))
		addChild(label3, 0, 3)
		label3.position = cc.Point(s.width / 2, 0 + 100)
		
		cocos.director.scheduler.repeat(this, updateStrings, 1.0)
		
		setEmpty = false
	}
	
	function updateStrings()
	{
		var label1 = getChildByTag(1)
		var label2 = getChildByTag(2)
		var label3 = getChildByTag(3)
		
		if (!setEmpty)
		{
			label1.string = "not empty"
			label2.string = "not empty"
			label3.string = "hi"
			
			setEmpty = true
		}
		else
		{
			label1.string = ""
			label2.string = ""
			label3.string = ""
			
			setEmpty = false
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

////////////////////////////////////////////////////////////////////////////////
//
// LabelBMFontHD
//
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

////////////////////////////////////////////////////////////////////////////////
//
// LabelAtlasHD
//
class LabelAtlasHD : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var label1 = cc.LabelAtlas("TESTING RETINA DISPLAY", pack.locate("larabie-16.png"), 10, 20, char('A'))
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

////////////////////////////////////////////////////////////////////////////////
//
// LabelGlyphDesigner
//
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
		time = 0
		
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

////////////////////////////////////////////////////////////////////////////////
//
// LabelTTFTest
//
class LabelTTFTest : AtlasDemo
{
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		var left = cc.LabelTTF("align left", cc.Size(s.width, 50), cc.LabelTTF.ALIGN_LEFT, "Marker Felt", 32)
		var center = cc.LabelTTF("align center", cc.Size(s.width, 50), cc.LabelTTF.ALIGN_CENTER, "marker Felt", 32)
		var right = cc.LabelTTF("align right", cc.Size(s.width, 50), cc.LabelTTF.ALIGN_RIGHT, "marker Felt", 32)
		
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


////////////////////////////////////////////////////////////////////////////////
//
// LabelTTFMultiline
//
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

////////////////////////////////////////////////////////////////////////////////
//
// LabelTTFInternational
//
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

