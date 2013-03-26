var pack = script.locator

testlist := [ ]

function addTest(nitFile, testname, testclass)
{
	testlist.append({nitFile= nitFile, name = testname, tclass = testclass})
}

require "testlist"

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
		
		foreach (index, data in testlist)
		{
			var label = cc.LabelTTF(data.name, "산돌고딕Neo1 Bold", 34)
			var item = cc.MenuItemLabel(label, this, onSelectMenu)
				
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
		var info = testlist[evt.item.zOrder - 10000]
		print("select : " + info.name)
		
		require(info.nitFile)
		
		if (info.tclass == null)
			return
			
		var inst = getroottable()[info.tclass]()
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

