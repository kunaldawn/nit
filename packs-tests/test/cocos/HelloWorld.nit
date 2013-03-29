print("loading: ")
dump(script.unit)

var title = "UTF-8 다국어 표시 테스트"
var msg = "[kr] 이하 문장은 2011.12.09 yahoo에서 발췌함:\n"
msg += "[jp] 【社会的な活動】藤川球児ら阪神タイガース選手がチャリティー出品\n"
msg += "[cn] 邵逸夫将卸任TVB董事局主席 企业常青靠坚持\n"
msg += "[de] Röttgen: Klima-Allianz mit Entwicklungsländern in Durban\n"
msg += "[ru] Московская полиция будет пресекать любые несанкционированные акции\n"
msg += "[it] La forza nella stretta di mano è uno dei fattori più indicativi sullo stato di salute in età avanzata.\n"
msg += "[fr] François Fillon réaffirme son hostilité au vote des étrangers\n"
msg += "[es] Rajoy dice España necesita acometer reformas estructurales ¿Recuerdas a Carol Seaver, de 'Los problemas crecen'?\n"

//app.runtime.alert(title, msg)

var pack = script.locator

dump(pack)

class TouchNode : cc.ScriptNode
{
}

class HelloWorld : cc.ScriptLayer
{
	constructor()
	{
		//////////////////////////////
		// 1. super init first
		base.constructor()

		/////////////////////////////
		// 2. add a menu item with "X" image, which is clicked to quit the program
		//    you may modify it.
		
		var closeItem = cc.MenuItemImage(
			pack.locate("CloseNormal.png"),
			pack.locate("CloseSelected.png"),
			this, onCloseMenu)
			
		closeItem.position = cc.Point(cocos.director.winSize.width - 20, 20)
		
		var menu = cc.Menu(closeItem)
		menu.position = cc.Point(0, 0)
		
		addChild(menu, 1)
		
		/////////////////////////////
		// 3. add your codes below...

		// add a label shows "Hello World"
		// create and initialize a label
		var label = cc.LabelTTF("Hello World", "Arial", 24)
		var size = cocos.director.winSize
		
		// position the label on the center of the screen
		label.position = cc.Point(size.width / 2, size.height - 50)
		
		// add the label as a child to this layer
		addChild(label, 1)

		// add "HelloWorld" splash screen
		var spriteback = cc.Sprite(pack.locate("HelloWorld.png")) with
		{
			position = cc.Point(size.width / 2, size.height / 2)
		}
		addChild(spriteback, 0)
		
		// add fire particle on screen
		var particle = cc.particle.Fire()
		particle.texture = cocos.textureCache.addImage(pack.locate("fire.png"))
		
		particle.position = cc.Point(size.width / 3, size.height / 2)
		
		// and add it to a touch node
		var touch = TouchNode()
		touch.addChild(particle)
		
		::tn := touch // remember in global table

		var upvalue = "hello"
		var m = cc.action.MoveBy(0.5, cc.Point(100, 100))
		particle.runAction(
			cc.action.Sequence(
				m,
				m.reverse(),
				cc.action.EventCall(cc.MenuItemEvent(closeItem), this, onActionEvent),
				cc.action.EventCall(null, null, @{print(upvalue)})
			)
		)
		
		addChild(touch, 9)
	}
	
	destructor()
	{
		print(this + " destroyed")
	}
	
	function OnEnter()
	{
		print(this + ": onEnter")

		// detect session closing
		session.channel().bind(EVT.SESSION_STOP, this) by { print("pop"); cocos.director.PopScene() }
	}
	
	function OnExit()
	{
		print(this + ": onExit")
		
		session.channel().unbind(EVT.SESSION_STOP, this)
	}
		
	function onActionEvent(evt)
	{
		print(this + ": onActionEvent: " + evt)
	}
	
	function onCloseMenu(evt: cc.MenuItemEvent)
	{
		print(this + ": onCloseMenu : " + evt)
		session.restart()
	}
}

s := cc.ScriptScene()
h := HelloWorld()
s.addChild(h)

cocos.director.pushScene(s)