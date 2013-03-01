/// nit - Noriter Framework
/// A Cross-platform Open Source Integration for Game-oriented Apps
///
/// http://www.github.com/ellongrey/nit
///
/// Copyright (c) 2013 by Jun-hyeok Jang
/// 
/// (see each file to see the different copyright owners)
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// Author: ellongrey

#pragma once

#include "nit/nit.h"

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/native_activity.h>
#include <android/input.h>
#include <android/sensor.h>

#include "nit/platform/android/android_native_app_glue.h"
#include "nit/io/Stream.h"
#include "nit/io/Archive.h"
#include "nit/io/FileLocator.h"
#include "nit/event/Event.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define g_AndroidJNI					(nit::NitRuntime::getSingleton()->getJNI())

class NIT_API AndroidJNI
{
public:
	AndroidJNI();

public:
	struct MethodInfo
	{
		JNIEnv*							env;
		jclass							javaClass;
		jmethodID						methodID;
	};

	void								init(ANativeActivity* nativeActivity);

	jint								onLoadJNI(JavaVM* vm, void* reserved);
	void								onUnloadJNI(JavaVM* vm, void* reserved);

	JavaVM*								getJavaVM()								{ return _javaVm; }

	JNIEnv*								getEnv(JNIEnv* env = NULL);
	jclass								getJavaClass(const char* classname, JNIEnv* env = NULL);

	bool								getStaticMethodInfo(MethodInfo& outInfo, jclass javaClass, const char* methodname, const char* signature, JNIEnv* env = NULL);
	bool								getStaticMethodInfo(MethodInfo& outInfo, jobject instance, const char* methodname, const char* signature, JNIEnv* env = NULL);
	bool								getStaticMethodInfo(MethodInfo& outInfo, const char* classname, const char* methodname, const char* signature, JNIEnv* env = NULL);

	bool								getMethodInfo(MethodInfo& outInfo, jclass javaClass, const char* methodname, const char* signature, JNIEnv* env = NULL);
	bool								getMethodInfo(MethodInfo& outInfo, jobject instance, const char* methodname, const char* signature, JNIEnv* env = NULL);
	bool								getMethodInfo(MethodInfo& outInfo, const char* classname, const char* methodname, const char* signature, JNIEnv* env = NULL);

	String								toString(jstring jstr, JNIEnv* env = NULL);

	ANativeActivity*					getActivity()							{ return _nativeActivity; }
	jobject								getActivityInstance()					{ return _nativeActivity ? _nativeActivity->clazz : NULL; }

private:
	JavaVM*								_javaVm;
	ANativeActivity*					_nativeActivity;
	android_app*						_androidApp;

	jobject								_classLoader;
	jmethodID							_findClassMethod;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API LogCatLogger : public Logger
{
public:
	virtual void						doLog(const LogEntry* entry);

private:
	std::string							_line;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API TextViewLogger : public Logger
{
public:
	TextViewLogger();
	virtual ~TextViewLogger();

public:
	void								setPeer(JNIEnv* env, jobject textView);

protected:
	virtual void						doLog(const LogEntry* entry);
	void								write(const char* cstr);

private:
	jweak								_peer;
	jmethodID							_appendMethod;

	std::string							_line;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API AssetArchive : public Archive
{
public:
	AssetArchive(AAssetManager* assetManager);
	AssetArchive(JNIEnv* env, jobject javaAssetManager);
	AssetArchive(const String& name, AssetArchive* parent, const String& dir);

public:
	const String&						getBaseURL()							{ return this ? _dir : StringUtil::BLANK(); }
	virtual String						makeUrl(const String& sourceName);

	bool								getFilenameCaseFix()					{ return _fixCaseProblem; }
	void								setFilenameCaseFix(bool flag)			{ _fixCaseProblem = flag; }

public:									// Archive Impl
	virtual bool						isCaseSensitive()						{ return true; }
	virtual StreamSource*				locateLocal(const String& streamName);
	virtual void						findLocal(const String& pattern, StreamSourceMap& varResults);
	virtual void						load();
	virtual void						unload();
	virtual bool						isReadOnly()							{ return true; }

	virtual StreamReader*				open(const String& streamName);
	virtual StreamWriter*				create(const String& streamName)		{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual StreamWriter*				modify(const String& streamName)		{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						remove(const String& pattern)			{ NIT_THROW(EX_NOT_SUPPORTED); }
	virtual void						rename(const String& streamName, const String& newName) { NIT_THROW(EX_NOT_SUPPORTED); }

public:
	class Asset;
	class AssetReader;
	class AssetFileReader;

	AAsset*								openAsset(String& varStreamName);
	bool								fixCase(String& varStreamName);

private:
	jobject								_assetManagerRef;
	AAssetManager*						_assetManager;
	String								_dir;
	bool								_fixCaseProblem;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API AssetArchive::Asset : public File
{
public:
	Asset(AssetArchive* arc, const String& name);

public:									// StreamSource Impl
	virtual size_t						getStreamSize()							{ return getMemorySize(); }
	virtual size_t						getMemorySize();
	virtual Timestamp					getTimestamp()							{ return Timestamp::ZERO(); /* not supported */ }
	virtual StreamReader*				open();
	virtual StreamReader*				openRange(size_t offset, size_t size, StreamSource* source);

private:
	size_t								_cachedSize;

	friend class AssetArchive;
	StreamReader*						openWith(AAsset* peer);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API AssetArchive::AssetReader : public StreamReader
{
public:
	AssetReader(Asset* asset, AAsset* peer);

public:
	bool								isAllocated()							{ return AAsset_isAllocated(_peer) != 0; }

public:									// StreamReaderImpl
	virtual StreamSource*				getSource()								{ return _asset; }
	virtual bool						isBuffered()							{ return false; }
	virtual bool						isSized()								{ return true; }
	virtual bool						isSeekable()							{ return true; }
	virtual bool						isEof()									{ return AAsset_getRemainingLength(_peer) == 0; }
	virtual size_t						getSize()								{ return AAsset_getLength(_peer); }
	virtual void						skip(int count)							{ AAsset_seek(_peer, count, SEEK_CUR); }
	virtual void						seek(size_t pos)						{ AAsset_seek(_peer, pos, SEEK_SET); }
	virtual size_t						tell()									{ return AAsset_seek(_peer, 0, SEEK_CUR); }
	virtual size_t						readRaw(void* buf, size_t size);

protected:
	virtual void						onDelete();

private:
	Ref<Asset>							_asset;
	AAsset*								_peer;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API AssetArchive::AssetFileReader : public FileReader
{
public:
	AssetFileReader(StreamSource* source, AAsset* assetPeer, NIT_FILE_HANDLE fileHandle, size_t offset, size_t size);

protected:
	AAsset*								_assetPeer;

	virtual void						onDelete();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API AndroidAppCmdEvent : public Event
{
public:
	AndroidAppCmdEvent() : _app(NULL), _cmd(0) { }
	AndroidAppCmdEvent(android_app* app, int32 cmd) : _app(app), _cmd(cmd) { }

	android_app*						getApp() const							{ return _app; }
	int32								getCmd() const							{ return _cmd; }

private:
	// see APP_CMD definition of android_native_app_glue.h
	android_app*						_app;
	int32								_cmd;
};

////////////////////////////////////////////////////////////////////////////////

// see <android/input.h>
// NOTE: If you consume this input event, android will get it as handled otherwise not handled.

class NIT_API AndroidInputEvent : public Event
{
public:
	AndroidInputEvent() : _app(NULL), _input(NULL) { }
	AndroidInputEvent(android_app* app, AInputEvent* input) : _app(app), _input(input) { }

	android_app*						getApp() const							{ return _app; }
	AInputEvent*						getInput() const						{ return _input; }

private:
	android_app*						_app;
	AInputEvent*						_input;
};

////////////////////////////////////////////////////////////////////////////////

// see <android/sensor.h>

class NIT_API AndroidSensorEvent : public Event
{
public:
	AndroidSensorEvent() : _sensor(NULL) { }
	AndroidSensorEvent(ASensorEvent* sensor) : _sensor(sensor) { }

	ASensorEvent*						getSensor()	const						{ return _sensor; }

private:
	ASensorEvent*						_sensor;
};

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DECLARE(NIT_API, OnAndroidAppCmd, AndroidAppCmdEvent);
NIT_EVENT_DECLARE(NIT_API, OnAndroidInput, AndroidInputEvent);
NIT_EVENT_DECLARE(NIT_API, OnAndroidSensor, AndroidSensorEvent);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
