var pack = script.locator

var TEST_COUNT = 2

var _touchCurCase = 0

////////////////////////////////////////////////////////////////////////////////

var function runTouchesTest()
{
	_touchCurCase = 0;
    var pScene = cc.Scene();
    var pLayer = TouchesPerformTest1(true, TEST_COUNT, _touchCurCase);

    pScene.addChild(pLayer);

	cocos.director.replaceScene(pScene);
}

////////////////////////////////////////////////////////////////////////////////

class TouchesMainScene : PerformBasicLayer
{
	var _label = null
	var _numTouchesB = null
	var _numTouchesM = null
    var _numTouchesE = null
    var _numTouchesC = null
    var _elapsedTime = null

	constructor(bControlMenuVisible, nMaxCases, nCurCase)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
	}
	
	function showCurrentTest()
	{
		var pLayer = null
		switch (m_nCurCase)
		{
			case 0: pLayer = TouchesPerformTest1(true, TEST_COUNT, m_nCurCase); break
			case 1: pLayer = TouchesPerformTest2(true, TEST_COUNT, m_nCurCase); break
		}
		_touchCurCase = m_nCurCase
		
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
		
		_label = cc.LabelBMFont("00.0", pack.locate("arial16.fnt"))
		_label.position = cc.Point(s.width/2, s.height/2)
		addChild(_label)
		
		cocos.director.scheduler.repeat(this, update, 0.01)
		
		_elapsedTime = 0
		_numTouchesB = _numTouchesM = _numTouchesE = _numTouchesC = 0
	}
	
	function update(evt)
	{
		_elapsedTime += evt.delta
		
		if (_elapsedTime > 1.0)
		{
			var frameRateB = _numTouchesB / _elapsedTime;
			var frameRateM = _numTouchesM / _elapsedTime;
			var frameRateE = _numTouchesE / _elapsedTime;
			var frameRateC = _numTouchesC / _elapsedTime;
			_elapsedTime = 0;
			_numTouchesB = _numTouchesM = _numTouchesE = _numTouchesC = 0;

			_label.string = format("%.3f - %.3f - %.3f - %.3f", frameRateB, frameRateM, frameRateE, frameRateC)
		}
	}
	
	function title()
	{
		return "No title"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TouchesPerformTest1 : TouchesMainScene
{
	constructor(bControlMenuVisible, nMaxCases = 0, nCurCase = 0)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
		
		channel().bind(EVT.CC_TOUCH_BEGIN, this, ccTouchesBegin)
		channel().bind(EVT.CC_TOUCH_MOVED, this, ccTouchesMoved)
		channel().bind(EVT.CC_TOUCH_ENDED, this, ccTouchesEnded)
		channel().bind(EVT.CC_TOUCH_CANCELLED, this, ccTouchesCanceled)
	}
	
	function onEnter()
	{
		base.onEnter()
		touchEnabled = true
	}
	
	function ccTouchesBegin(evt: cc.TouchEvent)
	{
		_numTouchesB++;
		return true;
	}
	
	function ccTouchesMoved(evt: cc.TouchEvent)
	{
		_numTouchesM++;
	}
	
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		_numTouchesE++;
	}
	
	function ccTouchesCanceled(evt: cc.TouchEvent)
	{
		_numTouchesC++;
	}
	
	function title()
	{
		return "Targeted touches"
	}
}

////////////////////////////////////////////////////////////////////////////////

class TouchesPerformTest2 : TouchesMainScene
{
	constructor(bControlMenuVisible, nMaxCases = 0, nCurCase = 0)
	{
		base.constructor(bControlMenuVisible, nMaxCases, nCurCase)
		
		channel().bind(EVT.CC_TOUCH_BEGIN, this, ccTouchesBegin)
		channel().bind(EVT.CC_TOUCH_MOVE, this, ccTouchesMoved)
		channel().bind(EVT.CC_TOUCH_ENDED, this, ccTouchesEnded)
		channel().bind(EVT.CC_TOUCH_CANCELLED, this, ccTouchesCanceled)
	}
	
	function onEnter()
	{
		base.onEnter()
		touchEnabled = true
	}
	
	function ccTouchesBegin(evt: cc.TouchEvent)
	{
		var count = evt.touches.len()	
		_numTouchesB += count
		return true;
	}
	function ccTouchesMoved(evt: cc.TouchEvent)
	{
		var count = evt.touches.len()	
		_numTouchesM += count
	}
	function ccTouchesEnded(evt: cc.TouchEvent)
	{
		var count = evt.touches.len()
		_numTouchesE += count
	}
	function ccTouchesCanceled(evt: cc.TouchEvent)
	{
		var count = evt.touches.len()
		_numTouchesC += count
	}
	
	function title()
	{
		return  "Standard touches";
	}
}