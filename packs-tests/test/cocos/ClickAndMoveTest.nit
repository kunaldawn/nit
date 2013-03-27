var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

class MainLayer : cc.ScriptLayer
{
	constructor()
	{
		
		base.constructor()
	
		var sprite = cc.Sprite(pack.locate("grossini.png",  "*Images*"))
		
		var layer = cc.LayerColor(nit.Color(0xFFFFFF00));
		addChild(layer, -1)
		addChild(sprite, 0, 1);
		
		sprite.position = cc.Point(20, 150);
		sprite.runAction(cc.action.JumpTo(4,cc.Point(300,48), 100, 4));
		layer.runAction(
			cc.action.RepeatForever(
				cc.action.Sequence(cc.action.FadeIn(1),cc.action.FadeOut(1))
			) 
		)
		
		touchEnabled = true

		channel().bind(Events.OnCCTouchEnded, this, onTouchEnded)
		
	}
	
	function onTouchEnded(evt: cc.TouchEvent)
	{
		var touch = evt.touch0.locationInView(evt.touch0.view)
		var convertedLocation = cocos.director.toGl(touch)

		var s = getChildByTag(1);
		s.stopAllActions();
		s.runAction(cc.action.MoveTo(1, convertedLocation))
		var o = touch.x - s.position.x;
		var a = touch.x - s.position.x;
		var at =  math.deg(math.atan(o/a));
		if (a < 0)
		{
			if (o < 0)
				at = 180 + math.fabs(at);
			else
				at = 180 - math.fabs(at);
		}
		s.runAction(cc.action.RotateTo(1, at));
	}
}

////////////////////////////////////////////////////////////////////////////////

class CocosClickAndMoveTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		addChild(MainLayer());
		cocos.director.replaceScene(this)
	}
}

return CocosClickAndMoveTestScene()