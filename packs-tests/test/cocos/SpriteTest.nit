var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var TAG = 
{
	TILE_MAP 		= 1
	SPRITE_BATCH_NODE = 1
	NODE 			= 2
	ANIMATION1 		= 1
	SPRITE_LEFT		= 2
	SPRITE_RIGHT	= 3

	SPRITE1 = 0
	SPRITE2	= 1
	SPRITE3	= 2
	SPRITE4	= 3
	SPRITE5	= 4
	SPRITE6	= 5
	SPRITE7	= 6
	SPRITE8	= 7
}

var IDC_NEXT 	= 100
var IDC_BACK	= 101
var IDC_RESTART = 102

var MAX_LAYER = 48
var sceneIdx = -1

////////////////////////////////////////////////////////////////////////////////

var function createSpriteTestLayer(nIndex)
{
	switch (nIndex)
	{
		case 0: return  Sprite1();
        case 1: return  SpriteBatchNode1();
        case 2: return  SpriteFrameTest();
        case 3: return  SpriteFrameAliasNameTest();
        case 4: return  SpriteAnchorPoint();
        case 5: return  SpriteBatchNodeAnchorPoint();
        case 6: return  SpriteOffsetAnchorRotation();
        case 7: return  SpriteBatchNodeOffsetAnchorRotation();
        case 8: return  SpriteOffsetAnchorScale();
        case 9: return  SpriteBatchNodeOffsetAnchorScale();
        case 10: return  SpriteAnimationSplit();
        case 11: return  Spritecoloropacity();
        case 12: return  SpriteBatchNodecoloropacity();
        case 13: return  SpriteZOrder();
        case 14: return  SpriteBatchNodeZOrder();
        case 15: return  SpriteBatchNodeReorder();
        case 16: return  SpriteBatchNodeReorderIssue744();
        case 17: return  SpriteBatchNodeReorderIssue766();
        case 18: return  SpriteBatchNodeReorderIssue767();
        case 19: return  SpriteZVertex();
        case 20: return  SpriteBatchNodeZVertex();
        case 21: return  Sprite6();
        case 22: return  SpriteFlip();
        case 23: return  SpriteBatchNodeFlip();
        case 24: return  SpriteAliased();
        case 25: return  SpriteBatchNodeAliased();
        case 26: return  SpriteNewTexture();
        case 27: return  SpriteBatchNodeNewTexture();
        case 28: return  SpriteHybrid();
        case 29: return  SpriteBatchNodeChildren();
        case 30: return  SpriteBatchNodeChildren2();
        case 31: return  SpriteBatchNodeChildrenZ();
        case 32: return  SpriteChildrenVisibility();
        case 33: return  SpriteChildrenVisibilityIssue665();
        case 34: return  SpriteChildrenAnchorPoint();
        case 35: return  SpriteBatchNodeChildrenAnchorPoint();
        case 36: return  SpriteBatchNodeChildrenScale();
        case 37: return  SpriteChildrenChildren();
        case 38: return  SpriteBatchNodeChildrenChildren();
        case 39: return  SpriteNilTexture();
        case 40: return  SpriteSubclass();
        case 41: return  AnimationCache();
		case 42: return  SpriteOffsetAnchorSkew();
		case 43: return  SpriteBatchNodeOffsetAnchorSkew();
		case 44: return  SpriteOffsetAnchorSkewScale();
		case 45: return  SpriteBatchNodeOffsetAnchorSkewScale();
		case 46: return  SpriteOffsetAnchorFlip();
		case 47: return  SpriteBatchNodeOffsetAnchorFlip();
	}
}

var function nextSpriteTestAction()
{
    sceneIdx++;
    sceneIdx = sceneIdx % MAX_LAYER;

    var pLayer = createSpriteTestLayer(sceneIdx);
    return pLayer;
}

var function backSpriteTestAction()
{
    sceneIdx--
    var total = MAX_LAYER
    if (sceneIdx < 0)
        sceneIdx += total;    
    
    var pLayer = createSpriteTestLayer(sceneIdx);
    return pLayer;
}

var function restartSpriteTestAction()
{
    var pLayer = createSpriteTestLayer(sceneIdx);
    return pLayer;
} 

////////////////////////////////////////////////////////////////////////////////

class SpriteTestDemo : cc.ScriptLayer
{
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		
		var label = cc.LabelTTF(title(), "Arial", 28)
		label.position = cc.Point(x / 2, y - 50)
		addChild(label, 1)
		
		var strSubtitle = subtitle()
		if (strSubtitle != "")
		{
			var l = cc.LabelTTF(strSubtitle, "Arial", 16)
			addChild(l, 1)
			l.position = cc.Point(x / 2, y - 80)
		}
		
		var item1 = cc.MenuItemImage(IMG.B1,IMG.B2, this,backCallBack);
		var item2 = cc.MenuItemImage(IMG.R1, IMG.R2, this,restartCallBack);		
		var item3 = cc.MenuItemImage(IMG.F1, IMG.F2, this,nextCallBack);
		
		var menu = cc.Menu(item1, item2, item3);
		menu.position= cc.Point(0,0);
		item1.position= cc.Point(size.width/2 - 100,30) ;
		item2.position= cc.Point(size.width/2, 30) ;
		item3.position= cc.Point(size.width/2 + 100,30) ;
		addChild(menu, 1);
	}
	
	function title()
	{
		return "NO title"
	}
	
	function subtitle()
	{
		return "";
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		var s = SpriteTestScene()
		s.addChild(restartSpriteTestAction())
		cocos.director.replaceScene(s)
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = SpriteTestScene()
		s.addChild(nextSpriteTestAction())
		cocos.director.replaceScene(s)
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		var s = SpriteTestScene()
		s.addChild(backSpriteTestAction())
		cocos.director.replaceScene(s)
	}
}

////////////////////////////////////////////////////////////////////////////////

class Sprite1 : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		touchEnabled = true
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		addNewSpriteWithCoords(cc.Point(x/2, y/2))
		
		channel().bind(Events.OnCCTouchEnded, this, ccTouchesEnded)
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	function addNewSpriteWithCoords(p)
	{
		var idx = math.random() * 14;
		var x = (idx mod 5) * 85;
		var y = (idx div 5) * 125;
		
		var sprite = cc.Sprite(pack.locate("grossini_dance_atlas.png"), 
											cc.Rect(x,y,85,125));
		addChild(sprite);
		sprite.position = cc.Point(p.x, p.y);
		
		var action = null;
		var random = math.random()
		
		if (random < 0.20)
			action = cc.action.ScaleBy(3, 2);
		else if (random < 0.40)
			action = cc.action.RotateBy(3, 360);
		else if (random < 0.60)
			action = cc.action.Blink(1, 3);
		else if (random < 0.8)
			action = cc.action.TintBy(2, 0, -255, -255);
		else 
			action = cc.action.FadeOut(2);
			
		var action_back = action.reverse();
		var seq = cc.action.Sequence(action, action_back);
		
		sprite.runAction(cc.action.RepeatForever(seq))
	}
	
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		foreach (touch in evt.touches)
		{
			var location = touch.locationInView(touch.view)
			location = cocos.director.toGl(location);
		
			addNewSpriteWithCoords(location);
		}
	}

	function title()
	{
		return "Sprite (tap screen)"
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNode1 : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		touchEnabled = true
		
		var BatchNode = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"), 50);
		addChild(BatchNode, 0, TAG.SPRITE_BATCH_NODE)
		
		var size = cocos.director.winSize;
		var x = size.width;
		var y = size.height;
		addNewSpriteWithCoords(cc.Point(x/2, y/2))
		
		channel().bind(Events.OnCCTouchEnded, this, ccTouchesEnded)
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	function addNewSpriteWithCoords(p)
	{
		var BatchNode = getChildByTag(TAG.SPRITE_BATCH_NODE)
	
		var idx = math.random() * 14;
		var x = (idx mod 5) * 85;
		var y = (idx div 5) * 125;
		
		var sprite = cc.Sprite(BatchNode.texture, 
											cc.Rect(x,y,85,125));
		addChild(sprite);
		sprite.position = cc.Point(p.x, p.y);
		
		var action = null;
		var random = math.random()
		
		if (random < 0.20)
			action = cc.action.ScaleBy(3, 2);
		else if (random < 0.40)
			action = cc.action.RotateBy(3, 360);
		else if (random < 0.60)
			action = cc.action.Blink(1, 3);
		else if (random < 0.8)
			action = cc.action.TintBy(2, 0, -255, -255);
		else 
			action = cc.action.FadeOut(2);
			
		var action_back = action.reverse();
		var seq = cc.action.Sequence(action, action_back);
		
		sprite.runAction(cc.action.RepeatForever(seq))
	}
	
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		foreach (touch in evt.touches)
		{
			var location = touch.locationInView(touch.view)
			location = cocos.director.toGl(location);
		
			addNewSpriteWithCoords(location);
		}
	}

	function title()
	{
		return "SpriteBatchNode (tap screen)"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Spritecoloropacity : SpriteTestDemo
{
	var _update = null
	
	constructor()
	{
		base.constructor()
		
		var sprite1 = cc.Sprite(pack.locate("grossini_dance_atlas.png"),
					cc.Rect(85*0, 121*1, 85, 121))
		var sprite2 = cc.Sprite(pack.locate("grossini_dance_atlas.png"),
					cc.Rect(85*1, 121*1, 85, 121));
		var sprite3 = cc.Sprite(pack.locate("grossini_dance_atlas.png"),
					cc.Rect(85*2, 121*1, 85, 121));
		var sprite4 = cc.Sprite(pack.locate("grossini_dance_atlas.png"),
					cc.Rect(85*3, 121*1, 85, 121));
		var sprite5 = cc.Sprite(pack.locate("grossini_dance_atlas.png"),
					cc.Rect(85*0, 121*1, 85, 121));
		var sprite6 = cc.Sprite(pack.locate("grossini_dance_atlas.png"),
					cc.Rect(85*1, 121*1, 85, 121));
		var sprite7 = cc.Sprite(pack.locate("grossini_dance_atlas.png"),
					cc.Rect(85*2, 121*1, 85, 121));
		var sprite8 = cc.Sprite(pack.locate("grossini_dance_atlas.png"),
					cc.Rect(85*3, 121*1, 85, 121));
		
		var s = cocos.director.winSize;
		sprite1.position = cc.Point((s.width/5)*1, (s.height/3)*1)
		sprite2.position = cc.Point((s.width/5)*2, (s.height/3)*1) 
		sprite3.position = cc.Point((s.width/5)*3, (s.height/3)*1) 
		sprite4.position = cc.Point((s.width/5)*4, (s.height/3)*1) 
		sprite5.position = cc.Point((s.width/5)*1, (s.height/3)*2)
		sprite6.position = cc.Point((s.width/5)*2, (s.height/3)*2) 
		sprite7.position = cc.Point((s.width/5)*3, (s.height/3)*2)
		sprite8.position = cc.Point((s.width/5)*4, (s.height/3)*2)
		
		var action = cc.action.FadeIn(2)
		var action_back = action.reverse()
		var fade = cc.action.RepeatForever(cc.action.Sequence(action, action_back))
		
		var tintred = cc.action.TintBy(2, 0, -255, -255)
		var tintred_back = tintred.reverse()
		var red = cc.action.RepeatForever(cc.action.Sequence(tintred, tintred_back))
		
		var tintgreen = cc.action.TintBy(2, -255, 0, -255)
		var tintgreen_back = tintgreen.reverse()
		var green = cc.action.RepeatForever(cc.action.Sequence(tintgreen, tintgreen_back))
		
		var tintblue = cc.action.TintBy(2, -255, -255, 0)
		var tintblue_back = tintblue.reverse()
		var blue = cc.action.RepeatForever(cc.action.Sequence(tintblue, tintblue_back))
		
		sprite5.runAction(red);
		sprite6.runAction(green);
		sprite7.runAction(blue);
		sprite8.runAction(fade);
		
		addChild(sprite1, 0, TAG.SPRITE1);
		addChild(sprite2, 0, TAG.SPRITE2);
		addChild(sprite3, 0, TAG.SPRITE3);
		addChild(sprite4, 0, TAG.SPRITE4);
		addChild(sprite5, 0, TAG.SPRITE5);
		addChild(sprite6, 0, TAG.SPRITE6);
		addChild(sprite7, 0, TAG.SPRITE7);
		addChild(sprite8, 0, TAG.SPRITE8);
		
		_update = cocos.director.scheduler.repeat(this, removeAndAddSprite, 2.0)
	}
	
	function removeAndAddSprite()
	{
		var sprite = getChildByTag(TAG.SPRITE5)
		removeChild(sprite, false)
		addChild(sprite, 0, TAG.SPRITE5)
	}

	function title()
	{
		return "Sprite: color & opacity"
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodecoloropacity : SpriteTestDemo
{
	var _update = null
	
	constructor()
	{
		base.constructor()
		
		var  batch = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"), 1);
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE)
		
		var sprite1 = cc.Sprite(batch.texture, cc.Rect(85*0, 121*1, 85, 121))
		var sprite2 = cc.Sprite(batch.texture, cc.Rect(85*1, 121*1, 85, 121));
		var sprite3 = cc.Sprite(batch.texture, cc.Rect(85*2, 121*1, 85, 121));
		var sprite4 = cc.Sprite(batch.texture, cc.Rect(85*3, 121*1, 85, 121));
		var sprite5 = cc.Sprite(batch.texture, cc.Rect(85*0, 121*1, 85, 121));
		var sprite6 = cc.Sprite(batch.texture, cc.Rect(85*1, 121*1, 85, 121));
		var sprite7 = cc.Sprite(batch.texture, cc.Rect(85*2, 121*1, 85, 121));
		var sprite8 = cc.Sprite(batch.texture, cc.Rect(85*3, 121*1, 85, 121));
		
		var s = cocos.director.winSize;
		sprite1.position = cc.Point((s.width/5)*1, (s.height/3)*1)
		sprite2.position = cc.Point((s.width/5)*2, (s.height/3)*1) 
		sprite3.position = cc.Point((s.width/5)*3, (s.height/3)*1) 
		sprite4.position = cc.Point((s.width/5)*4, (s.height/3)*1) 
		sprite5.position = cc.Point((s.width/5)*1, (s.height/3)*2)
		sprite6.position = cc.Point((s.width/5)*2, (s.height/3)*2) 
		sprite7.position = cc.Point((s.width/5)*3, (s.height/3)*2)
		sprite8.position = cc.Point((s.width/5)*4, (s.height/3)*2)
		
		var action = cc.action.FadeIn(2)
		var action_back = action.reverse()
		var fade = cc.action.RepeatForever(cc.action.Sequence(action, action_back))
		
		var tintred = cc.action.TintBy(2, 0, -255, -255)
		var tintred_back = tintred.reverse()
		var red = cc.action.RepeatForever(cc.action.Sequence(tintred, tintred_back))
		
		var tintgreen = cc.action.TintBy(2, -255, 0, -255)
		var tintgreen_back = tintgreen.reverse()
		var green = cc.action.RepeatForever(cc.action.Sequence(tintgreen, tintgreen_back))
		
		var tintblue = cc.action.TintBy(2, -255, -255, 0)
		var tintblue_back = tintblue.reverse()
		var blue = cc.action.RepeatForever(cc.action.Sequence(tintblue, tintblue_back))
		
		sprite5.runAction(red);
		sprite6.runAction(green);
		sprite7.runAction(blue);
		sprite8.runAction(fade);
		
		batch.addChild(sprite1, 0, TAG.SPRITE1);
		batch.addChild(sprite2, 0, TAG.SPRITE2);
		batch.addChild(sprite3, 0, TAG.SPRITE3);
		batch.addChild(sprite4, 0, TAG.SPRITE4);
		batch.addChild(sprite5, 0, TAG.SPRITE5);
		batch.addChild(sprite6, 0, TAG.SPRITE6);
		batch.addChild(sprite7, 0, TAG.SPRITE7);
		batch.addChild(sprite8, 0, TAG.SPRITE8);
		
		_update = cocos.director.scheduler.repeat(this, removeAndAddSprite, 2.0)
	}
	
	function removeAndAddSprite()
	{
		var batch = getChildByTag(TAG.SPRITE_BATCH_NODE)
		var sprite = batch.getChildByTag(TAG.SPRITE5)
		batch.removeChild(sprite, false)
		batch.addChild(sprite, 0, TAG.SPRITE5)
	}

	function title()
	{
		return "SpriteBatchNode: color & opacity"
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteZOrder : SpriteTestDemo
{
	var _dir = 1;
	
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize;
		var step = s.width/11;
		
		for (var i = 0; i< 5; i++)
		{
			var sprite = cc.Sprite(pack.locate("grossini_dance_atlas.png"), cc.Rect(85*0, 121*1, 85, 121))
			sprite.position = cc.Point((i+1)*step, s.height/2)
			addChild(sprite, i)
		}
		
		for (var i = 5;  i<10; i++)
		{
			var sprite = cc.Sprite(pack.locate("grossini_dance_atlas.png"), cc.Rect(85*0, 121*0, 85, 121))
			sprite.position = cc.Point((i+1)*step, s.height/2)
			addChild(sprite, 14-i)
		}
		
		var sprite = cc.Sprite(pack.locate("grossini_dance_atlas.png"), cc.Rect(85*3, 121*0, 85, 121))
		addChild(sprite, -1, 0)
		sprite.position = cc.Point(s.width/2, s.height/2 - 20)
		sprite.scaleX = 6
		sprite.color = nit.Color.RED
		
		cocos.director.scheduler.repeat(this, reorderSprite, 1)
	}
	
	function reorderSprite(dt)
	{
		var sprite = getChildByTag(0)
		var z = sprite.zOrder;
		
		if (z < -1)
			_dir = 1;
		if (z > 10)
			_dir = -1
			
		z += _dir * 3
		
		reorderChild(sprite, z)
	}
	
	function title()
	{
		return "Sprite: Z order";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeReorder : SpriteTestDemo
{
	var _dir = 1;
	
	constructor()
	{
		base.constructor()
		 // small capacity. Testing resizing.
		// Don't use capacity=1 in your real game. It is expensive to resize the capacity
		var a = []
		var asmtest = cc.SpriteBatchNode(pack.locate("ghosts.png"))
		//addChild(batch, 0, TAG.SPRITE_BATCH_NODE)
	
		var s = cocos.director.winSize;
		var step = s.width/11;
		
		for (var i = 0; i< 10; i++)
		{
			var s1 = cc.Sprite(asmtest, cc.Rect(0,0,50,50))
			a.push(s1)
			asmtest.addChild(s1, 10)
		}
		
		for (var i = 0; i< 10; i++)
		{
			if (i!=5)
			{
				asmtest.reorderChild(a[i], 9)
			}
		}
		
		var prev = -1
		var children = asmtest.children
		var child = null
		foreach (k,v in children)
		{
			child = v
			if (!child)
				break;
			var currentIndex = child.atlasIndex
			prev = currentIndex
		}
		prev = -1
		var sChildren = asmtest.getDescendants()
		
		foreach (k, v in sChildren)
		{
			child = v
			if (!child)
				break;
			var currentIndex = child.atlasIndex
			prev = currentIndex
		}
	}
	
	function title()
	{
		return  "SpriteBatchNode: reorder #1";
	}
	
	function subtitle()
	{
		return "Should not crash";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeZOrder : SpriteTestDemo
{
	var _dir = 1;
	
	constructor()
	{
		base.constructor()
		
		 // small capacity. Testing resizing.
		// Don't use capacity=1 in your real game. It is expensive to resize the capacity
		var batch = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"), 1)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE)
	
		var s = cocos.director.winSize;
		var step = s.width/11;
		
		for (var i = 0; i< 5; i++)
		{
			var sprite = cc.Sprite(batch.texture, cc.Rect(85*0, 121*1, 85, 121))
			sprite.position = cc.Point((i+1)*step, s.height/2)
			batch.addChild(sprite, i)
		}
		
		for (var i = 5;  i<10; i++)
		{
			var sprite = cc.Sprite(batch.texture, cc.Rect(85*0, 121*0, 85, 121))
			sprite.position = cc.Point((i+1)*step, s.height/2)
			batch.addChild(sprite, 14-i)
		}
		
		var sprite = cc.Sprite(batch.texture, cc.Rect(85*3, 121*0, 85, 121))
		batch.addChild(sprite, -1, TAG.SPRITE1)
		sprite.position = cc.Point(s.width/2, s.height/2 - 20)
		sprite.scaleX = 6
		sprite.color = nit.Color.RED
		
		cocos.director.scheduler.repeat(this, reorderSprite, 1)
	}
	
	function reorderSprite()
	{
		var batch = getChildByTag(TAG.SPRITE_BATCH_NODE)
		var sprite = batch.getChildByTag(TAG.SPRITE1)
		var z = sprite.zOrder;
		
		if (z < -1)
			_dir = 1;
		if (z > 10)
			_dir = -1
			
		z += _dir * 3
		
		batch.reorderChild(sprite, z)
	}
	
	function title()
	{
		return "SpriteBatchNode: Z order";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeReorderIssue744 : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		// Testing issue #744
		// http://code.google.com/p/cocos2d-iphone/issues/detail?id=744
		var batch = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"), 15)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE);
		
		var s = cocos.director.winSize;
		var sprite = cc.Sprite(batch, cc.Rect(0, 0, 85, 121))
		sprite.position = cc.Point(s.width/2, s.height/2) 
		batch.addChild(sprite, 3)
		batch.reorderChild(sprite, 1)
	}
	
	function title()
	{
		return "SpriteBatchNode: reorder issue #744";
	}

	function subtitle()
	{
		return "Should not crash";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeReorderIssue766 : SpriteTestDemo
{
	var _batchNode = null
	var _sprite1 = null
	
	constructor()
	{
		base.constructor()
		
		_batchNode = cc.SpriteBatchNode(pack.locate("piece.png"), 15)
		addChild(_batchNode, 1, 0);
		
		_sprite1 =  makeSpriteZ(2)
		_sprite1.position = cc.Point(200, 160) 
		
		var sprite2 =  makeSpriteZ(3)
		sprite2.position = cc.Point(264, 160) 
		
		var sprite3 =  makeSpriteZ(4)
		sprite3.position = cc.Point(328, 160) 
		
		cocos.director.scheduler.once(this, reorderSprite, 2)
	}
	
	function makeSpriteZ(aZ)
	{
		var sprite = cc.Sprite(_batchNode, cc.Rect(128, 0, 64, 64))
		_batchNode.addChild(sprite, aZ+1, 0)
		
		var spriteShadow = cc.Sprite(_batchNode, cc.Rect(0, 0, 64, 64))
		spriteShadow.opacity = 128;
		sprite.addChild(spriteShadow, aZ, 3)
		
		var spriteTop = cc.Sprite(_batchNode, cc.Rect(64, 0, 64, 64))
		sprite.addChild(spriteTop, aZ+2, 3)
		
		return sprite;
	}
	
	function reorderSprite()
	{
		_batchNode.reorderChild(_sprite1, 4);
	}
	
	function title()
	{
		return "SpriteBatchNode: reorder issue #766";
	}

	function subtitle()
	{
		return "In 2 seconds 1 sprite will be reordered";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeReorderIssue767 : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
	
		var s = cocos.director.winSize;
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("ghosts.plist"),"ghosts.png")
		
		var aParent = null
		var l1, l2a, l2b, l3a1, l3a2, l3b1, l3b2 = null
		//
		// SpriteBatchNode: 3 levels of children
		//
		aParent = cc.SpriteBatchNode(pack.locate("ghosts.png"))
		addChild(aParent, 0, TAG.SPRITE1)
		
		//parent
		l1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("father.gif"))
		l1.position = cc.Point(s.width/2, s.height/2)
		aParent.addChild(l1, 0, TAG.SPRITE2)
		var l1Size = l1.contentSize;
		
		//child left
		l2a = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("sister1.gif"))
		l2a.position = cc.Point(-25 + l1Size.width/2, 0 + l1Size.height/2)
		l1.addChild(l2a, -1,  TAG.SPRITE_LEFT)
		var l2aSize = l2a.contentSize
		
		//child right
		l2b = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("sister2.gif"))
		l2b.position = cc.Point(25 + l1Size.width/2, 0 + l1Size.height/2)
		l1.addChild(l2b, 1,  TAG.SPRITE_RIGHT)
		var l2bSize = l2b.contentSize
		
		//child left bottom
		l3a1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3a1.scale(0.65)
		l3a1.position = cc.Point(0 + l2aSize.width/2, -50 + l2aSize.height/2)
		l2a.addChild(l3a1, -1)
		
		//child left top
		l3a2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3a2.scale(0.65)
		l3a2.position = cc.Point(l2aSize.width/2, 50 + l2aSize.height/2)
		l2a.addChild(l3a2, 1)
		
		//child right bottom
		l3a1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3a1.position = cc.Point(l2bSize.width/2, -50 + l2bSize.height/2)
		l3a1.scale(0.65)
		l2b.addChild(l3a1, -1)
		var l2aSize = l3a1.contentSize
		
		//child right top
		l3b2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3b2.scale(0.65)
		l3b2.position = cc.Point(l2bSize.width/2, 50 + l2bSize.height/2)
		l2b.addChild(l3b2, 1)
		var l2aSize = l2a.contentSize
		
		cocos.director.scheduler.repeat(this, reorderSprites, 1)
	}
	
	function reorderSprites(dt)
	{
		var spritebatch = getChildByTag(TAG.SPRITE1)
		var father  = spritebatch.getChildByTag(TAG.SPRITE2)
		var left = father.getChildByTag(TAG.SPRITE_LEFT)
		var right = father.getChildByTag(TAG.SPRITE_RIGHT)
		
		var newZLeft = 1
		
		if (left.zOrder == 1)
			newZLeft = -1
			
		father.reorderChild(left, newZLeft)
		father.reorderChild(right, -newZLeft)
	}
	
	function title()
	{
		return "SpriteBatchNode: reorder issue #767";
	}

	function subtitle()
	{
		return "Should not crash";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteZVertex : SpriteTestDemo
{
	var _dir = 1
	var _time = 0
	
	constructor()
	{
		base.constructor()
		//
		// This test tests z-order
		// If you are going to use it is better to use a 3D projection
		//
		// WARNING:
		// The developer is resposible for ordering it's sprites according to it's Z if the sprite has
		// transparent parts.
		//
		var s = cocos.director.winSize
		var step = s.width/12
		
		var  node = cc.Node()
		node.contentSize = cc.Size(s.width, s.height)
		node.anchorPoint = cc.Point(0.5, 0.5)
		node.position = cc.Point(s.width/2, s.height/2)
		addChild(node, 0)
		
		for (var i=0; i<5; i++)
		{
			var sprite = cc.Sprite(pack.locate("grossini_dance_atlas.png"), cc.Rect(0, 121, 85, 121))
			sprite.position = cc.Point((i+1)*step, s.height/2)
			sprite.vertexZ = 10 + i*40
			node.addChild(sprite, 0)
		}
		for (var i=5; i<11; i++)
		{
			var sprite = cc.Sprite(pack.locate("grossini_dance_atlas.png"), cc.Rect(85, 0, 85, 121))
			sprite.position = cc.Point((i+1)*step, s.height/2)
			sprite.vertexZ = 10 + (10-i)*40
			node.addChild(sprite, 0)
		}
		
		node.runAction(cc.action.OrbitCamera(10, 1, 0, 0, 360, 0, 0))
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
		// TIP: don't forget to enable Alpha test
		
		// NOTE: can't call functions below.
		//glEnable(GL_ALPHA_TEST);
		//glAlphaFunc(GL_GREATER, 0.0f);
		cocos.director.projection = cocos.director.PROJ_3D
	}
	
	function onExit()
	{
		print(this + ": onExit")
		//NOTE: can't call function below.
		//glDisable(GL_ALPHA_TEST);
		cocos.director.projection = cocos.director.PROJ_2D
	}
	
	function title()
	{
		return "Sprite: openGL Z vertex";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeZVertex : SpriteTestDemo
{
	var _dir = 1
	var _time = 0
	
	constructor()
	{
		base.constructor()
		//
		// This test tests z-order
		// If you are going to use it is better to use a 3D projection
		//
		// WARNING:
		// The developer is resposible for ordering it's sprites according to it's Z if the sprite has
		// transparent parts.
		//
		var s = cocos.director.winSize
		var step = s.width/12
		
		
		var  batch = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"), 1)
		batch.contentSize = cc.Size(s.width, s.height)
		batch.anchorPoint = cc.Point(0.5, 0.5)
		batch.position = cc.Point(s.width/2, s.height/2)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE)
		
		for (var i=0; i<5; i++)
		{
			var sprite = cc.Sprite(pack.locate("grossini_dance_atlas.png"), cc.Rect(0, 121, 85, 121))
			sprite.position = cc.Point((i+1)*step, s.height/2)
			sprite.vertexZ = 10 + i*40
			batch.addChild(sprite, 0)
		}
		for (var i=5; i<11; i++)
		{
			var sprite = cc.Sprite(pack.locate("grossini_dance_atlas.png"), cc.Rect(85, 0, 85, 121))
			sprite.position = cc.Point((i+1)*step, s.height/2)
			sprite.vertexZ = 10 + (10-i)*40
			batch.addChild(sprite, 0)
		}
		
		batch.runAction(cc.action.OrbitCamera(10, 1, 0, 0, 360, 0, 0))
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
		// TIP: don't forget to enable Alpha test
		
		//NOTE: can't call functions below.
		//glEnable(GL_ALPHA_TEST)
		//glAlphaFunc(GL_GREATER, 0.0f)
		cocos.director.projection = cocos.director.PROJ_3D
	}
	
	function onExit()
	{
		print(this + ": onExit")
		//NOTE: can't call function below.
		//glDisable(GL_ALPHA_TEST);
		cocos.director.projection = cocos.director.PROJ_2D
	}
	
	function title()
	{
		return "SpriteBatchNode: openGL Z vertex"
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteAnchorPoint : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var action = []
		var s = cocos.director.winSize
		var rotate = cc.action.RotateBy(10, 360)
		action.push(cc.action.RepeatForever(rotate))
		
		var rotate1 = cc.action.RotateBy(10, 360)
		action.push(cc.action.RepeatForever(rotate1))
		
		var rotate2 = cc.action.RotateBy(10, 360)
		action.push(cc.action.RepeatForever(rotate2))
		
		for (var i=0; i<3; i++)
		{
			var sprite = cc.Sprite(pack.locate("grossini_dance_atlas.png"),
			cc.Rect(85*i, 121*1, 85, 121))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 10)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			sprite.runAction(action[i])
			addChild(sprite, i)
		}
	}
		
	function title()
	{
		return "Sprite: anchor point"
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeAnchorPoint : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var batch = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"), 1)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE)
		
		var action = []
		var s = cocos.director.winSize
		var rotate = cc.action.RotateBy(10, 360)
		action.push(cc.action.RepeatForever(rotate))
		
		var rotate1 = cc.action.RotateBy(10, 360)
		action.push(cc.action.RepeatForever(rotate1))
		
		var rotate2 = cc.action.RotateBy(10, 360)
		action.push(cc.action.RepeatForever(rotate2))
		
		for (var i=0; i<3; i++)
		{
			var sprite = cc.Sprite(batch.texture, cc.Rect(85*i, 121*1, 85, 121))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 10)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			sprite.runAction(action[i])
			addChild(sprite, i)
		}
	}
		
	function title()
	{
		return "SpriteBatchNode: anchor point"
	}
}

////////////////////////////////////////////////////////////////////////////////

class Sprite6 : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var batch = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"), 1)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE)
		batch.relativeAnchorPoint = false
		
		var action = []
		var s = cocos.director.winSize
		
		batch.anchorPoint = cc.Point(0.5, 0.5)
		batch.contentSize = cc.Size(s.width, s.height)
		
		// SpriteBatchNode actions		
		var rotate0 = cc.action.RotateBy(5, 360)
		action.push(cc.action.RepeatForever(rotate0))
		
		var rotate1 = cc.action.RotateBy(5, 360)
		action.push(cc.action.RepeatForever(rotate1))
		
		var rotate2 = cc.action.RotateBy(5, 360)
		action.push(cc.action.RepeatForever(rotate2))
		
		var rotate = cc.action.RotateBy(5, 360)
		var rotate_back = rotate.reverse()
		var rotate_seq = cc.action.Sequence(rotate, rotate_back)
		var rotate_forever = cc.action.RepeatForever(rotate_seq)
		
		var scale = cc.action.ScaleBy(5, 1.5)
		var scale_back = scale.reverse()
		var scale_seq = cc.action.Sequence(scale, scale_back)
		var scale_forever = cc.action.RepeatForever(scale_seq)
		
		var step = s.width/4
		for (var i=0; i<3; i++)
		{
			var sprite = cc.Sprite(batch.texture, cc.Rect(85*i, 121*1, 85, 121))
			sprite.position = cc.Point((i+1)*step, s.height/2)
			sprite.runAction(action[i])
			batch.addChild(sprite, i)
		}
		batch.runAction(scale_forever)
		batch.runAction(rotate_forever)
	}
		
	function title()
	{
		return "SpriteBatchNode transformation";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteFlip : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var sprite1 = cc.Sprite(pack.locate("grossini_dance_atlas.png"), 
			cc.Rect(85, 121, 85, 121))
		sprite1.position = cc.Point(s.width/2 - 100, s.height/2)
		addChild(sprite1, 0, TAG.SPRITE1)	
			
		var sprite2 = cc.Sprite(pack.locate("grossini_dance_atlas.png"), 
			cc.Rect(85, 121, 85, 121))
		sprite2.position = cc.Point(s.width/2 + 100, s.height/2)
		addChild(sprite2, 0, TAG.SPRITE2)
		
		cocos.director.scheduler.repeat(this, flipSprites, 1)
	}
		
	function flipSprites(dt)
	{
		var sprite1 = getChildByTag(TAG.SPRITE1)
		var sprite2 = getChildByTag(TAG.SPRITE2)
		
		var x = sprite1.flipX
		var y = sprite2.flipY
		
		print("Pre: "+ sprite1.contentSize.height)
		sprite1.flipX = !x
		sprite2.flipY = !y
		print("Post: "+ sprite1.contentSize.height)
	}
	function title()
	{
		 return "Sprite Flip X & Y";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeFlip : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var batch = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"),10)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE)
		
		var s = cocos.director.winSize
		
		var sprite1 = cc.Sprite(batch.texture, cc.Rect(85, 121, 85, 121))
		sprite1.position = cc.Point(s.width/2 - 100, s.height/2)
		batch.addChild(sprite1, 0, TAG.SPRITE1)	
			
		var sprite2 = cc.Sprite(batch.texture, cc.Rect(85, 121, 85, 121))
		sprite2.position = cc.Point(s.width/2 + 100, s.height/2)
		batch.addChild(sprite2, 0, TAG.SPRITE2)
		
		cocos.director.scheduler.repeat(this, flipSprites, 1)
	}
		
	function flipSprites()
	{
		var batch = getChildByTag(TAG.SPRITE_BATCH_NODE)
		var sprite1 = batch.getChildByTag(TAG.SPRITE1)
		var sprite2 = batch.getChildByTag(TAG.SPRITE2)
		
		var x = sprite1.flipX
		var y = sprite2.flipY
		
		print("Pre: "+ sprite1.contentSize.height)
		sprite1.flipX = !x
		sprite2.flipY = !y
		print("Post: "+ sprite1.contentSize.height)
	}
	function title()
	{
		 return "SpriteBatchNode Flip X & Y";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteAliased : SpriteTestDemo
{
	constructor()
	{
		base.constructor()

		var s = cocos.director.winSize
		var sprite1 = cc.Sprite(pack.locate("grossini_dance_atlas.png"), 
			cc.Rect(85, 121, 85, 121))
		sprite1.position = cc.Point(s.width/2 - 100, s.height/2)
		addChild(sprite1, 0, TAG.SPRITE1)
		
		var sprite2 = cc.Sprite(pack.locate("grossini_dance_atlas.png"), 
			cc.Rect(85, 121, 85, 121))
		sprite2.position = cc.Point(s.width/2 + 100, s.height/2)
		addChild(sprite2, 0, TAG.SPRITE2)
		
		var scale = cc.action.ScaleBy(2, 5)
		var scale_back = scale.reverse()
		var seq = cc.action.Sequence(scale, scale_back)
		
		var scale1 = cc.action.ScaleBy(2, 5)
		var scale_back1 = scale1.reverse()
		var seq1 = cc.action.Sequence(scale1, scale_back1)
		
		var repeat = cc.action.RepeatForever(seq)
		var repeat2 = cc.action.RepeatForever(seq1)
		
		sprite1.runAction(repeat)
		sprite2.runAction(repeat2)
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
		//
		// IMPORTANT:
		// This change will affect every sprite that uses the same texture
		// So sprite1 and sprite2 will be affected by this change
		//
		var sprite = getChildByTag(TAG.SPRITE1)
		sprite.texture.setAliasTexParameters()
	}
	
	function onExit()
	{
		print(this + ": onExit")
		// restore the tex parameter to AntiAliased.
		var sprite = getChildByTag(TAG.SPRITE1)
		sprite.texture.setAntiAliasTexParameters()
	}
	
	function title()
	{
		return "Sprite Aliased";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeAliased : SpriteTestDemo
{
	constructor()
	{
		base.constructor()

		var s = cocos.director.winSize
		
		var batch = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"),10) 
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE)
		
		var sprite1 = cc.Sprite(batch.texture, cc.Rect(85, 121, 85, 121))
		sprite1.position = cc.Point(s.width/2 - 100, s.height/2)
		batch.addChild(sprite1, 0, TAG.SPRITE1)
		
		var sprite2 = cc.Sprite(batch.texture, cc.Rect(85, 121, 85, 121))
		sprite2.position = cc.Point(s.width/2 + 100, s.height/2)
		batch.addChild(sprite2, 0, TAG.SPRITE2)
		
		var scale = cc.action.ScaleBy(2, 5)
		var scale_back = scale.reverse()
		var seq = cc.action.Sequence(scale, scale_back)
		
		var scale1 = cc.action.ScaleBy(2, 5)
		var scale_back1 = scale1.reverse()
		var seq1 = cc.action.Sequence(scale1, scale_back1)
		
		var repeat = cc.action.RepeatForever(seq)
		var repeat2 = cc.action.RepeatForever(seq1)
		
		sprite1.runAction(repeat)
		sprite2.runAction(repeat2)
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
		//
		// IMPORTANT:
		// This change will affect every sprite that uses the same texture
		// So sprite1 and sprite2 will be affected by this change
		//
		var batch = getChildByTag(TAG.SPRITE_BATCH_NODE)
		batch.texture.setAliasTexParameters()
	}
	
	function onExit()
	{
		print(this + ": onExit")
		// restore the tex parameter to AntiAliased.
		var batch = getChildByTag(TAG.SPRITE_BATCH_NODE)
		batch.texture.setAntiAliasTexParameters()
	}
	
	function title()
	{
		return "SpriteBatchNode Aliased";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteNewTexture : SpriteTestDemo
{
	var _texture1 = null
	var _texture2 = null
	var _usingTexture1 = null
	
	constructor()
	{
		base.constructor()
		touchEnabled = true
		
		var node = cc.Node()
		addChild(node, 0, TAG.SPRITE_BATCH_NODE)
		
		_texture1 = cocos.textureCache.addImage(pack.locate("grossini_dance_atlas.png"))
		_texture2 = cocos.textureCache.addImage(pack.locate("grossini_dance_atlas-mono.png"))
		
		_usingTexture1 = true
		for (var i = 0; i<30; i++)
		{
			addNewSprite()
		}
		
		channel().bind(Events.OnCCTouchEnded, this, ccTouchesEnded)
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	function addNewSprite()
	{
		var s = cocos.director.winSize
		var p = cc.Point(math.random() * s.width, math.random()*s.height)
		var idx = math.random() * 14;
		var x = (idx mod 5) * 85;
		var y = (idx div 5) * 125;
		
		var node = getChildByTag(TAG.SPRITE_BATCH_NODE)
		var sprite = cc.Sprite(_texture1, cc.Rect(x ,y, 85, 121))
		node.addChild(sprite)
		
		sprite.position = cc.Point(p.x, p.y)
		
		var action = null;
		var random = math.random()
		
		if (random < 0.20)
			action = cc.action.ScaleBy(3, 2);
		else if (random < 0.40)
			action = cc.action.RotateBy(3, 360);
		else if (random < 0.60)
			action = cc.action.Blink(1, 3);
		else if (random < 0.8)
			action = cc.action.TintBy(2, 0, -255, -255);
		else 
			action = cc.action.FadeOut(2);
			
		var action_back = action.reverse();
		var seq = cc.action.Sequence(action, action_back);
		
		sprite.runAction(cc.action.RepeatForever(seq))
	}
	
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		var node = getChildByTag(TAG.SPRITE_BATCH_NODE)
		var children = node.children
		
		if (_usingTexture1)
		{
			foreach (sprite in children)
			{
				try sprite.texture = _texture2
			}
			_usingTexture1 = false
		}
		else
		{
			foreach (sprite in children)
			{
				try sprite.texture = _texture1
			}
			_usingTexture1 = true
		}
	}

	function title()
	{
		return "Sprite new texture (tap)";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeNewTexture : SpriteTestDemo
{
	var _texture1 = null
	var _texture2 = null
	
	constructor()
	{
		base.constructor()
		
		touchEnabled = true
		
		var batch = cc.SpriteBatchNode(pack.locate("grossini_dance_atlas.png"), 50)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE)
		
		_texture1 = batch.texture
		_texture2 = cocos.textureCache.addImage(pack.locate("grossini_dance_atlas-mono.png"))
		
		for (var i = 0; i<30; i++)
		{
			addNewSprite()
		}
		
		channel().bind(Events.OnCCTouchEnded, this, ccTouchesEnded)
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	function addNewSprite()
	{
		var s = cocos.director.winSize
		var p = cc.Point(math.random() * s.width, math.random()*s.height)
		var idx = math.random() * 14;
		var x = (idx mod 5) * 85;
		var y = (idx div 5) * 125;
		
		var batch = getChildByTag(TAG.SPRITE_BATCH_NODE)
		
		var sprite = cc.Sprite(batch.texture, cc.Rect(x ,y, 85, 121))
		batch.addChild(sprite)
		
		sprite.position = cc.Point(p.x, p.y)
		
		var action = null;
		var random = math.random()
		
		if (random < 0.20)
			action = cc.action.ScaleBy(3, 2);
		else if (random < 0.40)
			action = cc.action.RotateBy(3, 360);
		else if (random < 0.60)
			action = cc.action.Blink(1, 3);
		else if (random < 0.8)
			action = cc.action.TintBy(2, 0, -255, -255);
		else 
			action = cc.action.FadeOut(2);
			
		var action_back = action.reverse();
		var seq = cc.action.Sequence(action, action_back);
		
		sprite.runAction(cc.action.RepeatForever(seq))
	}
	
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		var batch = getChildByTag(TAG.SPRITE_BATCH_NODE)
		
		if (batch.texture == _texture1)
			batch.texture = _texture2
		else
			batch.texture = _texture1
	}

	function title()
	{
		return "SpriteBatchNode new texture (tap)";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteFrameTest : SpriteTestDemo
{
	var _pSprite1 = null
	var _pSprite2 = null
	var _nCounter = 0
	
	constructor()
	{
		base.constructor()
		
		// IMPORTANT:
		// The sprite frames will be cached AND RETAINED, and they won't be released unless you call
		//     cocos.spriteFrameCache.removeUnusedSpriteFrames()
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png") 
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_blue.plist"), "grossini_blue.png") 
		//
		// Animation using Sprite BatchNode
		//
		var s = cocos.director.winSize
		_pSprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		_pSprite1.position = cc.Point(s.width/2-80, s.height/2)
		
		var spritebatch  = cc.SpriteBatchNode(pack.locate("grossini.png"))
		spritebatch.addChild(_pSprite1)
		addChild(spritebatch)
		
		var animation = cc.Animation()
		for (var i = 1; i<15; i++)
		{
			var str = format("grossini_dance_%02d.png", i)
			var frame = cocos.spriteFrameCache.spriteFrameByName(str)
			animation.addFrame(frame)
		}
		
		_pSprite1.runAction(cc.action.RepeatForever(cc.action.Animate(animation, false)))
		 // to test issue #732, uncomment the following line
		_pSprite1.flipX=false
		_pSprite1.flipY=false
		//
		// Animation using standard Sprite
		//
		_pSprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		_pSprite2.position = cc.Point(s.width/2 + 80, s.height/2)
		addChild(_pSprite2)

		
		var animMixed = cc.Animation()
		
		for (var i = 1; i<15; i++)
		{
			var str = format("grossini_dance_gray_%02d.png", i)
			var frame = cocos.spriteFrameCache.spriteFrameByName(str)
			animMixed.addFrame(frame)
		}
		for (var i = 1; i<5; i++)
		{
			var str = format("grossini_blue_%02d.png", i)
			var frame = cocos.spriteFrameCache.spriteFrameByName(str)
			animMixed.addFrame(frame)
		}
			
		_pSprite2.runAction(cc.action.RepeatForever(cc.action.Animate(animMixed, false)))
		
		_pSprite2.flipX = false
		_pSprite2.flipY = false
		
		cocos.director.scheduler.once(this, StartIn05Secs, 0.5)
	}
	
	function onEnter()
	{
		print(this + ": onEnter")
	}
	
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_blue.plist"))
	}

	function StartIn05Secs()
	{
		cocos.director.scheduler.repeat(this, flipSprites, 1.0)
	}
	
	function flipSprites()
	{
		_nCounter++;

		var fx = false;
		var fy = false;
		var  i  = _nCounter % 4;

		switch (i) {
			case 0:
				fx = false;
				fy = false;
				break;
			case 1:
				fx = true;
				fy = false;
				break;
			case 2:
				fx = false;
				fy = true;
				break;
			case 3:
				fx = true;
				fy = true;
				break;
		}

		_pSprite1.flipX=fx;
		_pSprite1.flipY=fy;
		_pSprite2.flipX=fx;
		_pSprite2.flipY=fy;
	}
	
	function title()
	{
		return "Sprite vs. SpriteBatchNode animation";
	}
	
	function subtitle()
	{
		return "Testing issue #792";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteFrameAliasNameTest : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		// IMPORTANT:
		// The sprite frames will be cached AND RETAINED, and they won't be released unless you call
		//     [[CCspriteFrameCache sharedspriteFrameCache] removeUnusedSpriteFrames];
		//
		// CCspriteFrameCache is a cache of CCSpriteFrames
		// CCSpriteFrames each contain a texture id and a rect (frame).
		
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini-aliases.plist"), "grossini-aliases.png") 
	
		//
		// Animation using Sprite batch
		//
		// A CCSpriteBatchNode can reference one and only one texture (one .png file)
		// Sprites that are contained in that texture can be instantiatied as CCSprites and then added to the CCSpriteBatchNode
		// All CCSprites added to a CCSpriteBatchNode are drawn in one OpenGL ES draw call
		// If the CCSprites are not added to a CCSpriteBatchNode then an OpenGL ES draw call will be needed for each one, which is less efficient
		//
		// When you animate a sprite, CCAnimation changes the frame of the sprite using setDisplayFrame: (this is why the animation must be in the same texture)
		// When setDisplayFrame: is used in the CCAnimation it changes the frame to one specified by the CCSpriteFrames that were added to the animation,
		// but texture id is still the same and so the sprite is still a child of the CCSpriteBatchNode, 
		// 
		var sprite  = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite.position = cc.Point(s.width * 0.5, s.height * 0.5)
		
		var spriteBatch = cc.SpriteBatchNode(pack.locate("grossini-aliases.png"))
		spriteBatch.addChild(sprite)
		addChild(spriteBatch)
		
		var anim = cc.Animation()
		for (var i = 1; i<15; i++)
		{
			var str = format("dance_%02d", i)
			var frame = cocos.spriteFrameCache.spriteFrameByName(str)
			anim.addFrame(frame)
		}
		sprite.runAction(cc.action.Animate(14.0, anim, false))
	
	}
	
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini-aliases.plist"))
	}
	
	function title()
	{
		return "SpriteFrame Alias Name";
	}
	
	function subtitle()
	{
		return "SpriteFrames are obtained using the alias name";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteOffsetAnchorRotation : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist")) 
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png")
		
		for (var i=0; i<3; i++)
		{
			// Animation using Sprite BatchNode
			var sprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 1)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			
			var anim = cc.Animation()
			for (var i = 1; i<14; i++)
			{
				var str = format("grossini_dance_%02d.png", i)
				var frame = cocos.spriteFrameCache.spriteFrameByName(str)
				anim.addFrame(frame)
			}
			
			sprite.runAction(cc.action.RepeatForever(cc.action.Animate(anim, false)))
			sprite.runAction(cc.action.RepeatForever(cc.action.RotateBy(10, 360)))
			addChild(sprite, 0)
			
		}
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
	}
	
	function title()
	{
		return  "Sprite offset + anchor + rot";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeOffsetAnchorRotation : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		for (var i=0; i<3; i++)
		{
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist")) 
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png")
		
			// Animation using Sprite BatchNode
			var sprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 200)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			
			var spritebatch = cc.SpriteBatchNode(pack.locate("grossini.png"))
			addChild(spritebatch)
			
			var anim = cc.Animation()
			for (var i = 1; i<14; i++)
			{
				var str = format("grossini_dance_%02d.png", i)
				var frame = cocos.spriteFrameCache.spriteFrameByName(str)
				anim.addFrame(frame)
			}
			
			sprite.runAction(cc.action.RepeatForever(cc.action.Animate(anim, false)))
			sprite.runAction(cc.action.RepeatForever(cc.action.RotateBy(10, 360)))
			addChild(sprite, i)
			
		}
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
	}
	
	function title()
	{
		return  "SpriteBatchNode offset + anchor + rot";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteOffsetAnchorScale : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist")) 
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png")
		
		for (var i=0; i<3; i++)
		{
			// Animation using Sprite BatchNode
			var sprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 1)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			
			var anim = cc.Animation()
			for (var i = 1; i<14; i++)
			{
				var str = format("grossini_dance_%02d.png", i)
				var frame = cocos.spriteFrameCache.spriteFrameByName(str)
				anim.addFrame(frame)
			}
			
			var scale = cc.action.ScaleBy(2, 2)
			var scale_back = scale.reverse()
			var seq_scale = cc.action.Sequence(scale, scale_back)
			sprite.runAction(cc.action.RepeatForever(seq_scale))
			addChild(sprite, 0)
			
		}
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
	}
	
	function title()
	{
		return  "Sprite offset + anchor + scale";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeOffsetAnchorScale : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		for (var i=0; i<3; i++)
		{
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist")) 
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png")
		
			// Animation using Sprite BatchNode
			var sprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 200)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			
			var spritesheet = cc.SpriteBatchNode(pack.locate("grossini.png"))
			addChild(spritesheet)
			
			var anim = cc.Animation()
			for (var i = 1; i<14; i++)
			{
				var str = format("grossini_dance_%02d.png", i)
				var frame = cocos.spriteFrameCache.spriteFrameByName(str)
				anim.addFrame(frame)
			}
			
			var scale = cc.action.ScaleBy(2, 2)
			var scale_back = scale.reverse()
			var seq_scale = cc.action.Sequence(scale, scale_back)
			sprite.runAction(cc.action.RepeatForever(seq_scale))
			
			spritesheet.addChild(sprite, i)
			
		}
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
	}
	
	function title()
	{
		return  "SpriteBatchNode offset + anchor + scale";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteAnimationSplit : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
	
		var texture = cocos.textureCache.addImage(pack.locate("dragon_animation.png"))
		
		// manually add frames to the frame cache
		var frame0 = cc.SpriteFrame(texture, cc.Rect(132*0, 132*0, 132, 132));
		var frame1 = cc.SpriteFrame(texture, cc.Rect(132*1, 132*0, 132, 132));
		var frame2 = cc.SpriteFrame(texture, cc.Rect(132*2, 132*0, 132, 132));
		var frame3 = cc.SpriteFrame(texture, cc.Rect(132*3, 132*0, 132, 132));
		var frame4 = cc.SpriteFrame(texture, cc.Rect(132*0, 132*1, 132, 132));
		var frame5 = cc.SpriteFrame(texture, cc.Rect(132*1, 132*1, 132, 132));		
		//
		// Animation using Sprite BatchNode
		//	
		var sprite = cc.Sprite(frame0)
		sprite.position = cc.Point(s.width/2-80, s.height/2)
		addChild(sprite)
		
		var animation = cc.Animation()
		animation.delay = 0.2
		animation.addFrame(frame0)
		animation.addFrame(frame1)
		animation.addFrame(frame2)
		animation.addFrame(frame3)
		animation.addFrame(frame4)
		animation.addFrame(frame5)
		
		var animate = cc.action.Animate(animation, false)
		var animatecopy = cc.action.Animate(animation, false)
		var seq = cc.action.Sequence(animate, cc.action.FlipX(true),
									animatecopy, cc.action.FlipX(false))
		
		sprite.runAction(cc.action.RepeatForever(seq))
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeUnusedSpriteFrames()
	}
	
	function title()
	{
		return  "Sprite: Animation + flip";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteHybrid : SpriteTestDemo
{
	_usingSpriteBatchNode = null
	
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var parent1 = cc.Node()
		var parent2 = cc.SpriteBatchNode(pack.locate("grossini.png"), 50)
		addChild(parent1, 0, TAG.NODE)
		addChild(parent2, 0, TAG.SPRITE_BATCH_NODE) 
	
		// IMPORTANT:
		// The sprite frames will be cached AND RETAINED, and they won't be released unless you call
		//     CCspriteFrameCache::sharedspriteFrameCache()->removeUnusedSpriteFrames);
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist"))

		// create 250 sprites
		// only show 80% of them
		for (var i = 1; i<250; i++)
		{
			var spriteIdx = math.floor(math.random() * 14) + 1

			var str = format("grossini_dance_%02d.png", spriteIdx)
			var frame = cocos.spriteFrameCache.spriteFrameByName(str)
			var sprite = cc.Sprite(frame)
			parent1.addChild(sprite, i, i)
			
			var x = -1000
			var y = -1000
			if (math.random() < 0.2)
			{
				x = math.random() * s.width
				y = math.random() * s.height
			}
			sprite.position = cc.Point(x, y)
			
			var action = cc.action.RotateBy(4, 360)
			sprite.runAction(cc.action.RepeatForever(action))
		}
		_usingSpriteBatchNode = false
		
		cocos.director.scheduler.repeat(this, reparentSprite, 2)
	}
	
	function reparentSprite(dt)
	{	
		var p1 = getChildByTag(TAG.NODE)
		var p2 = getChildByTag(TAG.SPRITE_BATCH_NODE)
		
		var retArray = []
		if (_usingSpriteBatchNode)
		{
			var temp = cc.Node()
			temp = p1
			p1 = p2
			p2 = temp
		}
		
		var node = null
		var pObject = null
		var children = p1.children
		foreach (k, v in children)
		{	
			node = v
			if (!node)
				break
			retArray.push(node)
		}
		
		var i=0;
		p1.removeAllChildren(false)
	
		foreach (k, v in retArray)
		{	
			node = v
			if (!node)
				break
			p2.addChild(node, i, i)
			i++
		}
		_usingSpriteBatchNode = !_usingSpriteBatchNode
			
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
	}
	
	function title()
	{
		return "HybrCCSprite* sprite Test";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeChildren : SpriteTestDemo
{
	
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var batch = cc.SpriteBatchNode(pack.locate("grossini.png"), 50)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE) 
		
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist"))
	
		var sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite1.position = cc.Point(s.width/3, s.height/2)
		
		var sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(50, 50)
		
		var sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-50, -50)
		
		batch.addChild(sprite1)
		sprite1.addChild(sprite2)
		sprite1.addChild(sprite3)
		
		//BEGIN NEW CODE
		var anim = cc.Animation()
		anim.delay = 0.2
		for (var i = 1; i<15; i++)
		{
			var str = format("grossini_dance_%02d.png", i)
			var frame = cocos.spriteFrameCache.spriteFrameByName(str)
			anim.addFrame(frame)
		}
		sprite1.runAction(cc.action.RepeatForever(cc.action.Animate(anim, false)))
		//END NEW CODE
		
		var action = cc.action.MoveBy(2, cc.Point(200,0))
		var action_back = action.reverse()
		var action_rot = cc.action.RotateBy(2, 360)
		var action_s = cc.action.ScaleBy(2, 2)
		var action_s_back = action_s.reverse()
		
		var seq2 = action_rot.reverse()
		sprite2.runAction(cc.action.RepeatForever(seq2))
		
		sprite1.runAction(cc.action.RepeatForever(action_rot))
		sprite1.runAction(cc.action.RepeatForever(cc.action.Sequence(action, action_back)))
		sprite1.runAction(cc.action.RepeatForever(cc.action.Sequence(action_s, action_s_back)))
		
	}
	
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeUnusedSpriteFrames()
	}
	
	function title()
	{
		return "SpriteBatchNode Grand Children";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeChildren2 : SpriteTestDemo
{
	
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		var batch = cc.SpriteBatchNode(pack.locate("grossini.png"), 50)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE) 
		
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist"))
	
		var sprite11 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite11.position = cc.Point(s.width/3, s.height/2)
		
		var sprite12 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite12.position = cc.Point(20, 30)
		sprite12.scale(0.2)
		
		var sprite13 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite13.position = cc.Point(-20, 30)
		sprite13.scale(0.2)
		
		batch.addChild(sprite11)
		sprite11.addChild(sprite12, -2)
		sprite11.addChild(sprite13, 2)
		
		// don't rotate with it's parent
		sprite12.honorParentTransform = (sprite12.honorParentTransform & ~2)
		// don't scale and rotate with it's parent
		sprite13.honorParentTransform = (sprite13.honorParentTransform & (~2 | 2))
		
		var action = cc.action.MoveBy(2, cc.Point(200,0))
		var action_back = action.reverse()
		var action_rot = cc.action.RotateBy(2, 360)
		var action_s = cc.action.ScaleBy(2, 2)
		var action_s_back = action_s.reverse()
		
		sprite11.runAction(cc.action.RepeatForever(action_rot))
		sprite11.runAction(cc.action.RepeatForever(cc.action.Sequence(action, action_back)))
		sprite11.runAction(cc.action.RepeatForever(cc.action.Sequence(action_s, action_s_back)))
		
		//
		// another set of parent / children
		//
		
		var sprite21 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite21.position = cc.Point(2*s.width/3, s.height/2-50)
		
		var sprite22 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite22.position = cc.Point(20, 30)
		sprite22.scale(0.8)
		
		var sprite23 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite23.position = cc.Point(-20, 30)
		sprite23.scale(0.8)
		
		batch.addChild(sprite21)
		sprite21.addChild(sprite22, -2)
		sprite21.addChild(sprite23, 2)
		
		// don't rotate with it's parent
		sprite22.honorParentTransform = (sprite22.honorParentTransform & ~(cc.Sprite :> HONOR_ROTATE))
		// don't scale and rotate with it's parent
		sprite23.honorParentTransform = (sprite23.honorParentTransform & ~(cc.Sprite :> HONOR_ROTATE | HONOR_SCALE))
		
		sprite21.runAction(cc.action.RepeatForever(cc.action.RotateBy(1, 360)))
		sprite21.runAction(cc.action.RepeatForever(cc.action.Sequence(
			cc.action.ScaleTo(0.5, 5.0), cc.action.ScaleTo(0.5, 1))))
	}
	
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeUnusedSpriteFrames()
	}
	
	function title()
	{
		return "SpriteBatchNode HonorTransform";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeChildrenZ : SpriteTestDemo
{
	
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist"))
		
		// test 1
		var batch = cc.SpriteBatchNode(pack.locate("grossini.png"), 50)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE) 
	
		var sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite1.position = cc.Point(s.width/3, s.height/2)
		
		var sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		var sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		batch.addChild(sprite1)
		sprite1.addChild(sprite2, 2)
		sprite1.addChild(sprite3, -2)
		
		// test 2
		batch = cc.SpriteBatchNode(pack.locate("grossini.png"), 50)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE) 
	
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite1.position = cc.Point(2 * s.width/3, s.height/2)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		batch.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, 2)
		
		// test 3
		batch = cc.SpriteBatchNode(pack.locate("grossini.png"), 50)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE) 
	
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite1.position = cc.Point(s.width/2 - 90, s.height/4)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(s.width/2 - 60, s.height/4)
		
		sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(s.width/2 - 30, s.height/4)
		
		batch.addChild(sprite1, 10)
		batch.addChild(sprite2, -10)
		batch.addChild(sprite3, -5)
		
		// test 4
		batch = cc.SpriteBatchNode(pack.locate("grossini.png"), 50)
		addChild(batch, 0, TAG.SPRITE_BATCH_NODE) 
	
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite1.position = cc.Point(s.width/2 + 30, s.height/4)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(s.width/2 + 60, s.height/4)
		
		sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(s.width/2 + 90, s.height/4)
		
		batch.addChild(sprite1, -10)
		batch.addChild(sprite2, -5)
		batch.addChild(sprite3, -2)
	}
	
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeUnusedSpriteFrames()
	}
	
	function title()
	{
		return "SpriteBatchNode Children Z";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteChildrenVisibility : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist"))
		//
		// SpriteBatchNode
		//
		// parents
		var aParent = cc.SpriteBatchNode(pack.locate("grossini.png"), 50)
		aParent.position = cc.Point(s.width/3, s.height/2)
		addChild(aParent, 0) 
	
	
		var sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite1.position = cc.Point(0, 0)
		
		var sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		var sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, 2)
		
		sprite1.runAction(cc.action.Blink(5, 10))
		//
		// Sprite
		//
		aParent = cc.Node()
		aParent.position = cc.Point(2*s.width/3, s.height/2)
		addChild(aParent, 0)
		
		var sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite1.position = cc.Point(0, 0)
		
		var sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		var sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, 2)
		
		sprite1.runAction(cc.action.Blink(5, 10))
	}
	
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeUnusedSpriteFrames()
	}
	
	function title()
	{
		return "Sprite & SpriteBatchNode Visibility";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteChildrenVisibilityIssue665 : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist"))
		//
		// SpriteBatchNode
		//
		// parents
		var aParent = cc.SpriteBatchNode(pack.locate("grossini.png"), 50)
		aParent.position = cc.Point(s.width/3, s.height/2)
		addChild(aParent, 0) 
	
	
		var sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite1.position = cc.Point(0, 0)
		
		var sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		var sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		//test issue #665
		sprite1.visible = false
		
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, 2)

		//
		// Sprite
		//
		aParent = cc.Node()
		aParent.position = cc.Point(2*s.width/3, s.height/2)
		addChild(aParent, 0)
		
		var sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
		sprite1.position = cc.Point(0, 0)
		
		var sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		var sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		//test issue #665
		sprite1.visible = false
		
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, 2)
	}
	
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeUnusedSpriteFrames()
	}
	
	function title()
	{
		return "Sprite & SpriteBatchNode Visibility";
	}
	
	function subtitle()
	{
		return "No sprites should be visible";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteChildrenAnchorPoint : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist"))
		//
		// SpriteBatchNode
		//
		// parents
		var aParent = cc.Node()
		addChild(aParent, 0)
		
		//anchor(0,0)
		var sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_08.png"))
		sprite1.position = cc.Point(s.width/4, s.height/2)
		sprite1.anchorPoint = cc.Point(0, 0)
		
		var sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		var sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		var sprite4 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_04.png"))
		sprite4.position = cc.Point(0,0)
		sprite4.scale(0.5)
	
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, -2)
		sprite1.addChild(sprite4, 3)

		var point = cc.Sprite(pack.locate("r1.png"))
		point.scale(0.25)
		point.position = sprite1.position
		addChild(point, 10)
		
		//anchor (0.5, 0.5)
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_08.png"))
		sprite1.position = cc.Point(s.width/2, s.height/2)
		sprite1.anchorPoint = cc.Point(0.5, 0.5)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		sprite4 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_04.png"))
		sprite4.position = cc.Point(0,0)
		sprite4.scale(0.5)
	
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, -2)
		sprite1.addChild(sprite4, 3)

		point = cc.Sprite(pack.locate("r1.png"))
		point.scale(0.25)
		point.position = sprite1.position
		addChild(point, 10)
		
		//anchor (1, 1)
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_08.png"))
		sprite1.position = cc.Point(s.width/2 + s.width/4, s.height/2)
		sprite1.anchorPoint = cc.Point(1, 1)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		sprite4 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_04.png"))
		sprite4.position = cc.Point(0,0)
		sprite4.scale(0.5)
	
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, -2)
		sprite1.addChild(sprite4, 3)

		point = cc.Sprite(pack.locate("r1.png"))
		point.scale(0.25)
		point.position = sprite1.position
		addChild(point, 10)
	}
	
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeUnusedSpriteFrames()
	}
	
	function title()
	{
		return "Sprite: children + anchor";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeChildrenAnchorPoint : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist"))
		//
		// SpriteBatchNode
		//
		// parents
		var aParent = cc.SpriteBatchNode(pack.locate("grossini.png"), 50)
		addChild(aParent, 0)
		
		//anchor(0,0)
		var sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_08.png"))
		sprite1.position = cc.Point(s.width/4, s.height/2)
		sprite1.anchorPoint = cc.Point(0, 0)
		
		var sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		var sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		var sprite4 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_04.png"))
		sprite4.position = cc.Point(0,0)
		sprite4.scale(0.5)
	
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, -2)
		sprite1.addChild(sprite4, 3)

		var point = cc.Sprite(pack.locate("r1.png"))
		point.scale(0.25)
		point.position = sprite1.position
		addChild(point, 10)
		
		//anchor (0.5, 0.5)
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_08.png"))
		sprite1.position = cc.Point(s.width/2, s.height/2)
		sprite1.anchorPoint = cc.Point(0.5, 0.5)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		sprite4 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_04.png"))
		sprite4.position = cc.Point(0,0)
		sprite4.scale(0.5)
	
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, -2)
		sprite1.addChild(sprite4, 3)

		point = cc.Sprite(pack.locate("r1.png"))
		point.scale(0.25)
		point.position = sprite1.position
		addChild(point, 10)
		
		//anchor (1, 1)
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_08.png"))
		sprite1.position = cc.Point(s.width/2 + s.width/4, s.height/2)
		sprite1.anchorPoint = cc.Point(1, 1)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_02.png"))
		sprite2.position = cc.Point(20, 30)
		
		sprite3 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_03.png"))
		sprite3.position = cc.Point(-20, 30)
		
		sprite4 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_04.png"))
		sprite4.position = cc.Point(0,0)
		sprite4.scale(0.5)
	
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2, -2)
		sprite1.addChild(sprite3, -2)
		sprite1.addChild(sprite4, 3)

		point = cc.Sprite(pack.locate("r1.png"))
		point.scale(0.25)
		point.position = sprite1.position
		addChild(point, 10)
	}
	
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeUnusedSpriteFrames()
	}
	
	function title()
	{
		return "SpriteBatchNode: children + anchor";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeChildrenScale : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_family.plist"))
		
		var aParent = null
		var rot = cc.action.RotateBy(10, 360)
		var seq = cc.action.RepeatForever(rot)

		//children + scale using sprite
		//Test 1
		
		aParent = cc.Node()
		var sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossinis_sister1.png"))
		sprite1.position = cc.Point(s.width/4, s.height/4)
		sprite1.scaleX = -0.5
		sprite1.scaleY = 2.0
		sprite1.runAction(seq)
		
		var sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossinis_sister2.png"))
		sprite2.position = cc.Point(50, 0)
		
		addChild(aParent)
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2)
		
		// Children + Scale using SpriteBatchNode
		// Test 2

		var rot1 = cc.action.RotateBy(10, 360)
		var seq1 = cc.action.RepeatForever(rot1)
		
		aParent = cc.SpriteBatchNode(pack.locate("grossini_family.png"))
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossinis_sister1.png"))
		sprite1.position = cc.Point(3*s.width/4, s.height/4)
		sprite1.scaleX = -0.5
		sprite1.scaleY = 2.0
		sprite1.runAction(seq1)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossinis_sister2.png"))
		sprite2.position = cc.Point(50, 0)
		
		addChild(aParent)
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2)
		
		// Children + Scale using Sprite
		// Test 3

		var rot2 = cc.action.RotateBy(10, 360)
		var seq2 = cc.action.RepeatForever(rot2)
		
		aParent = cc.Node()
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossinis_sister1.png"))
		sprite1.position = cc.Point(s.width/4, 2*s.height/3)
		sprite1.scaleX = 1.5
		sprite1.scaleY = -0.5
		sprite1.runAction(seq2)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossinis_sister2.png"))
		sprite2.position = cc.Point(50, 0)
		
		addChild(aParent)
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2)
		
		// Children + Scale using SpriteBatchNode
		// Test 4

		var rot3 = cc.action.RotateBy(10, 360)
		var seq3 = cc.action.RepeatForever(rot3)
		
		aParent = cc.SpriteBatchNode(pack.locate("grossini_family.png"))
		sprite1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossinis_sister1.png"))
		sprite1.position = cc.Point(3*s.width/4, 2*s.height/3)
		sprite1.scaleX = 1.5
		sprite1.scaleY = -0.5
		sprite1.runAction(seq3)
		
		sprite2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossinis_sister2.png"))
		sprite2.position = cc.Point(50, 0)
		
		addChild(aParent)
		aParent.addChild(sprite1)
		sprite1.addChild(sprite2)
	}
	
	function title()
	{
		return "Sprite/BatchNode + child + scale + rot";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteChildrenChildren : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
	
		var s = cocos.director.winSize;
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("ghosts.plist"),"ghosts.png")
		
		var aParent = null
		var l1, l2a, l2b, l3a1, l3a2, l3b1, l3b2 = null
		//
		// SpriteBatchNode: 3 levels of children
		//
		
		aParent = cc.Node()
		addChild(aParent, 0, TAG.SPRITE1)
		
		//parent
		var rot1 = cc.action.RotateBy(10, 360)
		var seq1 = cc.action.RepeatForever(rot1)
		l1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("father.gif"))
		l1.position = cc.Point(s.width/2, s.height/2)
		l1.runAction(seq1)
		aParent.addChild(l1, 0, TAG.SPRITE2)
		var l1Size = l1.contentSize;
		
		//child left
		var rot2 = cc.action.RotateBy(10, 360)
		var seq2 = cc.action.RepeatForever(rot2)
		l2a = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("sister1.gif"))
		l2a.position = cc.Point(-50 + l1Size.width/2, 0 + l1Size.height/2)
		l2a.runAction(seq2.reverse())
		l1.addChild(l2a)
		var l2aSize = l2a.contentSize
		
		//child right
		var rot3 = cc.action.RotateBy(10, 360)
		var seq3 = cc.action.RepeatForever(rot3)
		l2b = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("sister2.gif"))
		l2b.position = cc.Point(50 + l1Size.width/2, 0 + l1Size.height/2)
		l2b.runAction(seq3.reverse())
		l1.addChild(l2b)
		var l2bSize = l2b.contentSize
		
		//child left bottom
		l3a1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3a1.scale(0.45)
		l3a1.position = cc.Point(0 + l2aSize.width/2 - 50, -100 + l2aSize.height/2)
		l2a.addChild(l3a1)
		
		//child left top
		l3a2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3a2.scale(0.45)
		l3a2.position = cc.Point(l2aSize.width/2, 100 + l2aSize.height/2)
		l2a.addChild(l3a2)
		
		//child right bottom
		l3a1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3a1.position = cc.Point(l2bSize.width/2 - 50, -100 + l2bSize.height/2)
		l3a1.scale(0.45)
		l3a1.flipY = true
		l2b.addChild(l3a1)
		
		//child right top
		l3b2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3b2.scale(0.45)
		l3b2.flipY = true
		l3b2.position = cc.Point(l2bSize.width/2, 100 + l2bSize.height/2)
		l2b.addChild(l3b2)
	}

	function title()
	{
		return "Sprite multiple levels of children";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeChildrenChildren : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
	
		var s = cocos.director.winSize;
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("ghosts.plist"),"ghosts.png")
		
		var aParent = null
		var l1, l2a, l2b, l3a1, l3a2, l3b1, l3b2 = null
		//
		// SpriteBatchNode: 3 levels of children
		//
		
		aParent = cc.SpriteBatchNode(pack.locate("ghosts.png"))
		aParent.texture.generateMipmap()
		addChild(aParent)
		
		//parent
		var rot1 = cc.action.RotateBy(10, 360)
		var seq1 = cc.action.RepeatForever(rot1)
		l1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("father.gif"))
		l1.position = cc.Point(s.width/2, s.height/2)
		l1.runAction(seq1)
		aParent.addChild(l1, 0, TAG.SPRITE2)
		var l1Size = l1.contentSize;
		
		//child left
		var rot2 = cc.action.RotateBy(10, 360)
		var seq2 = cc.action.RepeatForever(rot2)
		l2a = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("sister1.gif"))
		l2a.position = cc.Point(-50 + l1Size.width/2, 0 + l1Size.height/2)
		l2a.runAction(seq2.reverse())
		l1.addChild(l2a)
		var l2aSize = l2a.contentSize
		
		//child right
		var rot3 = cc.action.RotateBy(10, 360)
		var seq3 = cc.action.RepeatForever(rot3)
		l2b = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("sister2.gif"))
		l2b.position = cc.Point(50 + l1Size.width/2, 0 + l1Size.height/2)
		l2b.runAction(seq3.reverse())
		l1.addChild(l2b)
		var l2bSize = l2b.contentSize
		
		//child left bottom
		l3a1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3a1.scale(0.45)
		l3a1.position = cc.Point(0 + l2aSize.width/2 - 50, -100 + l2aSize.height/2)
		l2a.addChild(l3a1)
		
		//child left top
		l3a2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3a2.scale(0.45)
		l3a2.position = cc.Point(l2aSize.width/2, 100 + l2aSize.height/2)
		l2a.addChild(l3a2)
		
		//child right bottom
		l3a1 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3a1.position = cc.Point(l2bSize.width/2 - 50, -100 + l2bSize.height/2)
		l3a1.scale(0.45)
		l3a1.flipY = true
		l2b.addChild(l3a1)
		
		//child right top
		l3b2 = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("child1.gif"))
		l3b2.scale(0.45)
		l3b2.flipY = true
		l3b2.position = cc.Point(l2bSize.width/2, 100 + l2bSize.height/2)
		l2b.addChild(l3b2)
	}

	function title()
	{
		return "SpriteBatchNode multiple levels of children";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteNilTexture : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		var sprite = null
		
		//TEst: If no texture is given, then opacity + color shoud work.
		
		sprite = cc.Sprite()
		sprite.textureRect = cc.Rect(0, 0, 300, 300)
		sprite.color = nit.Color.RED
		sprite.opacity = 128
		sprite.position = cc.Point(3*s.width/4, s.height/2)
		addChild(sprite, 100)
		
		sprite = cc.Sprite()
		sprite.textureRect = cc.Rect(0, 0, 300, 300)
		sprite.color = nit.Color.BLUE
		sprite.opacity = 128
		sprite.position = cc.Point(1*s.width/4, s.height/2)
		addChild(sprite, 100)
	}
	
	function title()
	{
		return "Sprite without texture";
	}
	
	function subtitle()
	{
		return "opacity and color should work";
	}
}

////////////////////////////////////////////////////////////////////////////////

class MySprite1
{
	constructor()
	{
	}
	
	static function spriteWithSpriteFrameName(pszSpriteFrameName)
	{
		var pobSprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName(pszSpriteFrameName))
		return pobSprite
	}
}

class MySprite2
{
	constructor()
	{
	}
	
	static function spriteWithFile(pszName)
	{
		var pobSprite = cc.Sprite(pszName)
		return pobSprite
	}
}

class SpriteSubclass : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("ghosts.plist"))
		var aParent = cc.SpriteBatchNode(pack.locate("ghosts.png"))
		
		// MySprite1
		var sprite = MySprite1.spriteWithSpriteFrameName("father.gif")
		sprite.position = cc.Point(s.width/4, s.height/2)
		aParent.addChild(sprite)
		addChild(aParent)
		
		// MySprite2
		var sprite2 = MySprite2.spriteWithFile(pack.locate("grossini.png", "*Images*"))
		sprite2.position = cc.Point(s.width/4*3, s.height/2)
		addChild(sprite2)
	}
	
	function title()
	{
		return  "Sprite subclass";
	}
	
	function subtitle()
	{
		return "Testing initWithTexture:rect method";
	}
}

////////////////////////////////////////////////////////////////////////////////

class AnimationCache : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"))
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_blue.plist"))
		
		//
		// create animation "dance"
		//
		var animation = cc.Animation()
		animation.delay = 0.2
		
		for (var i = 1; i<15; i++)
		{
			var str = format("grossini_dance_%02d.png", i)
			var frame = cocos.spriteFrameCache.spriteFrameByName(str)
			animation.addFrame(frame)
		}
		cocos.animationCache.add(animation, "dance")
		
		//
		// create animation "dance gray"
		//
	
		var animationgray = cc.Animation()
		animationgray.delay = 0.2
		
		for (var i = 1; i<15; i++)
		{
			var str = format("grossini_dance_gray_%02d.png", i)
			var frame = cocos.spriteFrameCache.spriteFrameByName(str)
			animationgray.addFrame(frame)
		}
		cocos.animationCache.add(animationgray, "dance_gray")
		
		//
		// create animation "dance blue"
		//
	
		var animationblue = cc.Animation()
		animationblue.delay = 0.2
		
		for (var i = 1; i<4; i++)
		{
			var str = format("grossini_blue_%02d.png", i)
			var frame = cocos.spriteFrameCache.spriteFrameByName(str)
			animationblue.addFrame(frame)
		}
		cocos.animationCache.add(animationblue, "dance_blue")
		
		var normal = cocos.animationCache.get("dance")
		var dance_gray =cocos.animationCache.get("dance_gray")
		var dance_blue = cocos.animationCache.get("dance_blue")
		
		var animN = cc.action.Animate(normal)
		var animG = cc.action.Animate(dance_gray)
		var animB = cc.action.Animate(dance_blue)
		
		var seq = cc.action.Sequence(animN, animG, animB)
		
		// create an sprite without texture
		var grossini = cc.Sprite()
		grossini.position = cc.Point(s.width/2, s.height/2)
		addChild(grossini)
		
		grossini.runAction(seq)
	}
	
	function title()
	{
		return   "animationCache";
	}
	
	function subtitle()
	{
		return "Sprite should be animated";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteOffsetAnchorSkew : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		for (var i=0; i<3; i++)
		{
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist")) 
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png")
			
			// Animation using Sprite BatchNode
			var sprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 1)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			
			var anim = cc.Animation()
			for (var i = 1; i<14; i++)
			{
				var str = format("grossini_dance_%02d.png", i)
				var frame = cocos.spriteFrameCache.spriteFrameByName(str)
				anim.addFrame(frame)
			}
			sprite.runAction(cc.action.RepeatForever(cc.action.Animate(2.8, anim, false)))

			
			var skewX = cc.action.SkewBy(2, 45, 0)
			var skewX_back = skewX.reverse()
			
			var skewY = cc.action.SkewBy(2, 0, 45)
			var skewY_back = skewY.reverse()
			
			var seq_skew = cc.action.Sequence(skewX, skewX_back, skewY, skewY_back)
			sprite.runAction(cc.action.RepeatForever(seq_skew))
			
			addChild(sprite, 0)
			
		}
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
	}
	
	function title()
	{
		return  "Sprite offset + anchor + skew";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeOffsetAnchorSkew : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		for (var i=0; i<3; i++)
		{
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist")) 
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png")
		
			// Animation using Sprite BatchNode
			var sprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 200)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			
			var spritebatch = cc.SpriteBatchNode(pack.locate("grossini.png"))
			addChild(spritebatch)
			
			var anim = cc.Animation()
			for (var i = 1; i<14; i++)
			{
				var str = format("grossini_dance_%02d.png", i)
				var frame = cocos.spriteFrameCache.spriteFrameByName(str)
				anim.addFrame(frame)
			}
			sprite.runAction(cc.action.RepeatForever(cc.action.Animate(2.8, anim, false)))
			
			var skewX = cc.action.SkewBy(2, 45, 0)
			var skewX_back = skewX.reverse()
			
			var skewY = cc.action.SkewBy(2, 0, 45)
			var skewY_back = skewY.reverse()
			
			var seq_skew = cc.action.Sequence(skewX, skewX_back, skewY, skewY_back)
			sprite.runAction(cc.action.RepeatForever(seq_skew))
			
			addChild(sprite, i)
			
		}
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
	}
	
	function title()
	{
		return  "SpriteBatchNode offset + anchor + skew";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteOffsetAnchorSkewScale : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist")) 
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png")
		
		for (var i=0; i<3; i++)
		{
			// Animation using Sprite BatchNode
			var sprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 1)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			
			var anim = cc.Animation()
			for (var i = 1; i<14; i++)
			{
				var str = format("grossini_dance_%02d.png", i)
				var frame = cocos.spriteFrameCache.spriteFrameByName(str)
				anim.addFrame(frame)
			}
			sprite.runAction(cc.action.RepeatForever(cc.action.Animate(2.8, anim, false)))
			
			//Skew
			var skewX = cc.action.SkewBy(2, 45, 0)
			var skewX_back = skewX.reverse()
			
			var skewY = cc.action.SkewBy(2, 0, 45)
			var skewY_back = skewY.reverse()
			
			var seq_skew = cc.action.Sequence(skewX, skewX_back, skewY, skewY_back)
			sprite.runAction(cc.action.RepeatForever(seq_skew))
			
			//Scale
			var scale = cc.action.ScaleBy(2, 2)
			var scale_back = scale.reverse()
			var seq_scale = cc.action.Sequence(scale, scale_back)
			sprite.runAction(cc.action.RepeatForever(seq_scale))
			addChild(sprite, 0)
			
		}
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
	}
	
	function title()
	{
		return  "Sprite offset + skew + scale";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeOffsetAnchorSkewScale : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		for (var i=0; i<3; i++)
		{
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist")) 
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png")
		
			// Animation using Sprite BatchNode
			var sprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 200)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			
			var spritesheet = cc.SpriteBatchNode(pack.locate("grossini.png"))
			addChild(spritesheet)
			
			var anim = cc.Animation()
			for (var i = 1; i<14; i++)
			{
				var str = format("grossini_dance_%02d.png", i)
				var frame = cocos.spriteFrameCache.spriteFrameByName(str)
				anim.addFrame(frame)
			}
			sprite.runAction(cc.action.RepeatForever(cc.action.Animate(2.8, anim, false)))
			
			//Skew
			var skewX = cc.action.SkewBy(2, 45, 0)
			var skewX_back = skewX.reverse()
			
			var skewY = cc.action.SkewBy(2, 0, 45)
			var skewY_back = skewY.reverse()
			
			var seq_skew = cc.action.Sequence(skewX, skewX_back, skewY, skewY_back)
			sprite.runAction(cc.action.RepeatForever(seq_skew))
			
			//Scale
			var scale = cc.action.ScaleBy(2, 2)
			var scale_back = scale.reverse()
			var seq_scale = cc.action.Sequence(scale, scale_back)
			sprite.runAction(cc.action.RepeatForever(seq_scale))
			
			spritesheet.addChild(sprite, i)
			
		}
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
	}
	
	function title()
	{
		return  "SpriteBatchNode offset + skew + scale";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteOffsetAnchorFlip : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist")) 
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png")
		
		for (var i=0; i<3; i++)
		{
			// Animation using Sprite BatchNode
			var sprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 1)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			
			var anim = cc.Animation()
			for (var i = 1; i<14; i++)
			{
				var str = format("grossini_dance_%02d.png", i)
				var frame = cocos.spriteFrameCache.spriteFrameByName(str)
				anim.addFrame(frame)
			}
			sprite.runAction(cc.action.RepeatForever(cc.action.Animate(2.8, anim, false)))
			
			var flip = cc.action.FlipY(true)
			var flip_back = cc.action.FlipY(false)
			
			var delay = cc.action.DelayTime(1)
			var delay1 = cc.action.DelayTime(1)
			var seq = cc.action.Sequence(delay, flip, delay1, flip_back)

			sprite.runAction(cc.action.RepeatForever(seq))
			addChild(sprite, 0)
		}
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
	}
	
	function title()
	{
		return "Sprite offset + anchor + flip";
	}
	
	function subtitle()
	{
		return "issue #1078";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteBatchNodeOffsetAnchorFlip : SpriteTestDemo
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		for (var i=0; i<3; i++)
		{
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini.plist")) 
			cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("grossini_gray.plist"), "grossini_gray.png")
		
			// Animation using Sprite BatchNode
			var sprite = cc.Sprite(cocos.spriteFrameCache.spriteFrameByName("grossini_dance_01.png"))
			sprite.position = cc.Point(s.width/4*(i+1), s.height/2)
			
			var point =  cc.Sprite(pack.locate("r1.png"))
			point.scale(0.25)
			point.position = sprite.position
			addChild(point, 200)
			
			switch (i)
			{
				case 0:	sprite.anchorPoint= cc.Point(0,0) 		
						break
				case 1: sprite.anchorPoint= cc.Point(0.5, 0.5)	
						break
				case 2: sprite.anchorPoint= cc.Point(1,1) 		
						break
			}
			point.position = sprite.position
			
			var spritesheet = cc.SpriteBatchNode(pack.locate("grossini.png"))
			addChild(spritesheet)
			
			var anim = cc.Animation()
			for (var i = 1; i<14; i++)
			{
				var str = format("grossini_dance_%02d.png", i)
				var frame = cocos.spriteFrameCache.spriteFrameByName(str)
				anim.addFrame(frame)
			}
			sprite.runAction(cc.action.RepeatForever(cc.action.Animate(2.8, anim, false)))
			
			var flip = cc.action.FlipY(true)
			var flip_back = cc.action.FlipY(false)
			
			var delay = cc.action.DelayTime(1)
			var delay1 = cc.action.DelayTime(1)
			var seq = cc.action.Sequence(delay, flip, delay1, flip_back)

			sprite.runAction(cc.action.RepeatForever(seq))
			spritesheet.addChild(sprite, i)
		}
	}
		
	function onExit()
	{
		print(this + ": onExit")
		
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini.plist"))
		cocos.spriteFrameCache.removeSpriteFramesFromFile(pack.locate("grossini_gray.plist"))
	}
	
	function title()
	{
		return "SpriteBatchNode offset + anchor + flip";
	}
	
	function subtitle()
	{
		return "issue #1078";
	}
}

////////////////////////////////////////////////////////////////////////////////

class SpriteTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = nextSpriteTestAction()
		addChild(pLayer);
		cocos.director.replaceScene(this)
	}
}

return SpriteTestScene()