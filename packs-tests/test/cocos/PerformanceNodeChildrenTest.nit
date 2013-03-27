var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var TAG =
{
	INFO_LAYER 		= 1
	MAIN_LAYER 		= 2
	LABEL_ATLAS 	= 3

	BASE			= 20000
}

var TEST_COUNT		= 4
	
var MAX_NODES 		= 15000
var NODES_INCREASE 	= 500
	
var curCase 		= 0

var function runNodeChildrenTest()
{
    var scene = IterateSpriteSheetCArray()
    scene.initWith_quantityOfNodes(NODES_INCREASE);
	cocos.director.replaceScene(scene);
}

////////////////////////////////////////////////////////////////////////////////

class NodeChildrenMenuLayer : PerformBasicLayer
{	
	constructor(bControlMenuVisible, nMaxCases, nCurCase)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
	}
	
	function showCurrentTest()
	{
		var nNodes = parent.get_quantityOfNodes()
		var pScene = null
		
		switch (_curCase)
		{
		case 0:
			pScene = IterateSpriteSheetCArray();
			break;
		case 1:
			pScene = AddSpriteSheet();
			break;
		case 2:
			pScene = RemoveSpriteSheet();
			break;
		case 3:
			pScene = ReorderSpriteSheet();
			break;
		}
		curCase = _curCase

		if (pScene)
		{
			pScene.initWith_quantityOfNodes(nNodes)
			cocos.director.replaceScene(pScene)
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class NodeChildrenMainScene : cc.ScriptScene
{
	var _lastRenderedCount = null
    var _quantityOfNodes = null
    var _currentQuantityOfNodes = null
	
	function get_quantityOfNodes()
	{
		return _quantityOfNodes;
	}
	
	function update_quantityOfNodes()
	{
	}
	
	function initWith_quantityOfNodes(nNodes)
	{
		var s = cocos.director.winSize
		
		//title label
		var label = cc.LabelTTF(title(), "Arial", 40)
		addChild(label, 1)
		label.position = cc.Point(s.width/2, s.height- 32)
		label.color = nit.Color(255/255, 255/255, 40/255, 0)
		var strSubTitle = subtitle()
		if (strSubTitle.len()!=0)
		{
			var l = cc.LabelTTF(strSubTitle, "Arial", 16)
			addChild(l, 1)
			l.position = cc.Point(s.width/2, s.height -80)
		}
		
		_lastRenderedCount = 0;
		_currentQuantityOfNodes = 0;
		_quantityOfNodes = nNodes;
		
		cc.MenuItemFont.setDefaultFontSize(65)
		var decrease = cc.MenuItemFont("-", this, onDecrease)
		decrease.color = nit.Color(0, 200/255, 20/255, 1)
		var increase = cc.MenuItemFont("+", this, onIncrease)
		increase.color = nit.Color(0, 200/255, 20/255, 1)
		
		var menu = cc.Menu(decrease, increase)
		menu.alignItemsHorizontally()
		menu.position = cc.Point(s.width/2, s.height/2+15)
		addChild(menu, 1)
		
		var infoLabel = cc.LabelTTF("0 nodes", "Arial", 30)
		infoLabel.color = nit.Color(0, 200/255, 20/255, 0)
		infoLabel.position = cc.Point(s.width/2, s.height/2 - 15)
		addChild(infoLabel, 1, TAG.INFO_LAYER)
		
		var pMenu = NodeChildrenMenuLayer(true, TEST_COUNT, curCase)
		addChild(pMenu)

		updateQuantityLabel()
		update_quantityOfNodes()
	}
	
	function title()
	{
		return "No title"
	}
	
	function subtitle()
	{
		return ""
	}
	
	function updateQuantityLabel()
	{
		if (_quantityOfNodes != _lastRenderedCount)
		{
			var infoLabel = getChildByTag(TAG.INFO_LAYER)
			var str = "" + _quantityOfNodes+ " nodes"
			infoLabel.string = str

			_lastRenderedCount = _quantityOfNodes;
		}
	}
	
	function onIncrease(pSender)
	{
		_quantityOfNodes += NODES_INCREASE
		if (_quantityOfNodes > MAX_NODES)
			_quantityOfNodes = MAX_NODES

		updateQuantityLabel()
		update_quantityOfNodes()
	}

	function onDecrease(pSender)
	{
		_quantityOfNodes -= NODES_INCREASE
		if (_quantityOfNodes < 0)
			_quantityOfNodes = 0

		updateQuantityLabel()
		update_quantityOfNodes()
	}
}

////////////////////////////////////////////////////////////////////////////////

class IterateSpriteSheet : NodeChildrenMainScene
{
	batchNode = null
	
	function title()
	{
		var str = "A ("+subtestNumber+") Size=4"
		return str;
	}
	
	function update(dt)
	{
	}

	function update_quantityOfNodes()
	{
		var s = cocos.director.winSize
		
		//increase nodes
		if (_currentQuantityOfNodes < _quantityOfNodes)
		{
			for (var i = 0; i < (_quantityOfNodes-_currentQuantityOfNodes); i++)
			{
				var sprite = cc.Sprite(batchNode.texture, cc.Rect(0, 0, 32, 32))
				batchNode.addChild(sprite)
				sprite.position=cc.Point(math.random()*s.width, math.random()*s.height)
			}
		}
		
		// decrease nodes
		else if (_currentQuantityOfNodes > _quantityOfNodes)
		{
			for (var i = 0; i < (_currentQuantityOfNodes-_quantityOfNodes); i++)
			{
				var index = _currentQuantityOfNodes-i-1;
				batchNode.removeChildAtIndex(index, true);
			}
		}
		
		_currentQuantityOfNodes = _quantityOfNodes;
	}
	
	function initWith_quantityOfNodes(nNodes)
	{
		batchNode = cc.SpriteBatchNode(pack.locate("spritesheet1.png"))
		addChild(batchNode)

		base.initWith_quantityOfNodes(nNodes)

		cocos.director.scheduler.repeat(this, @(evt)=>update(evt.delta), 0.01)
	}
}

////////////////////////////////////////////////////////////////////////////////

class IterateSpriteSheetFastEnum : IterateSpriteSheet
{
	function title()
	{
		return  "A - Iterate SpriteSheet"
	}
	function subtitle()
	{
		return  "Iterate children using Fast Enum API. See console";
	}

	function update(dt)
	{
		// iterate using fast enumeration protocol
		foreach (sprite in batchNode.children)
		{
			sprite.visible = false
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class IterateSpriteSheetCArray : IterateSpriteSheet
{
	function title()
	{
		return  "B - Iterate SpriteSheet";
	}
	
	function subtitle()
	{
		return  "Iterate children using C Array API. See console";
	}

	function update(dt)
	{
		// iterate using fast enumeration protocol
		var pChildren = batchNode.children
		var pObject = null;
		
		foreach (k, v in pChildren)
		{
			var pSprite = v
			pSprite.visible = false
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class AddRemoveSpriteSheet : NodeChildrenMainScene
{
	batchNode = null
	
	function update(dt)
	{
	}
	
	function initWith_quantityOfNodes(nNodes)
	{
		batchNode = cc.SpriteBatchNode(pack.locate("spritesheet1.png"))
		addChild(batchNode)
		
		base.initWith_quantityOfNodes(nNodes)
		
		cocos.director.scheduler.repeat(this, @(evt)=>update(evt.delta), 0.02) 
	}
	
	function update_quantityOfNodes()
	{
		var s = cocos.director.winSize
		
		//increase nodes
		if (_currentQuantityOfNodes < _quantityOfNodes)
		{
			for (var i = 0; i < (_quantityOfNodes-_currentQuantityOfNodes); i++)
			{
				var sprite = cc.Sprite(batchNode.texture, cc.Rect(0, 0, 32, 32))
				batchNode.addChild(sprite)
				sprite.position=cc.Point(math.random()*s.width, math.random()*s.height)
				sprite.visible = false
			}
		}
		
		// decrease nodes
		else if (_currentQuantityOfNodes > _quantityOfNodes)
		{
			for (var i = 0; i < (_currentQuantityOfNodes-_quantityOfNodes); i++)
			{
				var index = _currentQuantityOfNodes-i-1;
				batchNode.removeChildAtIndex(index, true);
			}
		}
		
		_currentQuantityOfNodes = _quantityOfNodes;
	}
}

////////////////////////////////////////////////////////////////////////////////

class AddSpriteSheet : AddRemoveSpriteSheet
{
	function title()
	{
		return  "C - Add to spritesheet";
	}
	function subtitle()
	{
		return  "Adds %10 of total sprites with random z. See console";
	}

	function update(dt)
	{
		var totalToAdd = _currentQuantityOfNodes * 0.15
		if (totalToAdd > 0)
		{
			var sprites = []
			var zs = []
			
			// Don't include the sprite creation time and random as part of the profiling
			for (var i=0; i < totalToAdd; i++)
			{
				var pSprite = cc.Sprite(batchNode.texture, cc.Rect(0,0,32,32))
				sprites.push(pSprite)
				zs.insert(i,(math.random()*2 -1) * 50)
			}
			
			for (var i=0; i < totalToAdd; i++)
			{
				batchNode.addChild(sprites[i], zs[i], TAG.BASE+i);
			}
			
			// remove them
			for (var i=0; i < totalToAdd; i++)
			{
				batchNode.removeChildByTag(TAG.BASE+i, true);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class RemoveSpriteSheet : AddRemoveSpriteSheet
{
	function title()
	{
		return  "D - Del from spritesheet"
	}
	
	function subtitle()
	{
		return  "Remove %10 of total sprites placed randomly. See console";
	}

	function update(dt)
	{
		var totalToAdd = _currentQuantityOfNodes * 0.15
		if (totalToAdd > 0)
		{
			var sprites = []
			
			// Don't include the sprite creation time and random as part of the profiling
			for (var i=0; i<totalToAdd; i++)
			{
				var pSprite = cc.Sprite(batchNode.texture, cc.Rect(0,0,32,32))
				sprites.push(pSprite)
			}
			
			// add them with random Z (very important!)
			for (var i=0; i < totalToAdd;i++)
			{
				batchNode.addChild(sprites[i], (math.random()*2 -1) * 50, TAG.BASE+i);
			}
			
			// remove them
			for (var i=0;i <  totalToAdd;i++)
			{
				batchNode.removeChildByTag(TAG.BASE+i, true);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

class ReorderSpriteSheet : AddRemoveSpriteSheet
{
	function title()
	{
		return  "E - Reorder from spritesheet";
	}
	function subtitle()
	{
		return  "Reorder %10 of total sprites placed randomly. See console";
	}

	function update(dt)
	{
		var totalToAdd = _currentQuantityOfNodes * 0.15
		if (totalToAdd > 0)
		{
			var sprites = []
			
			// Don't include the sprite creation time and random as part of the profiling
			for (var i=0; i<totalToAdd; i++)
			{
				var pSprite = cc.Sprite(batchNode.texture, cc.Rect(0,0,32,32))
				sprites.push(pSprite)
			}
			
			// add them with random Z (very important!)
			for (var i=0; i < totalToAdd;i++)
			{
				batchNode.addChild(sprites[i], (math.random()*2 -1) * 50, TAG.BASE+i);
			}
			
			// reorder them
			 for (var i=0;i <  totalToAdd;i++)
			{
				var pNode = batchNode.children[i]
				batchNode.reorderChild(pNode,(math.random()*2 -1) * 50);
			}
			
			// remove them
			for (var i=0;i <  totalToAdd;i++)
			{
				batchNode.removeChildByTag(TAG.BASE+i, true);
			}
		}
	}
}