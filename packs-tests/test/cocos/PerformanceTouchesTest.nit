var pack = script.locator

TEST_COUNT := 2
s_nTouchCurCase := 0
////////////////////////////////////////////////////////
//
// TouchesMainScene
//
////////////////////////////////////////////////////////

function runTouchesTest()
{
	s_nTouchCurCase = 0;
    var pScene = cc.Scene();
    var pLayer = TouchesPerformTest1(true, TEST_COUNT, s_nTouchCurCase);

    pScene.addChild(pLayer);

	cocos.director.replaceScene(pScene);
}

class TouchesMainScene : PerformBasicLayer
{
	m_plabel = null
	numberOfTouchesB = null
	numberOfTouchesM = null
    numberOfTouchesE = null
    numberOfTouchesC = null
    elapsedTime = null

	constructor(bControlMenuVisible, nMaxCases, nCurCase)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
	}
	
	function showCurrentTest()
	{
		var pLayer = null
		switch(m_nCurCase)
		{
			 case 0:
				pLayer = TouchesPerformTest1(true, TEST_COUNT, m_nCurCase);
			break;
			case 1:
				pLayer = TouchesPerformTest2(true, TEST_COUNT, m_nCurCase);
			break;
		}
		s_nTouchCurCase = m_nCurCase;
		
		if (pLayer)
		{
			var pScene = cc.Scene()
			pScene.addChild(pLayer)
			
			cocos.director.replaceScene(pScene)
		}
	}
	
	function onEnter()
	{
		base.onEnter()
		
		var s = cocos.director.winSize
		//Add title
		var label = cc.LabelTTF(title(), "Arial", 32)
		addChild(label, 1)
		label.position = cc.Point(s.width/2, s.height - 50)
		
		m_plabel = cc.LabelBMFont("00.0", pack.locate("arial16.fnt"))
		m_plabel.position = cc.Point(s.width/2, s.height/2)
		addChild(m_plabel)
		
		session.scheduler.repeat(this, update, 0.01)
		
		elapsedTime = 0
		numberOfTouchesB = numberOfTouchesM = numberOfTouchesE = numberOfTouchesC = 0
	}
	
	function update(evt)
	{
		elapsedTime += evt.delta
		
		if (elapsedTime > 1.0)
		{
			var frameRateB = numberOfTouchesB / elapsedTime;
			var frameRateM = numberOfTouchesM / elapsedTime;
			var frameRateE = numberOfTouchesE / elapsedTime;
			var frameRateC = numberOfTouchesC / elapsedTime;
			elapsedTime = 0;
			numberOfTouchesB = numberOfTouchesM = numberOfTouchesE = numberOfTouchesC = 0;

			m_plabel.string = format("%.3f - %.3f - %.3f - %.3f", frameRateB, frameRateM, frameRateE, frameRateC)
		}
	}
	
	function title()
	{
		return "No title"
	}
}
////////////////////////////////////////////////////////
//
// TouchesPerformTest1
//
////////////////////////////////////////////////////////
class TouchesPerformTest1 : TouchesMainScene
{
	constructor(bControlMenuVisible, nMaxCases = 0, nCurCase = 0)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
		
		this.channel().priority(-10000).bind(Events.OnCCTouchBegin, this, ccTouchesBegin)
		this.channel().priority(-10000).bind(Events.OnCCTouchMoved, this, ccTouchesMoved)
		this.channel().priority(-10000).bind(Events.OnCCTouchEnded, this, ccTouchesEnded)
		this.channel().priority(-10000).bind(Events.OnCCTouchCancelled, this, ccTouchesCanceled)
	}
	
	function onEnter()
	{
		base.onEnter()
		this.touchEnabled = true
	}
	
	function ccTouchesBegin(evt: cc.TouchEvent)
	{
		numberOfTouchesB++;
		return true;
	}
	function ccTouchesMoved(evt: cc.TouchEvent)
	{
		numberOfTouchesM++;
	}
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		numberOfTouchesE++;
	}
	function ccTouchesCanceled(evt: cc.TouchEvent)
	{
		numberOfTouchesC++;
	}
	
	function title()
	{
		return "Targeted touches"
	}
}

////////////////////////////////////////////////////////
//
// TouchesPerformTest2
//
////////////////////////////////////////////////////////
class TouchesPerformTest2 : TouchesMainScene
{
	constructor(bControlMenuVisible, nMaxCases = 0, nCurCase = 0)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
		
		this.channel().priority(-10000).bind(Events.OnCCTouchBegin, this, ccTouchesBegin)
		this.channel().priority(-10000).bind(Events.OnCCTouchMoved, this, ccTouchesMoved)
		this.channel().priority(-10000).bind(Events.OnCCTouchEnded, this, ccTouchesEnded)
		this.channel().priority(-10000).bind(Events.OnCCTouchCancelled, this, ccTouchesCanceled)
	}
	
	function onEnter()
	{
		base.onEnter()
		this.touchEnabled = true
	}
	
	function ccTouchesBegin(evt: cc.TouchEvent)
	{
		var count = evt.touches.len()	
		numberOfTouchesB += count
		return true;
	}
	function ccTouchesMoved(evt: cc.TouchEvent)
	{
		var count = evt.touches.len()	
		numberOfTouchesM += count
	}
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		var count = evt.touches.len()
		numberOfTouchesE += count
	}
	function ccTouchesCanceled(evt: cc.TouchEvent)
	{
		var count = evt.touches.len()
		numberOfTouchesC += count
	}
	
	function title()
	{
		return  "Standard touches";
	}

}