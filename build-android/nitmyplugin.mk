SRC_PATH := $(call my-dir)/../src

LOCAL_PATH := $(SRC_PATH)

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

LOCAL_MODULE    := nitmyplugin

## source

LOCAL_SRC_FILES += \
	nitmyplugin/nitmyplugin.cpp \

### compile options

LOCAL_CFLAGS := -DHAVE_EXPAT_CONFIG_H

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += \
	$(SRC_PATH) \

### export options

LOCAL_EXPORT_C_INCLUDES += \
	$(SRC_PATH) \

### dependancy

LOCAL_STATIC_LIBRARIES += \
	nit \

include $(BUILD_STATIC_LIBRARY)

