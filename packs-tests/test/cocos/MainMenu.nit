require "TestScene"

var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

IMG := 
{
	GROSSINI		= pack.locate("grossini.png", "*Images*")
	SISTER1			= pack.locate("grossinis_sister1.png")
	SISTER2			= pack.locate("grossinis_sister2.png")
	B1				= pack.locate("b1.png")
	B2				= pack.locate("b2.png")
	R1				= pack.locate("r1.png")
	R2				= pack.locate("r2.png")
	F1				= pack.locate("f1.png")
	F2				= pack.locate("f2.png")
	BLOCKS			= pack.locate("blocks.png")
	BACK			= pack.locate("background.png")
	BACK1			= pack.locate("background1.png")
	BACK2			= pack.locate("background2.png")
	BACK3			= pack.locate("background3.png")
	STARS1			= pack.locate("stars.png")
	STARS2			= pack.locate("stars2.png")
	FIRE			= pack.locate("fire.png")
	SNOW			= pack.locate("snow.png")
	STREAK			= pack.locate("streak.png")
	PLAY_NORMAL		= pack.locate("btn-play-normal.png")
	PLAY_SELECT		= pack.locate("btn-play-selected.png")
	ABOUT_NORMAL	= pack.locate("btn-about-normal.png")
	ABOUT_SELECT	= pack.locate("btn-about-selected.png")
	HIGH_NORMAL		= pack.locate("btn-highscores-normal.png")
	HIGH_SELECT		= pack.locate("btn-highscores-selected.png")
	BALL			= pack.locate("ball.png")
	PADDLE			= pack.locate("paddle.png")
	CLOSE			= pack.locate("close.png")
	MENU_ITEM		= pack.locate("menuitemsprite.png")
	SEND_SCORE		= pack.locate("SendScoreButton.png")
	SEND_SCORE_PRESS = pack.locate("SendScoreButtonPressed.png")
	POWERED			= pack.locate("powered.png")
	ATLAS_TEST		= pack.locate("atlastest.png")
}

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

class TestMainMenu : cc.ScriptLayer
{
	var _touchPos = null
	var _touchBegin = null
	var _itemMenu = null
	
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
		
		channel().bind(EVT.CC_TOUCH_BEGIN, this, onTouchBegin)
		channel().bind(EVT.CC_TOUCH_MOVED, this, onTouchMove)
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

////////////////////////////////////////////////////////////////////////////////

function startTestMain()
{
	var s = cc.ScriptScene()
	var testmain = TestMainMenu()
	s.addChild(testmain)

	cocos.director.replaceScene(s)
}

startTestMain()

