require "TestScene"

var pack = script.locator

s_pPathGrossini       := pack.locate("grossini.png", "*Images*")
s_pPathSister1        := pack.locate("grossinis_sister1.png")
s_pPathSister2        := pack.locate("grossinis_sister2.png")
s_pPathB1             := pack.locate("b1.png")
s_pPathB2             := pack.locate("b2.png")
s_pPathR1             := pack.locate("r1.png")
s_pPathR2             := pack.locate("r2.png")
s_pPathF1             := pack.locate("f1.png")
s_pPathF2             := pack.locate("f2.png")
s_pPathBlock          := pack.locate("blocks.png")
s_back                := pack.locate("background.png")
s_back1               := pack.locate("background1.png")
s_back2               := pack.locate("background2.png")
s_back3               := pack.locate("background3.png")
s_stars1              := pack.locate("stars.png")
s_stars2              := pack.locate("stars2.png")
s_fire                := pack.locate("fire.png")
s_snow                := pack.locate("snow.png")
s_streak              := pack.locate("streak.png")
s_PlayNormal          := pack.locate("btn-play-normal.png")
s_PlaySelect          := pack.locate("btn-play-selected.png")
s_AboutNormal         := pack.locate("btn-about-normal.png")
s_AboutSelect         := pack.locate("btn-about-selected.png")
s_HighNormal          := pack.locate("btn-highscores-normal.png")
s_HighSelect          := pack.locate("btn-highscores-selected.png")
s_Ball                := pack.locate("ball.png")
s_Paddle              := pack.locate("paddle.png")
s_pPathClose          := pack.locate("close.png")
s_MenuItem            := pack.locate("menuitemsprite.png")
s_SendScore           := pack.locate("SendScoreButton.png")
s_PressSendScore      := pack.locate("SendScoreButtonPressed.png")
s_Power               := pack.locate("powered.png")
s_AtlasTest           := pack.locate("atlastest.png")

testlist := 
[ 
	"ActionsTest",
	"TransitionsTest",
	"ProgressActionsTest",
	"EffectsTest",
	"ClickAndMoveTest",
	"RotateWorldTest",
	"ParticleTest",
	"EaseActionsTest",
	"MotionStreakTest",
	"CocosNodeTest",
	"TouchesTest",
	"MenuTest",
	"LayerTest",
	"SceneTest",
	"IntervalTest",
	"LabelTest",
	"TextInputTest",
	"SpriteTest",
	"RenderTextureTest",
	"Texture2dTest",
	"EffectAdvancedTest",
	"AccelerometerTest",
	"KeyPadTest",
	"PerformanceTest",
	"ZwoptexTest",
	"DirectorTest",
	"FontTest",
]

class TestMainMenu : cc.ScriptLayer
{
	_touchPos = null
	_touchBegin = null
	_itemMenu = null
	
	constructor()
	{
		base.constructor()
		
		var closeItem = cc.MenuItemImage(
			pack.locate("CloseNormal.png"),
			pack.locate("CloseSelected.png"),
			this, onCloseMenu)
			
		var winSize = cocos.director.winSize
		closeItem.position = cc.Point(winSize.width - 30, winSize.height - 30)
		
		var closemenu = cc.Menu(closeItem)
		closemenu.position = cc.Point(0, 0)
		this.addChild(closemenu)	
		
		_itemMenu = cc.Menu()
		
		foreach (index, test in testlist)
		{
			var label = cc.LabelTTF(test, "산돌고딕Neo1 Bold", 34)
			var item = cc.MenuItemLabel(label, this, onSelectMenu)
			item.userValue = test
				
			_itemMenu.addChild(item, index + 10000)
			item.position = cc.Point(winSize.width / 2, (winSize.height - (index + 1) * 40))
		}
		
		_itemMenu.contentSize = cc.Size(winSize.width, (testlist.len() + 1) * 40)
		this.addChild(_itemMenu)
		
		_touchPos = cc.Point(0, 0) 
		_itemMenu.position = _touchPos
		
		this.touchEnabled = true
		
		channel().bind(Events.OnCCTouchBegin, this, onTouchBegin)
		channel().bind(Events.OnCCTouchMoved, this, onTouchMove)
	}
	
	function onCloseMenu(evt: cc.MenuItemEvent)
	{
		print(this + ": OnCloseMenu : " + evt)
		app.stop(0)
	}	
	
	function onSelectMenu(evt: cc.MenuItemEvent)
	{
		var test = evt.item.userValue
		print("select : " + test)
		
		var inst = dofile(test)

		inst.runThisTest()
	}

	function onTouchBegin(evt: cc.TouchEvent)
	{
		_touchBegin = evt.touch0.locationInView(evt.touch0.view)
		_touchBegin = cocos.director.toGl(_touchBegin)
	}
	
	function onTouchMove(evt: cc.TouchEvent)
	{
		var touch = evt.touch0.locationInView(evt.touch0.view)
		touch = cocos.director.toGl(touch)		
		
		var moveY = touch.y - _touchBegin.y
		var curPos = _itemMenu.position
		var nextPos = cc.Point(curPos.x, curPos.y + moveY)
		
		if (nextPos.y < 0.0)
		{
			_itemMenu.position = cc.Point(0, 0)
			return
		}
		
		var winSize = cocos.director.winSize
		if (nextPos.y > (testlist.len() + 1) * 40 - winSize.height)
		{
			_itemMenu.position = cc.Point(curPos.x, (testlist.len() + 1) * 40 - winSize.height)
			return
		}
		
		_itemMenu.position = nextPos
		_touchBegin = touch
		_touchPos = nextPos
	}
}

function startTestMain()
{
	var s = cc.ScriptScene()
	var testmain = TestMainMenu()
	s.addChild(testmain)

	cocos.director.replaceScene(s)
}

startTestMain()

