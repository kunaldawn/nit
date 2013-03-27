var pack = script.locator

kTagLayer := 1

sceneIdx 	:= -1; 

MAX_LAYER 	:= 1
s_currentOrientation := 0

class DirectorTestScene : TestScene
{
	
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		s_currentOrientation = cocos.director.ORIENT_PORTRAIT_UP;
		
		var pLayer = nextDirectorTestCase()
		addChild(pLayer)
		cocos.director.replaceScene(this)
	}
	
	function mainMenuCallback(pSender)
	{
		cocos.director.deviceOrientation = 0
		base.mainMenuCallback(pSender)
	}
}

function createTestCaseLayer(nIndex)
{
	switch(nIndex)
	{
		case 0:
		{
			var pRet = Director1()
			return pRet;
		}
	}
	return NULL;
}

function nextDirectorTestCase()
{
	sceneIdx++;
	sceneIdx = sceneIdx % MAX_LAYER;

	var pLayer = createTestCaseLayer(sceneIdx);
	
	return pLayer;
}

function backDirectorTestCase()
{
	sceneIdx--;
	var total = MAX_LAYER;
	if( sceneIdx < 0 )
		sceneIdx += total;	
	
	var pLayer = createTestCaseLayer(sceneIdx);

	return pLayer;
}

function restartDirectorTestCase()
{
	var pLayer = createTestCaseLayer(sceneIdx);

	return pLayer;
} 

class DirectorTest : cc.ScriptLayer
{
	var bRet = false;
	constructor()
	{
		base.constructor()
		
		var size = cocos.director.winSize
		var x = size.width
		var y = size.height
	
		var label = cc.LabelTTF( title(), "Arial", 26)
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
		return "No title";
	}
	
	function subtitle()
	{
		return "";
	}
	
	function restartCallBack(evt: cc.MenuItemEvent)
	{
		var s = DirectorTestScene()
		s.addChild( restartDirectorTestCase() )
		cocos.director.replaceScene(s)
	}
	
	function nextCallBack(evt: cc.MenuItemEvent)
	{
		var s = DirectorTestScene()
		s.addChild( nextDirectorTestCase() )
		cocos.director.replaceScene(s)
	}
	
	function backCallBack(evt: cc.MenuItemEvent)
	{
		var s = DirectorTestScene()
		s.addChild( backDirectorTestCase() )
		cocos.director.replaceScene(s)
	}
}

//---------------------------------------
// 
// director1
// 
//---------------------------------------

class Director1 : DirectorTest
{
	m_root = null
	m_target = null
	m_streak = null
	
	constructor()
	{
		base.constructor()
			
		var s = cocos.director.winSize
		var item = cc.MenuItemFont("Rotate Device", this, rotateDevice)
		var menu = cc.Menu(item)
		menu.position = cc.Point(s.width/2, s.height/2)
		addChild(menu)
		
		bRet = true
		
		this.touchEnabled = true;
		this.channel().priority(-10000).bind(Events.OnCCTouchEnded, this, ccTouchesEnded)
	}
	
	function newOrientation()
	{
		switch(s_currentOrientation)
		{
			case cocos.director.ORIENT_LANDSCAPE_LEFT:
				s_currentOrientation = cocos.director.ORIENT_PORTRAIT_UP;
				break;
			case cocos.director.ORIENT_PORTRAIT_UP:
				s_currentOrientation = cocos.director.ORIENT_LANDSCAPE_RIGHT;
				break;						
			case cocos.director.ORIENT_LANDSCAPE_RIGHT:
				s_currentOrientation = cocos.director.ORIENT_PORTRAIT_DOWN;
				break;
			case cocos.director.ORIENT_PORTRAIT_DOWN:
				s_currentOrientation = cocos.director.ORIENT_LANDSCAPE_LEFT;
				break;
		}
		cocos.director.deviceOrientation = s_currentOrientation
	}
	
	function rotateDevice(pSender)
	{
		newOrientation()
		restartCallBack(null)
	}
	
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		foreach (touch in evt.touches)
		{
			var a = touch.locationInView(touch.view)
			
			var director = cocos.director
			var b = director.toUi(director.toGl(a))
			print("("+a.x+","+a.y+") == ("+b.x+","+b.y+")" )
		}
	}
	
	function title()
	{	
		return "Testing conversion";
	}
	
	function subtitle()
	{
		return "Tap screen and see the debug console"
	}
}


return DirectorTestScene()