SRC_PATH := $(call my-dir)/../ext

LOCAL_PATH := $(SRC_PATH)

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

LOCAL_MODULE := freetype

### freetype ###########################

LOCAL_SRC_FILES += \
	freetype/autofit/autofit.c \
	freetype/bdf/bdf.c \
	freetype/cff/cff.c \
	freetype/base/ftbase.c \
	freetype/base/ftbitmap.c \
	freetype/cache/ftcache.c \
	freetype/base/ftdebug.c \
	freetype/base/ftfstype.c \
	freetype/base/ftgasp.c \
	freetype/base/ftglyph.c \
	freetype/gzip/ftgzip.c \
	freetype/base/ftinit.c \
	freetype/lzw/ftlzw.c \
	freetype/base/ftstroke.c \
	freetype/base/ftsystem.c \
	freetype/smooth/smooth.c \

### freetype modules

LOCAL_SRC_FILES += \
	freetype/base/ftbbox.c \
	freetype/base/ftgxval.c \
	freetype/base/ftlcdfil.c \
	freetype/base/ftmm.c \
	freetype/base/ftotval.c \
	freetype/base/ftpatent.c \
	freetype/base/ftpfr.c \
	freetype/base/ftsynth.c \
	freetype/base/fttype1.c \
	freetype/base/ftwinfnt.c \
	freetype/base/ftxf86.c \
	freetype/pcf/pcf.c \
	freetype/pfr/pfr.c \
	freetype/psaux/psaux.c \
	freetype/pshinter/pshinter.c \
	freetype/psnames/psmodule.c \
	freetype/raster/raster.c \
	freetype/sfnt/sfnt.c \
	freetype/truetype/truetype.c \
	freetype/type1/type1.c \
	freetype/cid/type1cid.c \
	freetype/type42/type42.c \
	freetype/winfonts/winfnt.c \

### compile options

LOCAL_CFLAGS := -DFT2_BUILD_LIBRARY

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += \
	$(SRC_PATH)/freetype \

### export options

include $(BUILD_STATIC_LIBRARY)
