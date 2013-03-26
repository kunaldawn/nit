var pack = script.locator

class CocosSceneTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = SceneTestLayer1()
		addChild(pLayer );
		cocos.director.replaceScene(this)
	}
}

class SceneTestLayer1 : cc.ScriptLayer 
{
	constructor()
	{
		
		base.constructor()
		
		
		var item1 = cc.MenuItemFont("Test pushScene", this, onpushScene );
		var item2 = cc.MenuItemFont("Test pushScene w/transition", this, onpushSceneTran );
		var item3 = cc.MenuItemFont("Quit", this, onQuit );
		
		var menu = cc.Menu( item1, item2, item3)
		menu.alignItemsVertically();
		this.addChild( menu)
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		
		var sprite = cc.Sprite(pack.locate("grossini.png", "*Images*"))
		sprite.position = cc.Point(x-40, y/2);
		this.addChild(sprite);
		
		var rot = cc.action.RotateBy(2, 360)
		sprite.runAction(cc.action.RepeatForever( rot) );
	
	
	}
	
	function onpushScene( Sender)
	{
		var scene = CocosSceneTestScene()
		var layer = SceneTestLayer2()
		scene.addChild( layer, 0)
		cocos.director.pushScene( scene)
	}
	
	function onpushSceneTran( Sender)
	{
		var scene = CocosSceneTestScene()
		var layer = SceneTestLayer2()
		scene.addChild( layer, 0)
		cocos.director.pushScene(cc.transition.SlideInT(1, scene) )
	}
	function onQuit( Sender)
	{
	}
}


class SceneTestLayer2 : cc.ScriptLayer 
{
	m_timeCounter = 0;
	constructor()
	{
		
		base.constructor()
		
		
		var item1 = cc.MenuItemFont("replaceScene", this, onReplaceScene );
		var item2 = cc.MenuItemFont("replaceScene w/transition", this, onReplaceSceneTran );
		var item3 = cc.MenuItemFont("Go Back", this, onGoBack );
		
		var menu = cc.Menu( item1, item2, item3)
		menu.alignItemsVertically()
		this.addChild( menu)
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		
		var sprite = cc.Sprite(pack.locate("grossini.png", "*Images*"))
		sprite.position = cc.Point(x-40, y/2);
		this.addChild(sprite);
		
		var rot = cc.action.RotateBy(2, 360)
		sprite.runAction(cc.action.RepeatForever( rot) );
	
	
	}
	
	function onGoBack(Sender)
	{
		cocos.director.popScene()
	}
	
	function onReplaceScene( Sender)
	{
		var scene = CocosSceneTestScene()
		var layer = SceneTestLayer3()
		scene.addChild( layer, 0)
		cocos.director.replaceScene( scene)
	}
	function onReplaceSceneTran( Sender)
	{
		var scene = CocosSceneTestScene()
		var layer = SceneTestLayer3()
		scene.addChild( layer, 0)
		cocos.director.replaceScene(cc.transition.FlipX(2, scene) )
	}
}



class SceneTestLayer3 : cc.ScriptLayer 
{
	
	constructor()
	{
		
		base.constructor()
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		
		this.touchEnabled=true;
		
		var label = cc.LabelTTF("Touch to popScene","Marker Felt", 28)
		addChild( label)
		label.position =cc.Point(x/2, y/2) 
	
		var sprite = cc.Sprite(pack.locate("grossini.png", "*Images*"))
		sprite.position = cc.Point(x-40, y/2);
		this.addChild(sprite);
		
		var rot = cc.action.RotateBy(2, 360)
		sprite.runAction(cc.action.RepeatForever( rot) );
	
		channel().bind(Events.OnCCTouchEnded, this, onTouchEnded)
	
	}
	
	function onGoBack(Sender)
	{
		cocos.director.popScene()
	}
	
	function onTouchEnded(evt: cc.TouchEvent)
	{
		cocos.director.popScene()
	}
}