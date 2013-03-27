var pack = script.locator

////////////////////////////////////////////////////////////////////////////////

var FONT_NAME = "Arial"
var FONT_SIZE = 36

var testIdx = -1

////////////////////////////////////////////////////////////////////////////////

class KeypadTest : cc.ScriptLayer
{
	_label = null

	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
	
		var label = cc.LabelTTF("Keypad Test", "Arial", 28)
		label.position = cc.Point(s.width/2, s.height-50)
		addChild(label, 0)
		
		keypadEnabled = true
		
		// create a label to display the tip string
		_label = cc.LabelTTF("Please press any key...", "Arial", 22)
		_label.position = cc.Point(s.width/2, s.height/2)
		addChild(_label, 0)	
	}
	
	function keyBackClicked()
	{
		_label.String = "Back clicked"
	}
	
	function keyMenuClicked()
	{
		_label.String = "Menu clicked!"
	}
}

////////////////////////////////////////////////////////////////////////////////

class KeypadTestScene : TestScene
{
	constructor()
	{
		base.constructor()
	}
	
	function runThisTest()
	{
		var pLayer = KeypadTest()
		addChild(pLayer)
		cocos.director.replaceScene(this)
	}
}

return KeypadTestScene()