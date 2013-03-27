var pack = script.locator


kTextFieldTTFDefaultTest := 0
kTextFieldTTFActionTest	 := 1
kTextInputTestsCount 	 := 2

FONT_NAME := "Thonburi"
FONT_SIZE := 36

testIdx := -1
//////////////////////////////////////////////////////////////////////////
// implement KeypadTestScene
//////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////
class KeypadTest : cc.ScriptLayer
{
	m_pLabel = null
	constructor()
	{
		base.constructor()
		
		var s = cocos.director.winSize
	
		var label = cc.LabelTTF( "Keypad Test", "Arial", 28)
		label.position = cc.Point(s.width/2, s.height-50)
		addChild(label, 0)
		
		this.keypadEnabled = true
		
		//create a labe to display the tip string
		m_pLabel = cc.LabelTTF("Please press any key...", "Arial", 22)
		m_pLabel.position = cc.Point(s.width/2, s.height/2)
		addChild(m_pLabel, 0)	
	}
	
	function keyBackClicked()
	{
		m_pLabel.String = "Back clicked"
	}
	
	function keyMenuClicked()
	{
		m_pLabel.String = "Menu clicked!"
	}
}




return KeypadTestScene()