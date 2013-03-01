SRC_PATH := $(call my-dir)/../src

LOCAL_PATH := $(SRC_PATH)

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

LOCAL_MODULE    := nit2d

### nit2d

LOCAL_SRC_FILES := \
	nit2d/CocosService.cpp \
	nit2d/nit2d.cpp \
	nit2d/NitLibNit2d.cpp \
	nit2d/NitLibCocos.cpp \
	nit2d/NitLibCocosAction.cpp \
	nit2d/NitLibCocosEvent.cpp \
	nit2d/NitLibCocosParticle.cpp \
	nit2d/NitLibCocosTransition.cpp \

### cocos2dx

LOCAL_SRC_FILES += \
	../ext/cocos2dx/CCAction.cpp \
	../ext/cocos2dx/CCActionCamera.cpp \
	../ext/cocos2dx/CCActionEase.cpp \
	../ext/cocos2dx/CCActionGrid.cpp \
	../ext/cocos2dx/CCActionGrid3D.cpp \
	../ext/cocos2dx/CCActionInstant.cpp \
	../ext/cocos2dx/CCActionInterval.cpp \
	../ext/cocos2dx/CCActionManager.cpp \
	../ext/cocos2dx/CCActionPageTurn3D.cpp \
	../ext/cocos2dx/CCActionProgressTimer.cpp \
	../ext/cocos2dx/CCActionTiledGrid.cpp \
	../ext/cocos2dx/CCAffineTransform.cpp \
	../ext/cocos2dx/CCAnimation.cpp \
	../ext/cocos2dx/CCAnimationCache.cpp \
	../ext/cocos2dx/CCAtlasNode.cpp \
	../ext/cocos2dx/CCAutoreleasePool.cpp \
	../ext/cocos2dx/CCCamera.cpp \
	../ext/cocos2dx/CCConfiguration.cpp \
	../ext/cocos2dx/CCData.cpp \
	../ext/cocos2dx/CCDirector.cpp \
	../ext/cocos2dx/CCFont.cpp \
	../ext/cocos2dx/CCGeometry.cpp \
	../ext/cocos2dx/CCGrabber.cpp \
	../ext/cocos2dx/CCGrid.cpp \
	../ext/cocos2dx/CCIMEDispatcher.cpp \
	../ext/cocos2dx/CCKeypadDelegate.cpp \
	../ext/cocos2dx/CCKeypadDispatcher.cpp \
	../ext/cocos2dx/CCLabelAtlas.cpp \
	../ext/cocos2dx/CCLabelBMFont.cpp \
	../ext/cocos2dx/CCLabelTTF.cpp \
	../ext/cocos2dx/CCLayer.cpp \
	../ext/cocos2dx/CCMenu.cpp \
	../ext/cocos2dx/CCMenuItem.cpp \
	../ext/cocos2dx/CCMotionStreak.cpp \
	../ext/cocos2dx/CCNode.cpp \
	../ext/cocos2dx/CCNS.cpp \
	../ext/cocos2dx/CCObject.cpp \
	../ext/cocos2dx/CCParallaxNode.cpp \
	../ext/cocos2dx/CCParticleExamples.cpp \
	../ext/cocos2dx/CCParticleSystem.cpp \
	../ext/cocos2dx/CCParticleSystemPoint.cpp \
	../ext/cocos2dx/CCParticleSystemQuad.cpp \
	../ext/cocos2dx/CCProgressTimer.cpp \
	../ext/cocos2dx/CCRenderTexture.cpp \
	../ext/cocos2dx/CCRibbon.cpp \
	../ext/cocos2dx/CCScene.cpp \
	../ext/cocos2dx/CCSet.cpp \
	../ext/cocos2dx/CCSprite.cpp \
	../ext/cocos2dx/CCSpriteBatchNode.cpp \
	../ext/cocos2dx/CCSpriteFrame.cpp \
	../ext/cocos2dx/CCSpriteFrameCache.cpp \
	../ext/cocos2dx/CCTextFieldTTF.cpp \
	../ext/cocos2dx/CCTexture2D.cpp \
	../ext/cocos2dx/CCTextureAtlas.cpp \
	../ext/cocos2dx/CCTextureCache.cpp \
	../ext/cocos2dx/CCTouchDispatcher.cpp \
	../ext/cocos2dx/CCTouchHandler.cpp \
	../ext/cocos2dx/CCTransition.cpp \
	../ext/cocos2dx/CCTransitionPageTurn.cpp \
	../ext/cocos2dx/CCTransitionRadial.cpp \
	../ext/cocos2dx/CCZone.cpp \
	../ext/cocos2dx/cocos2d.cpp \
	../ext/cocos2dx/support/base64.cpp \
	../ext/cocos2dx/support/CCArray.cpp \
	../ext/cocos2dx/support/CCPointExtension.cpp \
	../ext/cocos2dx/support/TransformUtils.cpp \

### compile options

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES

LOCAL_C_INCLUDES += \
	$(SRC_PATH) \
	$(SRC_PATH)/nit2d \
	$(SRC_PATH)/../ext/cocos2dx \
	$(SRC_PATH)/../ext/freetype \

### export options

LOCAL_EXPORT_LDLIBS += \
	-lGLESv1_CM \
	-lGLESv2 \
	-lEGL \

### dependancy

LOCAL_STATIC_LIBRARIES += \
	nitrender \
	nit \
	freetype \

include $(BUILD_STATIC_LIBRARY)