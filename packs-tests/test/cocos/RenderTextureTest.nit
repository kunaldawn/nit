var pack = script.locator

sceneIdx 	:= -1; 

MAX_LAYER 	:= 4

class RenderTextureScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = nextTestCase()
		addChild(pLayer)
		cocos.director.replaceScene(this)
	}
}

function createTestCase(nIndex)
{
	switch(nIndex)
	{
		case 0: return RenderTextureTest();
		case 1: return RenderTextureIssue937();
		case 2: return RenderTextureZbuffer();
		case 3: return RenderTextureSave();
	}  

	return NULL;
}

function nextTestCase()
{
	sceneIdx++;
	sceneIdx = sceneIdx % MAX_LAYER;

	var pLayer = createTestCase(sceneIdx);
	
	return pLayer;
}

function backTestCase()
{
	sceneIdx--;
	var total = MAX_LAYER;
	if( sceneIdx < 0 )
		sceneIdx += total;	
	
	var pLayer = createTestCase(sceneIdx);

	return pLayer;
}

function restartTestCase()
{
	var pLayer = createTestCase(sceneIdx);

	return pLayer;
} 


class RenderTextureTestDemo : cc.ScriptLayer
{
	m_atlas = null
	m_strTitle = null
	
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		var x = size.width
		var y = size.height
	
		var label = cc.LabelTTF( title(), "Arial", 28)
		label.position = cc.Point(x/2, y-50)
		addChild(label, 1)
		
		var strSubtitle = subtitle()
		if ( strSubtitle != "")
		{
			var l = cc.LabelTTF( strSubtitle, "Thonburi", 16)
			addChild(l, 1)
			l.position = cc.Point(size.width/2, size.height-80)
		}
		
		var item1 = cc.MenuItemImage(s_pPathB1,s_pPathB2, this,backCallBack);
		var item2 = cc.MenuItemImage(s_pPathR1, s_pPathR2, this,restartCallBack)	
		var item3 = cc.MenuItemImage(s_pPathF1, s_pPathF2, this,nextCallBack);
		
		var menu = cc.Menu(item1, item2, item3);
		menu.position= cc.Point(0,0);
		item1.position= cc.Point( size.width/2 - 100,30) ;
		item2.position= cc.Point( size.width/2, 30) ;
		item3.position= cc.Point( size.width/2 + 100,30) ;
		this.addChild( menu, 1 );	
		
	}
	
	function title()
	{
		return  "Render Texture Test";
	}
	
	function subtitle()
	{
		return "";
	}
	
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		var s = RenderTextureScene()
		s.addChild( restartTestCase() )
		cocos.director.replaceScene(s)
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = RenderTextureScene()
		s.addChild( nextTestCase() )
		cocos.director.replaceScene(s)
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		var s = RenderTextureScene()
		s.addChild( backTestCase() )
		cocos.director.replaceScene(s)
	}
}

//------------------------------------------------------------------
// RenderTextureTest
//------------------------------------------------------------------
class RenderTextureTest : RenderTextureTestDemo
{
	m_target = null
	m_brush	= null
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		
		// create a render texture, this is what we're going to draw into
		m_target = cc.RenderTexture(s.width, s.height)
		
		if (null == m_target)
		{
			return;
		}
		
		var ctx = render.beginContext(cocos.renderView)
		m_target.clear(ctx)
		render.endContext(ctx)
		
		m_target.position = cc.Point(s.width/2, s.height/2)
		// note that the render texture is a cocosnode, and contains a sprite of it's texture for convience,
		// so we can just parent it to the scene like any other cocos node
		addChild(m_target, 1);
		
		// create a brush image to draw into the texture with
		m_brush = cc.Sprite(pack.locate("stars.png"))
		
		m_brush.blendFuncSrc = cc.Sprite.SRC_BLEND_ONE
		m_brush.blendFuncDst = cc.Sprite.DST_BLEND_ONE_MINUS_SRC_ALPHA
		m_brush.opacity = 20
		
		this.touchEnabled = true;
		this.channel().priority(-10000).bind(Events.OnCCTouchMoved, this, cctouchesMoved)
		this.channel().priority(-10000).bind(Events.OnCCTouchEnded, this, cctouchesEnded)
	}
	
	function cctouchesMoved(evt: cc.TouchEvent)
	{
		var touch = evt.touches[0]
		var start = touch.point
		start = cocos.director.toGl(start)
		var end = touch.prevPoint
		end = cocos.director.toGl(end)
		
		var ctx = render.beginContext(cocos.renderView)
		
		//begin drawing to the render texture
		m_target.begin(ctx)
		
		 // for extra points, we'll draw this smoothly from the last position and vary the sprite's
		// scale/rotation/offset
		var distance =  math.sqrt( math.pow((start.x-end.x), 2) + math.pow((start.y-end.y),2) )
		if (distance > 1)
		{
			var d = distance
			for( var i=0; i<d; i++)
			{
				var difx = end.x - start.x
				var dify = end.y - start.y
				var delta = i/distance
				m_brush.position = cc.Point(start.x + (difx*delta), start.y + (dify*delta))
				m_brush.rotation = math.rand()%360
				var r = math.rand()%50/50.0 + 0.25
				m_brush.scale(r)
				// Call visit to draw the brush, don't call draw..
				m_brush.renderVisit(ctx)
			}
		}
		
		//#if CC_ENABLE_CACHE_TEXTTURE_DATA == if platform is android then
		//m_target.End()
		m_target.end(ctx)
		
		render.endContext(ctx)
	}
	
	function cctouchesEnded(evt: cc.TouchEvent)
	{
	//#if CC_ENABLE_CACHE_TEXTTURE_DATA == if platform is android then
		// for (var i = 0; i < evt.touches.len(); i++)
		// {
			// var touch = evt.touches[i]

			// if(!touch)
				// break
			
			// var location = touch.locationInView(touch.view)
			// location = cocos.director.toGl(location);
		
			// m_brush.position = location
			// m_brush.Rotation = rand()%360
		// }
		// m_target.Begin()
		// m_brush.Visit()
		// m_target.End(true)
	//#endif
	}

	function OnExit()
	{
		print(this + ": onExit")
				
	}
}

//------------------------------------------------------------------
// RenderTextureSave
//------------------------------------------------------------------
class RenderTextureSave : RenderTextureTestDemo
{
	counter = 0
	m_target = null
	m_brush	= null
	constructor()
	{
		base.constructor()
	
		var s = cocos.director.winSize
		
		var ctx = render.beginContext(cocos.renderView)
		
		// create a render texture, this is what we're going to draw into
		m_target = cc.RenderTexture(s.width, s.height)
		m_target.clear(ctx)
		m_target.position = cc.Point(s.width/2, s.height/2)
		
		::rt := m_target
		
		render.endContext(ctx)
		
		// note that the render texture is a cocosnode, and contains a sprite of it's texture for convience,
		// so we can just parent it to the scene like any other cocos node
		addChild(m_target, -1);
		
		// create a brush image to draw into the texture with
		m_brush = cc.Sprite(pack.locate("fire.png"))
		m_brush.opacity = 20
		
		//Save Image Menu
		var item1 = cc.MenuItemFont("Save Image", this, saveImage)
		item1.fontSize = 16
		var item2 = cc.MenuItemFont("clear", this, clearImage)
		item2.fontSize = 16
		
		var menu = cc.Menu( item1, item2)
		this.addChild(menu)
		menu.alignItemsVertically()
		menu.position = cc.Point(s.width - 80, s.height - 30)
		
		this.touchEnabled = true;
		this.channel().priority(-10000).bind(Events.OnCCTouchMoved, this, cctouchesMoved)
	}
	
	function cctouchesMoved(evt: cc.TouchEvent)
	{
		var touch = evt.touches[0]
		var start = touch.point
		start = cocos.director.toGl(start);
		var end = touch.prevPoint
		
		var ctx = render.beginContext(cocos.renderView)
		
		//begin drawing to the render texture
		m_target.begin(ctx)
		 // for extra points, we'll draw this smoothly from the last position and vary the sprite's
		// scale/rotation/offset
		var distance =  math.sqrt( math.pow((start.x-end.x), 2) + math.pow((start.y-end.y),2) )
		if (distance > 1)
		{
			var d = distance
			for( var i=0; i<d; i++)
			{
				var difx = end.x - start.x
				var dify = end.y - start.y	
				var delta = i/distance
				m_brush.position = cc.Point(start.x + (difx*delta), start.y + (dify*delta))
				m_brush.rotation = math.rand()%360
				var r = math.rand()%50/50.0 + 0.25
				m_brush.scale(r)
				m_brush.color = nit.Color((math.rand()%127 + 128)/255, 255/255, 255/255, 255/255)
				// Call visit to draw the brush, don't call draw..
				m_brush.renderVisit(ctx)
			}
		}
		
		m_target.end(ctx)
		
		render.endContext(ctx)
	}

	function clearImage(pSender)
	{
		var ctx = render.beginContext(cocos.renderView)
		m_target.clear(ctx, math.random(),  math.random(),  math.random(),  math.random())
		render.endContext(ctx)
	}

	function saveImage(pSender)
	{	
		var str = format("image-%03d.png", counter)
		
		var ctx = render.beginContext(cocos.renderView)
		var img = m_target.newImageCopy(ctx)
		render.endContext(ctx)
		
		if (img)
		{
			nit.FileUtil.createDir(app.userSavePath)
			var fl = nit.FileLocator("$imgsave", app.userSavePath)
			img.savePng(fl.create(str))
			print("Image saved: " + str)
			dump(fl.find("*.png"))
		}
		
		counter++
	}
	
	function onExit()
	{
		print(this + ": onExit")
		m_target.cleanup()
		cocos.textureCache.removeUnusedTextures()
	}
	
	function title()
	{	
		return "Touch the screen";
	}
	
	function subtitle()
	{	
		return "Press 'Save Image' to create an snapshot of the render texture";
	}
}

//------------------------------------------------------------------
// RenderTextureIssue937
//------------------------------------------------------------------
class RenderTextureIssue937 : RenderTextureTestDemo
{
	constructor()
	{
		base.constructor()
		/*
		*     1    2
		* A: A1   A2
		*
		* B: B1   B2
		*
		*  A1: premulti sprite
		*  A2: premulti render
		*
		*  B1: non-premulti sprite
		*  B2: non-premulti render
		*/
		var s = cocos.director.winSize
		var background = cc.LayerColor(nit.Color(200/255, 200/255, 200/255, 255/255))
		addChild(background)
		
		var spr_premulti = cc.Sprite(pack.locate("fire.png"))
		spr_premulti.position = cc.Point(16, 48)
		
		var spr_nonpremulti = cc.Sprite(pack.locate("fire.png"))
		spr_nonpremulti.position = cc.Point(16, 16)
		
		/* A2 & B2 setup */
		var rend = cc.RenderTexture(32, 64)
		if (null == rend)
		{
			return;
		}
		
		var ctx = render.beginContext(cocos.renderView)
		rend.clear(ctx)
		
		 // It's possible to modify the RenderTexture blending function by
		//		[[rend sprite] setBlendFunc:(ccBlendFunc) {GL_ONE, GL_ONE_MINUS_SRC_ALPHA}];
		rend.begin(ctx)
		spr_premulti.renderVisit(ctx)
		spr_nonpremulti.renderVisit(ctx)
		rend.end(ctx)

		render.endContext(ctx)
		
		 /* A1: setup */
		spr_premulti.position = cc.Point(s.width/2-16, s.height/2+16)
		/* B1: setup */
		spr_nonpremulti.position =cc.Point(s.width/2-16, s.height/2-16)
		
		rend.position = cc.Point(s.width/2 + 16, s.height/2)
		
		addChild(spr_nonpremulti)
		addChild(spr_premulti)
		addChild(rend)
		
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	function title()
	{	
		return "Testing issue #937";
	}
	
	function subtitle()
	{	
		return "All images should be equal...";
	}
}

/**
* Impelmentation of RenderTextureZbuffer
*/

class RenderTextureZbuffer : RenderTextureTestDemo
{
	mgr = null
	sp = []
	
	constructor()
	{
		base.constructor()
	
		var size = cocos.director.winSize
		
		var label = cc.LabelTTF("vertexZ = 50", "Marker Felt", 64)
		label.position = cc.Point(size.width/2, size.height * 0.25)
		this.addChild(label)
		
		var label2 = cc.LabelTTF("vertexZ = 0", "Marker Felt", 64)
		label2.position = cc.Point(size.width/2, size.height * 0.5)
		this.addChild(label2)
		
		var label3 = cc.LabelTTF("vertexZ = -50", "Marker Felt", 64)
		label3.position = cc.Point(size.width/2, size.height * 0.75)
		this.addChild(label3)
		
		label.vertexZ = 50
		label2.vertexZ = 0
		label3.vertexZ = -50
		
		cocos.spriteFrameCache.addSpriteFramesWithFile(pack.locate("circle.plist"))
		mgr = cc.SpriteBatchNode(pack.locate("circle.png"), 9)
		this.addChild(mgr)
		
		for( var i=0; i<9; i++)
		{
			var sprite = cc.Sprite(pack.locate("circle.png"))
			sprite.vertexZ = 400 - (i*100)
			sp.push( sprite )
			
			mgr.addChild( sprite, 9-i)
		}
		
		sp[8].scale(2)
		sp[8].color = nit.Color(255/255, 255/255, 0, 0)	
		
		this.touchEnabled = true;
		this.channel().priority(-10000).bind(Events.OnCCTouchBegin, this, cctouchesBegin)
		this.channel().priority(-10000).bind(Events.OnCCTouchMoved, this, cctouchesMoved)
		this.channel().priority(-10000).bind(Events.OnCCTouchEnded, this, cctouchesEnded)
	}
	
	function cctouchesBegin(evt: cc.TouchEvent)
	{
		foreach (touch in evt.touches)
		{
			var location = touch.locationInView(touch.view)
			location = cocos.director.toGl(location);
			
			foreach(k, v in sp)
			{
				v.position = location
			}
		}
	}
	
	function cctouchesMoved(evt: cc.TouchEvent)
	{
		foreach (touch in evt.touches)
		{
			var location = touch.locationInView(touch.view)
			location = cocos.director.toGl(location);
			
			foreach(k, v in sp)
			{
				v.position = location
			}
		}
	}
	
	function cctouchesEnded(evt: cc.TouchEvent)
	{
		renderScreenShot()
	}

	function renderScreenShot()
	{
		var texture = cc.RenderTexture(512, 512)
		if (null == texture)
		{
			return
		}
		
		var ctx = render.beginContext(cocos.renderView)
		
		texture.clear(ctx)
		texture.anchorPoint = cc.Point(0,0)
		
		texture.begin(ctx)
		this.renderVisit(ctx)
		texture.end(ctx)
		
		render.endContext(ctx)
		
		var sprite = cc.Sprite(texture.sprite.texture)
		
		sprite.position = cc.Point(256, 256)
		sprite.opacity = 182
		sprite.flipY = true
		this.addChild(sprite, 999999)
		sprite.color = nit.Color.GREEN
		
		sprite.runAction(cc.action.Sequence(cc.action.FadeTo(2, 0), cc.action.Hide()))
	}
	
	function onExit()
	{
		print(this + ": onExit")
	}
	
	function title()
	{	
		return "Testing Z Buffer in Render Texture";
	}
	
	function subtitle()
	{	
		return "Touch screen. It should be green";
	}
}
