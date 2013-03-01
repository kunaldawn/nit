SRC_PATH := $(call my-dir)/../src

LOCAL_PATH := $(SRC_PATH)

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

LOCAL_MODULE    := nit

LOCAL_SRC_FILES += \
	nit/nit.cpp \

### runtime
LOCAL_SRC_FILES += \
	nit/runtime/ErrorHandler.cpp \
	nit/runtime/Exception.cpp \
	nit/runtime/LogManager.cpp \
	nit/runtime/MemManager.cpp \
	nit/runtime/NitRuntime.cpp \
	nit/runtime/NitRuntime_android.cpp \
	
### app
LOCAL_SRC_FILES += \
	nit/app/AppBase.cpp \
	nit/app/AppConfig.cpp \
	nit/app/Module.cpp \
	nit/app/Package.cpp \
	nit/app/PackageService.cpp \
	nit/app/PackArchive.cpp \
	nit/app/PackBundle.cpp \
	nit/app/Plugin.cpp \
	nit/app/Plugin_none.cpp \
	nit/app/Service.cpp \
	nit/app/Session.cpp \
	nit/app/SessionService.cpp \
	
### async
LOCAL_SRC_FILES += \
	nit/async/AsyncJob.cpp \
	nit/async/Condition.cpp \
	nit/async/EventSemaphore.cpp \
	nit/async/Mutex.cpp \
	nit/async/RWLock.cpp \
	nit/async/Semaphore.cpp \
	nit/async/Thread.cpp \
	nit/async/ThreadLocal.cpp \

### content
LOCAL_SRC_FILES += \
	nit/content/Content.cpp \
	nit/content/ContentManager.cpp \
	nit/content/ContentsService.cpp \
	nit/content/Image.cpp \
	nit/content/PixelFormat.cpp \
	nit/content/Texture.cpp \
	
### data
LOCAL_SRC_FILES += \
	nit/data/Color.cpp \
	nit/data/Database.cpp \
	nit/data/DataChannel.cpp \
	nit/data/DataLoader.cpp \
	nit/data/DataSaver.cpp \
	nit/data/DataSchema.cpp \
	nit/data/DataValue.cpp \
	nit/data/DateTime.cpp \
	nit/data/NitString.cpp \
	nit/data/ParserUtil.cpp \
	nit/data/RegExp.cpp \
	nit/data/Settings.cpp \
	nit/data/StringUtil.cpp \
	../ext/sqlite3/sqlite3.c \
	../ext/expat/xmlparse.c \
	../ext/expat/xmlrole.c \
	../ext/expat/xmltok.c \
	../ext/expat/xmltok_impl.c \
	../ext/expat/xmltok_ns.c \

### event
LOCAL_SRC_FILES += \
	nit/event/Event.cpp \
	nit/event/EventAutomata.cpp \
	nit/event/Timer.cpp \
	
### input
LOCAL_SRC_FILES += \
	nit/input/InputCommand.cpp \
	nit/input/InputDevice.cpp \
	nit/input/InputService.cpp \
	nit/input/InputSource.cpp \
	nit/input/InputUser.cpp \
	
### io
LOCAL_SRC_FILES += \
	nit/io/Archive.cpp \
	nit/io/ContentTypes.cpp \
	nit/io/FileLocator.cpp \
	nit/io/FileLocator_unix.cpp \
	nit/io/MemoryBuffer.cpp \
	nit/io/Stream.cpp \
	nit/io/ZStream.cpp \
	
### legacy
LOCAL_SRC_FILES += \
	nit/legacy/Legacy_Ogre.cpp \
	nit/legacy/Legacy_Poco.cpp \
	
### logic
LOCAL_SRC_FILES += \
	nit/logic/AutomataComponent.cpp \
	nit/logic/Component.cpp \
	nit/logic/Feature.cpp \
	nit/logic/Object.cpp \
	nit/logic/Transform.cpp \
	nit/logic/World.cpp \
	
### math
LOCAL_SRC_FILES += \
	nit/math/AxisAlignedBox.cpp \
	nit/math/Curves.cpp \
	nit/math/Matrix3.cpp \
	nit/math/Matrix4.cpp \
	nit/math/NitMath.cpp \
	nit/math/Plane.cpp \
	nit/math/Quaternion.cpp \
	nit/math/Solver.cpp \
	nit/math/Vector2.cpp \
	nit/math/Vector3.cpp \
	nit/math/Vector4.cpp \
	
### net
LOCAL_SRC_FILES += \
	nit/net/Remote.cpp \
	nit/net/RemoteDebugger.cpp \
	nit/net/Socket.cpp \

### platform
LOCAL_SRC_FILES += \
	nit/platform/android/android_native_app_glue.c \
	nit/platform/android/AndroidSupport.cpp \
	nit/platform/SystemTimer_android.cpp \

### ref
LOCAL_SRC_FILES += \
	nit/ref/CacheHandle.cpp \
	nit/ref/RefCache.cpp \
	nit/ref/RefCounted.cpp \

### script
LOCAL_SRC_FILES += \
	nit/script/NitBind.cpp \
	nit/script/ScriptDebugger.cpp \
	nit/script/ScriptRuntime.cpp \
	nit/script/NitBindJava.cpp \
	nit/script/NitLibAndroid.cpp \
	nit/script/NitLibApp.cpp \
	nit/script/NitLibCore.cpp \
	nit/script/NitLibCoreExt.cpp \
	nit/script/NitLibData.cpp \
	nit/script/NitLibEvent.cpp \
	nit/script/NitLibJava.cpp \
	nit/script/NitLibMath.cpp \
	nit/script/NitLibTimer.cpp \

### squirrel
LOCAL_SRC_FILES += \
	../ext/squirrel/sqapi.cpp \
	../ext/squirrel/sqbaselib.cpp \
	../ext/squirrel/sqclass.cpp \
	../ext/squirrel/sqcompiler.cpp \
	../ext/squirrel/sqdebug.cpp \
	../ext/squirrel/sqfuncstate.cpp \
	../ext/squirrel/sqgc.cpp \
	../ext/squirrel/sqlexer.cpp \
	../ext/squirrel/sqmem.cpp \
	../ext/squirrel/sqobject.cpp \
	../ext/squirrel/sqstate.cpp \
	../ext/squirrel/sqstdaux.cpp \
	../ext/squirrel/sqstdblob.cpp \
	../ext/squirrel/sqstdio.cpp \
	../ext/squirrel/sqstdmath.cpp \
	../ext/squirrel/sqstdrex.cpp \
	../ext/squirrel/sqstdstream.cpp \
	../ext/squirrel/sqstdstring.cpp \
	../ext/squirrel/sqstdsystem.cpp \
	../ext/squirrel/sqtable.cpp \
	../ext/squirrel/squndump.cpp \
	../ext/squirrel/sqvm.cpp \
	../ext/squirrel/sqxapi.cpp \
	
### util
LOCAL_SRC_FILES += \
	nit/util/Allocator.cpp \

### compile options

LOCAL_CFLAGS := -DHAVE_EXPAT_CONFIG_H
#LOCAL_CPPFLAGS += -std=c++0x

LOCAL_ARM_MODE := arm

#LOCAL_CFLAGS += -O3

LOCAL_C_INCLUDES += \
	$(SRC_PATH) \
	$(SRC_PATH)/nit \
	$(SRC_PATH)/../ext \
	$(SRC_PATH)/../ext/expat \

### export options

LOCAL_EXPORT_C_INCLUDES += \
	$(SRC_PATH) \
	$(SRC_PATH)/../ext \
	$(SRC_PATH)/../ext/expat \

LOCAL_EXPORT_LDLIBS += \
	-llog \
	-lz \
	-landroid \
	
include $(BUILD_STATIC_LIBRARY)

