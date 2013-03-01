MY_PATH := $(call my-dir)

include $(MY_PATH)/../../nit.mk
include $(MY_PATH)/../../freetype.mk
include $(MY_PATH)/../../imagecodec.mk
include $(MY_PATH)/../../curl.mk
include $(MY_PATH)/../../nitnet.mk
include $(MY_PATH)/../../nitrender.mk
include $(MY_PATH)/../../nit2d.mk
include $(MY_PATH)/../../nitmyplugin.mk

include $(CLEAR_VARS)

LOCAL_PATH := $(MY_PATH)

LOCAL_ARM_MODE := arm

LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

LOCAL_MODULE := nitandroid

LOCAL_SRC_FILES := \
	NitAndroid.cpp \

# It seems that,
# when module 'A' uses module 'B', 'A' should appear prior to 'B' on libraries list

LOCAL_STATIC_LIBRARIES += \
	nitmyplugin \
	nit2d \
	nitrender \
	nitnet \
	curl \
	freetype \
	nit \
	imagecodec \
	cpufeatures \

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/cpufeatures)