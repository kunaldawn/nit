SRC_PATH := $(call my-dir)/../src

LOCAL_PATH := $(SRC_PATH)

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

LOCAL_MODULE    := nitnet

### nitnet

LOCAL_SRC_FILES := \
	nitnet/nitnet.cpp \
	nitnet/HttpRequest.cpp \
	nitnet/NetService.cpp \
	nitnet/NitLibNet.cpp \
	nitnet/URLRequest.cpp \

### compile options

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += \
	$(SRC_PATH) \
	$(SRC_PATH)/../ext/libcurl/include \

### export options

### dependancy

LOCAL_STATIC_LIBRARIES += \
	curl \
	nit \

include $(BUILD_STATIC_LIBRARY)