var pack = script.locator

kTagInfoLayer 		:= 1
kTagMainLayer 		:= 2
kTagLabelAtlas 		:= 3

kTagBase			:= 20000

TEST_COUNT			:= 4
	
kMaxNodes 			:= 15000
kNodesIncrease 		:= 500
	
s_nCurCase 			:= 0

function runNodeChildrenTest()
{
    var pScene = IterateSpriteSheetCArray()
    pScene.initWithQuantityOfNodes(kNodesIncrease);
	cocos.director.replaceScene(pScene);
}

////////////////////////////////////////////////////////
//
// NodeChildrenMenuLayer
//
////////////////////////////////////////////////////////
class NodeChildrenMenuLayer : PerformBasicLayer
{	
	constructor(bControlMenuVisible, nMaxCases, nCurCase)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
	}
	
	function showCurrentTest()
	{
		var nNodes = parent.getQuantityOfNodes()
		var pScene = null
		
		switch (m_nCurCase)
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
		s_nCurCase = m_nCurCase

		if (pScene)
		{
			pScene.initWithQuantityOfNodes(nNodes)
			cocos.director.replaceScene(pScene)
		}
	}
}
////////////////////////////////////////////////////////
//
// NodeChildrenMainScene
//
////////////////////////////////////////////////////////
class NodeChildrenMainScene : cc.ScriptScene
{
	lastRenderedCount = null
    quantityOfNodes = null
    currentQuantityOfNodes = null
	
	function getQuantityOfNodes()
	{
		return quantityOfNodes;
	}
	function updateQuantityOfNodes()
	{
	}
	
	function initWithQuantityOfNodes(nNodes)
	{
		var s = cocos.director.winSize
		
		//title label
		var label = cc.LabelTTF( title(), "Arial", 40)
		addChild(label, 1)
		label.position = cc.Point(s.width/2, s.height- 32)
		label.color = nit.Color(255/255, 255/255, 40/255, 0)
		var strSubTitle = subtitle()
		if (strSubTitle.len()!=0)
		{
			var l = cc.LabelTTF(strSubTitle, "Thonburi", 16)
			addChild(l, 1)
			l.position = cc.Point(s.width/2, s.height -80)
		}
		
		lastRenderedCount = 0;
		currentQuantityOfNodes = 0;
		quantityOfNodes = nNodes;
		
		cc.MenuItemFont.setDefaultFontSize(65)
		var decrease = cc.MenuItemFont("-", this, onDecrease)
		decrease.color = nit.Color(0, 200/255, 20/255, 1)
		var increase = cc.MenuItemFont("+", this, onIncrease)
		increase.color = nit.Color(0, 200/255, 20/255, 1)
		
		var menu = cc.Menu(decrease, increase)
		menu.alignItemsHorizontally()
		menu.position = cc.Point(s.width/2, s.height/2+15)
		addChild(menu, 1)
		
		var infoLabel = cc.LabelTTF("0 nodes", "Marker Felt", 30)
		infoLabel.color = nit.Color(0, 200/255, 20/255, 0)
		infoLabel.position = cc.Point(s.width/2, s.height/2 - 15)
		addChild(infoLabel, 1, kTagInfoLayer)
		
		var pMenu = NodeChildrenMenuLayer(true, TEST_COUNT, s_nCurCase)
		addChild(pMenu)

		updateQuantityLabel()
		updateQuantityOfNodes()
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
		if( quantityOfNodes != lastRenderedCount )
		{
			var infoLabel = getChildByTag(kTagInfoLayer)
			var str = "" + quantityOfNodes+ " nodes"
			infoLabel.string = str

			lastRenderedCount = quantityOfNodes;
		}
	}
	
	function onIncrease(pSender)
	{
		quantityOfNodes += kNodesIncrease
		if( quantityOfNodes > kMaxNodes )
			quantityOfNodes = kMaxNodes

		updateQuantityLabel()
		updateQuantityOfNodes()
	}

	function onDecrease(pSender)
	{
		quantityOfNodes -= kNodesIncrease
		if( quantityOfNodes < 0 )
			quantityOfNodes = 0

		updateQuantityLabel()
		updateQuantityOfNodes()
	}
}
////////////////////////////////////////////////////////
//
// IterateSpriteSheet
//
////////////////////////////////////////////////////////
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

	function updateQuantityOfNodes()
	{
		var s = cocos.director.winSize
		//increase nodes
		if( currentQuantityOfNodes < quantityOfNodes )
		{
			for(var i = 0; i < (quantityOfNodes-currentQuantityOfNodes); i++)
			{
				var sprite = cc.Sprite(batchNode.texture, cc.Rect(0, 0, 32, 32))
				batchNode.addChild(sprite)
				sprite.position=cc.Point(math.random()*s.width, math.random()*s.height)
			}
		}// decrease nodes
		else if ( currentQuantityOfNodes > quantityOfNodes )
		{
			for(var i = 0; i < (currentQuantityOfNodes-quantityOfNodes); i++)
			{
				var index = currentQuantityOfNodes-i-1;
				batchNode.removeChildAtIndex(index, true);
			}
		}
		currentQuantityOfNodes = quantityOfNodes;
	}
	
	function initWithQuantityOfNodes(nNodes)
	{
		batchNode = cc.SpriteBatchNode(pack.locate("spritesheet1.png"))
		addChild(batchNode)

		base.initWithQuantityOfNodes(nNodes)

		session.scheduler.repeat(this, @(evt)=>update(evt.delta), 0.01)
	}
}
////////////////////////////////////////////////////////
//
// IterateSpriteSheetFastEnum
//
////////////////////////////////////////////////////////
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
		var pChildren = batchNode.children
		var pObject = null;
		
		foreach(k, v in pChildren)
		{
			var pSprite = v
			pSprite.visible = false
		}
	}
}
////////////////////////////////////////////////////////
//
// IterateSpriteSheetCArray
//
////////////////////////////////////////////////////////
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
		
		foreach(k, v in pChildren)
		{
			var pSprite = v
			pSprite.visible = false
		}
	}
}
////////////////////////////////////////////////////////
//
// AddRemoveSpriteSheet
//
////////////////////////////////////////////////////////
class AddRemoveSpriteSheet : NodeChildrenMainScene
{
	batchNode = null
	
	function update(dt)
	{
	}
	
	function initWithQuantityOfNodes(nNodes)
	{
		batchNode = cc.SpriteBatchNode(pack.locate("spritesheet1.png"))
		addChild(batchNode)
		
		base.initWithQuantityOfNodes(nNodes)
		
		session.scheduler.repeat(this, @(evt)=>update(evt.delta), 0.02) 
	}
	
	function updateQuantityOfNodes()
	{
		var s = cocos.director.winSize
		//increase nodes
		if( currentQuantityOfNodes < quantityOfNodes )
		{
			for(var i = 0; i < (quantityOfNodes-currentQuantityOfNodes); i++)
			{
				var sprite = cc.Sprite(batchNode.texture, cc.Rect(0, 0, 32, 32))
				batchNode.addChild(sprite)
				sprite.position=cc.Point(math.random()*s.width, math.random()*s.height)
				sprite.visible = false
			}
		}// decrease nodes
		else if ( currentQuantityOfNodes > quantityOfNodes )
		{
			for(var i = 0; i < (currentQuantityOfNodes-quantityOfNodes); i++)
			{
				var index = currentQuantityOfNodes-i-1;
				batchNode.removeChildAtIndex(index, true);
			}
		}
		currentQuantityOfNodes = quantityOfNodes;
	}
}

////////////////////////////////////////////////////////
//
// AddSpriteSheet
//
////////////////////////////////////////////////////////
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
		var totalToAdd = currentQuantityOfNodes * 0.15
		if (totalToAdd > 0)
		{
			var sprites = []
			var zs = []
			// Don't include the sprite creation time and random as part of the profiling
			for(var i=0; i<totalToAdd; i++)
			{
				var pSprite = cc.Sprite(batchNode.texture, cc.Rect(0,0,32,32))
				sprites.push(pSprite)
				zs.insert(i,(math.random()*2 -1) * 50)
			}
			
			for( var i=0; i < totalToAdd;i++ )
			{
				batchNode.addChild( sprites[i], zs[i], kTagBase+i);
			}
			// remove them
			for( var i=0;i <  totalToAdd;i++)
			{
				batchNode.removeChildByTag(kTagBase+i, true);
			}
		}
	}
}
////////////////////////////////////////////////////////
//
// RemoveSpriteSheet
//
////////////////////////////////////////////////////////
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
		var totalToAdd = currentQuantityOfNodes * 0.15
		if (totalToAdd > 0)
		{
			var sprites = []
			// Don't include the sprite creation time and random as part of the profiling
			for(var i=0; i<totalToAdd; i++)
			{
				var pSprite = cc.Sprite(batchNode.texture, cc.Rect(0,0,32,32))
				sprites.push(pSprite)
			}
			// add them with random Z (very important!)
			for( var i=0; i < totalToAdd;i++ )
			{
				batchNode.addChild( sprites[i], (math.random()*2 -1) * 50, kTagBase+i);
			}
			// remove them
			for( var i=0;i <  totalToAdd;i++)
			{
				batchNode.removeChildByTag(kTagBase+i, true);
			}
		}
	}
}
////////////////////////////////////////////////////////
//
// ReorderSpriteSheet
//
////////////////////////////////////////////////////////
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
		var totalToAdd = currentQuantityOfNodes * 0.15
		if (totalToAdd > 0)
		{
			var sprites = []
			// Don't include the sprite creation time and random as part of the profiling
			for(var i=0; i<totalToAdd; i++)
			{
				var pSprite = cc.Sprite(batchNode.texture, cc.Rect(0,0,32,32))
				sprites.push(pSprite)
			}
			// add them with random Z (very important!)
			for( var i=0; i < totalToAdd;i++ )
			{
				batchNode.addChild( sprites[i], (math.random()*2 -1) * 50, kTagBase+i);
			}
			// reorder them
			 for( var i=0;i <  totalToAdd;i++)
			{
				var pNode = batchNode.children[i]
				batchNode.reorderChild(pNode,(math.random()*2 -1) * 50 );
			}
			// remove them
			for( var i=0;i <  totalToAdd;i++)
			{
				batchNode.removeChildByTag(kTagBase+i, true);
			}
		}
	}
}