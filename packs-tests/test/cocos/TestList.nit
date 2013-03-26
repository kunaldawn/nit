require "testscene"

var function locate(file, pack=null)
{
	return pack ? script.locator.locate(file, pack) : script.locator.locate(file)
}

s_pPathGrossini       := locate("grossini.png", "*Images*")
s_pPathSister1        := locate("grossinis_sister1.png")
s_pPathSister2        := locate("grossinis_sister2.png")
s_pPathB1             := locate("b1.png")
s_pPathB2             := locate("b2.png")
s_pPathR1             := locate("r1.png")
s_pPathR2             := locate("r2.png")
s_pPathF1             := locate("f1.png")
s_pPathF2             := locate("f2.png")
s_pPathBlock          := locate("blocks.png")
s_back                := locate("background.png")
s_back1               := locate("background1.png")
s_back2               := locate("background2.png")
s_back3               := locate("background3.png")
s_stars1              := locate("stars.png")
s_stars2              := locate("stars2.png")
s_fire                := locate("fire.png")
s_snow                := locate("snow.png")
s_streak              := locate("streak.png")
s_PlayNormal          := locate("btn-play-normal.png")
s_PlaySelect          := locate("btn-play-selected.png")
s_AboutNormal         := locate("btn-about-normal.png")
s_AboutSelect         := locate("btn-about-selected.png")
s_HighNormal          := locate("btn-highscores-normal.png")
s_HighSelect          := locate("btn-highscores-selected.png")
s_Ball                := locate("ball.png")
s_Paddle              := locate("paddle.png")
s_pPathClose          := locate("close.png")
s_MenuItem            := locate("menuitemsprite.png")
s_SendScore           := locate("SendScoreButton.png")
s_PressSendScore      := locate("SendScoreButtonPressed.png")
s_Power               := locate("powered.png")
s_AtlasTest           := locate("atlastest.png")

addTest("ActionsTest",  "ActionsTest", "ActionTestScene")
addTest("TransitionsTest", "TransitionsTest", "TransitionsTestScene")
addTest("ProgressActionsTest", "ProgressActionsTest", "ProgressActionsTestScene")
addTest("EffectsTest", "EffectsTest", "EffectTestScene")
addTest("ClickAndMoveTest", "ClickAndMoveTest", "CocosClickAndMoveTestScene")
addTest("RotateWorldTest", "RotateWorldTest", "CocosRotateWorldTestScene")
addTest("ParticleTest", "ParticleTest", "CocosParticleTestScene")
addTest("EaseActionsTest", "EaseActionsTest", "CocosEaseActionTestScene")
addTest("MotionStreakTest", "MotionStreakTest", "MotionStreakTestScene")
addTest("CocosNodeTest", "CocosNodeTest", "CocosNodeTestScene")
addTest("TouchesTest", "TouchesTest", "PongScene")
addTest("menutest", "MenuTest", "MenuTestScene")
addTest("LayerTest", "LayerTest", "LayerTestScene")
addTest("SceneTest", "SceneTest", "CocosSceneTestScene")
addTest("IntervalTest", "IntervalTest", "IntervalTestScene")
addTest("LabelTest", "LabelTest", "AtlasTestScene")
//addTest("ParallaxTest", "ParallaxTest", "CocosParallaxTestScene")
addTest(null, "TextInputTest", null)
addTest("SpriteTest", "SpriteTest", "SpriteTestScene")
addTest("RenderTextureTest", "RenderTextureTest", "RenderTextureScene")
addTest("texture2dtest", "Texture2DTest", "TextureTestScene")
addTest("EffectAdvancedTest", "EffectAdvancedTest", "EffectAdvanceScene")
addTest(null, "Accelerometer", null)						// TODO : pass
addTest("KeyPadTest", "KeypadTest", "KeypadTestScene")
addTest("PerformanceTest", "PerformanceTest", "PerformanceTestScene")
addTest("ZwoptexTest", "ZwoptexTest", "ZwoptexTestScene")
addTest("DirectorTest", "DirectorTest", "DirectorTestScene")
addTest("fonttest", "FontTest", "FontTestScene")