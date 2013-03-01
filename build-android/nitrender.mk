SRC_PATH := $(call my-dir)/../src

LOCAL_PATH := $(SRC_PATH)

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

LOCAL_MODULE    := nitrender

### nitrender

LOCAL_SRC_FILES := \
	nitrender/GLESRenderDevice.cpp \
	nitrender/GLESRenderSpec.cpp \
	nitrender/GLESRenderView_android.cpp \
	nitrender/GLESTexture.cpp \
	nitrender/nitrender.cpp \
	nitrender/RenderContext.cpp \
	nitrender/RenderDevice.cpp \
	nitrender/RenderHandle.cpp \
	nitrender/RenderService.cpp \
	nitrender/RenderSpec.cpp \
	nitrender/RenderTarget.cpp \
	nitrender/RenderView.cpp \
	nitrender/NitLibRender.cpp \

### compile options

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES

LOCAL_C_INCLUDES += \
	$(SRC_PATH) \
	$(SRC_PATH)/../ext/freetype \

### export options

LOCAL_EXPORT_LDLIBS += \
	-lGLESv1_CM \
	-lGLESv2 \
	-lEGL \

### dependancy

LOCAL_STATIC_LIBRARIES += \
	nit \

include $(BUILD_STATIC_LIBRARY)