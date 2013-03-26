var pack = script.locator

kTagLayer := 1

sceneIdx 	:= -1; 

MAX_COUNT 	:= 5
LINE_SPACE  := 40
kItemTagBasic := 1000

class PerformanceTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = PerformanceMainLayer()
		addChild(pLayer)
		cocos.director.replaceScene(this)
	}
}

testsName := 
[
    "PerformanceNodeChildrenTest",
    "PerformanceParticleTest",
    "PerformanceSpriteTest",
    "PerformanceTextureTest",
    "PerformanceTouchesTest"
]

class PerformanceMainLayer : cc.ScriptLayer
{
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
		var pMenu = cc.Menu()
		pMenu.position = cc.Point(0,0)
		
		cc.MenuItemFont.setDefaultFontName("Arial")
		cc.MenuItemFont.setDefaultFontSize(24)
		for( var i = 0; i<MAX_COUNT; i++)
		{	
			var pItem = null
			pItem = cc.MenuItemFont( testsName[i], this, @menuCallback(pItem))
			pItem.position = cc.Point(s.width/2, s.height - (i+1) * LINE_SPACE)
			pMenu.addChild(pItem, kItemTagBasic + i)
		}
		addChild(pMenu)
	}
}

function menuCallback(pSender)
{
	var pItem = pSender
	var nIndex = pItem.zOrder - kItemTagBasic
	
	switch(nIndex)
	{
		case 0:
			require "PerformanceNodeChildrenTest"
			runNodeChildrenTest();
			break;
		case 1:
			require "PerformanceParticleTest"
			runParticleTest();
			break;
		case 2:
			require "PerformanceSpriteTest"
			runSpriteTest();
			break;
		case 3:
			require "PerformanceTextureTest"
			runTextureTest();
			break;
		case 4:
			require "PerformanceTouchesTest"
			runTouchesTest();
			break;
		default:
			break;
	}
}
////////////////////////////////////////////////////////
//
// PerformBasicLayer
//
////////////////////////////////////////////////////////
class PerformBasicLayer : cc.ScriptLayer
{
	m_bControlMenuVisible  = null
	m_nMaxCases = null
	m_nCurCase = null
	
	constructor(bControlMenuVisible, nMaxCasec, nCurCase)
	{
		base.constructor()
		
		m_bControlMenuVisible 	= bControlMenuVisible
		m_nMaxCases				= nMaxCasec
		m_nCurCase				= nCurCase
	}
	
	function onEnter()
	{
		var s = cocos.director.winSize
		
		cc.MenuItemFont.setDefaultFontName("Arial")
		cc.MenuItemFont.setDefaultFontSize(24)
		var pMainItem = cc.MenuItemFont("Back", this, toMainLayer)
		pMainItem.position = cc.Point(s.width-50, 25)
		var pMenu = cc.Menu(pMainItem)
		pMenu.position = cc.Point(0,0)
		
		if (m_bControlMenuVisible)
		{
			var item1 = cc.MenuItemImage(s_pPathB1, s_pPathB2, this, backCallback)
			var item2 = cc.MenuItemImage(s_pPathR1, s_pPathR2, this, restartCallback)
			var item3 = cc.MenuItemImage(s_pPathF1, s_pPathF2, this, nextCallback)
			
			item1.position = cc.Point(s.width/2 -100, 30)
			item2.position = cc.Point(s.width/2, 30)
			item3.position = cc.Point(s.width/2 + 100, 30)
			
			pMenu.addChild(item1,  kItemTagBasic)
			pMenu.addChild(item2,  kItemTagBasic)
			pMenu.addChild(item3,  kItemTagBasic)
		}
		addChild(pMenu)
	}
	
	
	function title()
	{
		return "No title";
	}
	
	function subtitle()
	{
		return "";
	}
	
	function toMainLayer(pSender)
	{
		var pScene = PerformanceTestScene()
		pScene.runThisTest()
	}
	
	function restartCallback(evt: cc.MenuItemEvent)
	{
		showCurrentTest()
	}
	
	function nextCallback(evt: cc.MenuItemEvent)
	{
		m_nCurCase++
		m_nCurCase=m_nCurCase%m_nMaxCases
		
		showCurrentTest()
	}
	
	function backCallback(evt: cc.MenuItemEvent)
	{
		m_nCurCase--
		if( m_nCurCase < 0 )
			m_nCurCase += m_nMaxCases

		showCurrentTest()
	}
}