var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

class TestLayer : cc.ScriptLayer 
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		
		var label = cc.LabelTTF("cocos2d", "Arial", 64)
		label.position = cc.Point(x/2, y/2);
		addChild(label);
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteLayer : cc.ScriptLayer 
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		
		var sprite = cc.Sprite(pack.locate("grossini.png", "*Images*"))
		var spriteSister1 = cc.Sprite(pack.locate("grossinis_sister1.png"))
		var spriteSister2 = cc.Sprite(pack.locate("grossinis_sister2.png"))
		
		sprite.scale(1.5)
		spriteSister1.scale(1.5)
		spriteSister2.scale(1.5)
		
		sprite.position = cc.Point(x/2, y/2);
		spriteSister1.position = cc.Point(40, y/2);
		spriteSister2.position = cc.Point(x-40, y/2);
		
		var rot = cc.action.RotateBy(16, -3600)
		
		addChild(sprite);
		addChild(spriteSister1);
		addChild(spriteSister2);
		
		sprite.runAction(rot);
		
		var jump1 = cc.action.JumpBy(4, cc.Point(-400,0), 100, 4);
		var jump2 = jump1.reverse();
		var jump11 = cc.action.JumpBy(4, cc.Point(-400,0), 100, 4);
		var jump22 = jump11.reverse();
		
		var rot1 = cc.action.RotateBy(4, 360*2);
		var rot2 = rot1.reverse();
		var rot11 = cc.action.RotateBy(4, 360*2);
		var rot22 = rot1.reverse();
		
		spriteSister1.runAction(cc.action.Repeat(cc.action.Sequence(jump2,jump1), 5));
		spriteSister2.runAction(cc.action.Repeat(cc.action.Sequence(jump11,jump22), 5));
		
		spriteSister1.runAction(cc.action.Repeat(cc.action.Sequence(rot1,rot2), 5));
		spriteSister2.runAction(cc.action.Repeat(cc.action.Sequence(rot22,rot11), 5));
	}
}

////////////////////////////////////////////////////////////////////////////////

class RotateWorldMainLayer : cc.ScriptLayer 
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		
		var blue = cc.LayerColor(nit.Color(0,0,255/255, 255/255));
		var red = cc.LayerColor(nit.Color(255/255,0,0,255/255));
		var green = cc.LayerColor(nit.Color(0,255/255,0,255/255));
		var white = cc.LayerColor(nit.Color(255/255,255/255,255/255,255/255));
		
		blue.scale(0.5);
		blue.position = cc.Point(-x/4, -y/4);
		blue.addChild(SpriteLayer());
		
		red.scale(0.5);
		red.position = cc.Point(x/4, -y/4);
		
		green.scale(0.5);
		green.position = cc.Point(-x/4, y/4);
		green.addChild(TestLayer());
		
		white.scale(0.5);
		white.position = cc.Point(x/4, y/4);
		
		addChild(blue, -1)
		addChild(white)
		addChild(green)
		addChild(red)

		var rot = cc.action.RotateBy(8, 720);
		var rot1 = cc.action.RotateBy(8, 720);
		var rot2 = cc.action.RotateBy(8, 720);
		var rot3 = cc.action.RotateBy(8, 720);
		
		blue.runAction(rot);
		red.runAction(rot1);
		green.runAction(rot2);
		white.runAction(rot3);
	}
}

////////////////////////////////////////////////////////////////////////////////

class CocosRotateWorldTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = RotateWorldMainLayer()
		addChild(pLayer);
		runAction(cc.action.RotateBy(4, -360))
		cocos.director.replaceScene(this)
	}
}

return CocosRotateWorldTestScene()