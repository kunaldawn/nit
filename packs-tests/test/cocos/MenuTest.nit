var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var TAG = 
{
	MENU 							= 1,
	MENU0 							= 0,
	MENU1 							= 1,
}

var MID =
{
	CALLBACK 						= 1001,
	CALLBACK2 						= 1001,
	DISABLED 						= 1002,
	ENABLE 							= 1003,
	CONFIG 							= 1004,
	QUIT 							= 1005,
	opacity 						= 1006,
	ALIGN 							= 1007,
	CALLBACK3 						= 1008,
	BACKCALLBACK 					= 1009,
}

////////////////////////////////////////////////////////////////////////////////

class MenuLayer1 : cc.ScriptLayer
{
	var _disabledItem = null
	
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		removeAllChildren(true)
		
		cc.MenuItemFont.setDefaultFontSize(30)
		cc.MenuItemFont.setDefaultFontName("Courier New")
		
		touchEnabled = true    
		
		// Font Item
		var spriteNormal = cc.Sprite(s_MenuItem, cc.Rect(0, 23*2, 115, 23))
		var spriteSelected = cc.Sprite(s_MenuItem, cc.Rect(0,23*1, 115, 23))
		var spriteDisabled = cc.Sprite(s_MenuItem, cc.Rect(0, 23*0, 115, 23))
		
		var item1 = cc.MenuItemSprite(spriteNormal, spriteSelected, spriteDisabled,this, onMenu)
		// Image Item
		var item2 = cc.MenuItemImage(s_SendScore, s_PressSendScore, this, onMenu2)
		
		// Label Item (LabelAtlas)
		var labelAtlas = cc.LabelAtlas("0123456789", pack.locate("fps_images.png", "*fonts"), 16, 24, char('.'))
		var item3 = cc.MenuItemLabel(labelAtlas, this, onMenuDisabled)
		item3.disabledColor = nit.Color(32 / 255, 32 / 255, 64 / 255, 1)
		item3.color = nit.Color(200 / 255, 200 / 255, 1, 1)
		
		// Font Item
		var item4 = cc.MenuItemFont("I toggle enable items", this, onMenuEnable)
		
		item4.fontSize = 20
		item4.fontName = "Arial"
		
		// Label Item (CCLabelBMFont)
		var label = cc.LabelBMFont("configuration", pack.locate("bitmapFontTest3.fnt", "*fonts"))
		var item5 = cc.MenuItemLabel(label, this, onMenuConfig)
		
		// Testing issue #500
		item5.scale(0.8)
		
		// Font Item
		var item6 = cc.MenuItemFont("Quit", this, onQuit)
		
		var color_action = cc.action.TintBy(0.5, 0, -255, -255)
		var color_back = color_action.reverse()
		var seq = cc.action.Sequence(color_action, color_back)
		item6.runAction(cc.action.RepeatForever(seq))
		
		var menu = cc.Menu(item1, item2, item3, item4, item5, item6)
		menu.alignItemsVertically()
		
		// elastic effect
		var s = cocos.director.winSize
		
		var i = 0
		var array = menu.children
		
		foreach (k, v in array)
		{
			var child = v
			var dstpos = child.position
			var offset = s.width / 2 + 50
			if (i % 2 == 0)
				offset = -offset
			
			child.position = cc.Point(dstpos.x + offset, dstpos.y)
			child.runAction(cc.action.EaseElasticOut(
						cc.action.MoveBy(2, cc.Point(dstpos.x - offset, 0), 0.35)))
			i++
		}
		
		_disabledItem = item3
//		_disabledItem.enabled = false
		
		addChild(menu)
		
	}
	
	function onMenu()
	{
		parent.switchTo(1)
	}

	function onMenuConfig()
	{
		parent.switchTo(3)
	}
	
	function allowTouches(dt)
	{
		cocos.touchDispatcher.setPriority(-128 + 1, this)
		print("TOUCHES ALLOWED AGAIN")
	}
	
	function onMenuDisabled()
	{
		//hijack all touch events for 5 seconds
		cocos.touchDispatcher.setPriority(-128 - 1, this)
		cocos.director.scheduler.once(this, allowTouches, 5.0)
		print("TOUCHES DISABLED FOR 5 SECONDS")
	}
	
	function onMenuEnable()
	{
		_disabledItem.enabled = !_disabledItem.enabled
	}
	
	function onMenu2()
	{
		parent.switchTo(2)
	}
	
	function onQuit()
	{
		parent.switchTo(0)
	}
}

////////////////////////////////////////////////////////////////////////////////

class MenuLayer2 : cc.ScriptLayer
{
	var _centeredMenu = null
	var _alignedH = null
	
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		removeAllChildren(true)
		
		for (var i=0; i<2; i++)
		{
			var item1 = cc.MenuItemImage(s_PlayNormal, s_PlaySelect, this, onMenu)
			var item2 = null
			item2 = cc.MenuItemImage(s_HighNormal, s_HighSelect, this, @onMenuOpacity(item2))
			var item3 = cc.MenuItemImage(s_AboutNormal, s_AboutSelect, this, onMenuAlign)
			
			item1.scaleX = 1.5
			item2.scaleX = 0.5
			item3.scaleX = 0.5
			
			var menu = cc.Menu(item1, item2, item3)
			menu.tag = TAG.MENU
			addChild(menu, 0, 100+i)
			
			_centeredMenu = menu.position
		}
		_alignedH = true
		alignMenusH()
	}
	
	function alignMenusH()
	{
		for (var i=0; i<2; i++)
		{
			var menu = getChildByTag(100+i)
			menu.position = _centeredMenu
			if (i==0)
			{
				// TIP: if no padding, padding = 5
				menu.alignItemsHorizontally();			
				var p = menu.position
				menu.position = cc.Point(p.x, p.y+30)
			}
			else
			{
				// TIP: but padding is configurable
				menu.alignItemsHorizontallyWithPadding(40);			
				var p = menu.position
				menu.position = cc.Point(p.x, p.y-30)
			}
		}
	}

	function alignMenusV()
	{
		for (var i=0; i<2; i++)
		{
			var menu = getChildByTag(100+i)
			menu.position = _centeredMenu
			if (i==0)
			{
				// TIP: if no padding, padding = 5
				menu.alignItemsVertically();			
				var p = menu.position
				menu.position = cc.Point(p.x+100, p.y)
			}
			else
			{
				// TIP: but padding is configurable
				menu.alignItemsVerticallyWithPadding(40);			
				var p = menu.position
				menu.position = cc.Point(p.x-100, p.y)
			}
		}
	}
	
	function onMenu()
	{
		parent.switchTo(0)
	}
	
	function onMenuOpacity(menuItem)
	{
		var menu = menuItem.parent
		var opacity = menu.opacity
		if (opacity == 128)
			menu.opacity = 255
		else
			menu.opacity = 128 
	}
	
	function onMenuAlign()
	{
		_alignedH = !_alignedH
		
		if (_alignedH)
			alignMenusH()
		else
			alignMenusV()
	}
}

////////////////////////////////////////////////////////////////////////////////

class MenuLayer3 : cc.ScriptLayer
{
	var _disabledItem = null
	
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		removeAllChildren(true)

		cc.MenuItemFont.setDefaultFontSize(28)
		cc.MenuItemFont.setDefaultFontName("Arial")
		
		var label = cc.LabelBMFont("Enable AtlasItem", pack.locate("bitmapFontTest3.fnt"))
		var item1 = cc.MenuItemLabel(label, this, onMenu2)
		var item2 = cc.MenuItemFont("--- Go Back ---", this, onMenu)
		
		var spriteNormal = cc.Sprite(s_MenuItem, cc.Rect(0, 23*2, 115, 23))
		var spriteSelected = cc.Sprite(s_MenuItem, cc.Rect(0, 23*1, 115, 23))
		var spriteDisabled = cc.Sprite(s_MenuItem, cc.Rect(0, 23*0, 115, 23))
		
		var item3 = cc.MenuItemSprite(spriteNormal, spriteSelected, spriteDisabled, this, onMenu3)
		_disabledItem = item3
		_disabledItem.enabled = false
		
		var menu = cc.Menu(item1, item2,  item3)
		menu.position = cc.Point(0, 0)
		
		var s = cocos.director.winSize
		item1.position = cc.Point(s.width/2-150, s.height/2)
		item2.position = cc.Point(s.width/2-200, s.height/2)
		item3.position = cc.Point(s.width/2, s.height/2-100)
		
		var jump = cc.action.JumpBy(3, cc.Point(400, 0), 50, 4)
		item2.runAction(cc.action.RepeatForever(cc.action.Sequence(jump, jump.reverse())))
		
		var spin1 = cc.action.RotateBy(3, 360)
		var spin2 = cc.action.RotateBy(3, 360)
		var spin3 = cc.action.RotateBy(3, 360)
		
		item1.runAction(cc.action.RepeatForever(spin1))
		item2.runAction(cc.action.RepeatForever(spin2))
		item3.runAction(cc.action.RepeatForever(spin3))
		
		addChild(menu)
	}
	
	function onMenu()
	{
		parent.switchTo(0)
	}
	
	function onMenu2(sender)
	{
		_disabledItem.enabled = !_disabledItem.enabled
		_disabledItem.stopAllActions()
	}
	
	function onMenu3()
	{
	
	}
}

////////////////////////////////////////////////////////////////////////////////

class MenuLayer4 : cc.ScriptLayer
{
	var _disabledItem = null
	
	constructor()
	{
		base.constructor()
	}
	
	function onEnter()
	{
		removeAllChildren(true)

		cc.MenuItemFont.setDefaultFontSize(18)
		cc.MenuItemFont.setDefaultFontName("American Typewriter")
		var title1 = cc.MenuItemFont("Sound")
		title1.enabled = false
		cc.MenuItemFont.setDefaultFontSize(34)
		cc.MenuItemFont.setDefaultFontName("Arial")
		var item1 = cc.MenuItemToggle(this, onMenu, 
				cc.MenuItemFont("On"), cc.MenuItemFont("Off"))
		
		cc.MenuItemFont.setDefaultFontSize(18)
		cc.MenuItemFont.setDefaultFontName("American Typewriter")
		var title2 = cc.MenuItemFont("Music")
		title2.enabled = false
		cc.MenuItemFont.setDefaultFontSize(34)
		cc.MenuItemFont.setDefaultFontName("Arial")
		var item2 = cc.MenuItemToggle(this, onMenu, 
				cc.MenuItemFont("On"), cc.MenuItemFont("Off"))
				
		cc.MenuItemFont.setDefaultFontSize(18)
		cc.MenuItemFont.setDefaultFontName("American Typewriter")
		var title3 = cc.MenuItemFont("Quality")
		title3.enabled = false
		cc.MenuItemFont.setDefaultFontSize(34)
		cc.MenuItemFont.setDefaultFontName("Arial")
		var item3 = cc.MenuItemToggle(this, onMenu, 
				cc.MenuItemFont("High"), cc.MenuItemFont("Low"))
		
		cc.MenuItemFont.setDefaultFontSize(18)
		cc.MenuItemFont.setDefaultFontName("American Typewriter")
		var title4 = cc.MenuItemFont("Orientation")
		title4.enabled = false
		cc.MenuItemFont.setDefaultFontSize(34)
		cc.MenuItemFont.setDefaultFontName("Arial")
		var item4 = cc.MenuItemToggle(this, onMenu, 
				cc.MenuItemFont("Off"))
	
		// TIP: you can manipulate the items like any other CCMutableArray
		item4.subItems.push(cc.MenuItemFont("33%"))
		item4.subItems.push(cc.MenuItemFont("66%"))
		item4.subItems.push(cc.MenuItemFont("100%"))
		
		// you can change the one of the items by doing this
		print(item4.selectedIndex)
		
		cc.MenuItemFont.setDefaultFontSize(34)
		cc.MenuItemFont.setDefaultFontName("Arial")
		
		var label = cc.LabelBMFont("go back", pack.locate("bitmapFontTest3.fnt"))
		var back = cc.MenuItemLabel(label, this, backCallback)
		
		var menu = cc.Menu(title1, title2, item1, item2, 
							title3, title4, item3, item4, back)
		
		//Notice : class has no such a function.
		menu.alignItemsInColumns(2, 2, 2, 2, 1)
		addChild(menu)
	}
	
	function onMenu()
	{
	}
	
	function backCallback(sender)
	{
		parent.switchTo(0)
	}
	
	function onMenu3()
	{
	}
}

////////////////////////////////////////////////////////////////////////////////

class MenuTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var layer1 = MenuLayer1()
		var layer2 = MenuLayer2()
		var layer3 = MenuLayer3()
		var layer4 = MenuLayer4()
		
		var layer = cc.LayerMultiplex(layer1, layer2, layer3, layer4)
		addChild(layer)
		cocos.director.replaceScene(this)
	}
}

return MenuTestScene()