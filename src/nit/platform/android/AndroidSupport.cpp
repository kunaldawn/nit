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

#include "nit_pch.h"

#include "nit/platform/android/AndroidSupport.h"

#include "nit/runtime/NitRuntime.h"

#include <android/asset_manager_jni.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NIT_EVENT_DEFINE(ANDROID_APP_CMD,		AndroidAppCmdEvent);
NIT_EVENT_DEFINE(ANDROID_INPUT,		AndroidInputEvent);
NIT_EVENT_DEFINE(ANDROID_SENSOR,		AndroidSensorEvent);

////////////////////////////////////////////////////////////////////////////////

void LogCatLogger::doLog(const LogEntry* entry)
{
	int len = entry->messageLen;

	if (_lineStart)
	{
		_line.append(entry->tagStr);
		_line.push_back(' ');
	}

	_line.append(entry->message, len);

	if (entry->lineEnd)
	{
		android_LogPriority prio;

		switch (entry->logLevel)
		{
		case LOG_LEVEL_FATAL:			prio = ANDROID_LOG_FATAL; break;
		case LOG_LEVEL_ERROR:			prio = ANDROID_LOG_ERROR; break;
		case LOG_LEVEL_WARNING:			prio = ANDROID_LOG_WARN; break;
		case LOG_LEVEL_INFO:			prio = ANDROID_LOG_INFO; break;
		case LOG_LEVEL_DEBUG:			prio = ANDROID_LOG_DEBUG; break;
		case LOG_LEVEL_VERBOSE:			prio = ANDROID_LOG_VERBOSE; break;
		default:
			prio = ANDROID_LOG_DEBUG;
		}

		std::string lcTag = "nit/";
		lcTag.append(entry->channel->getName());

		__android_log_write(prio, lcTag.c_str(), _line.c_str());
		_line.resize(0);
	}
}

////////////////////////////////////////////////////////////////////////////////

TextViewLogger::TextViewLogger()
{
	_peer = NULL;
}

TextViewLogger::~TextViewLogger()
{
	if (_peer)
	{
		JNIEnv* env = g_AndroidJNI->getEnv();
		if (env)
			env->DeleteWeakGlobalRef(_peer);
	}
}

void TextViewLogger::doLog(const LogEntry* entry)
{
	if (_lineStart)
	{
		_line.append(entry->channel->getName());
		_line.append("> ");
		_line.append(entry->tagStr);
		_line.push_back(' ');
	}

	_line.append(entry->message, entry->messageLen);

	if (entry->lineEnd)
	{
		_line.push_back('\n');
		write(_line.c_str());
		_line.resize(0);
	}
}

void TextViewLogger::write(const char* cstr)
{
	if (_peer == NULL) return;

	JNIEnv* env = g_AndroidJNI->getEnv();
	if (env == NULL) 
	{
#ifndef NIT_NO_LOG
		__android_log_write(ANDROID_LOG_INFO, "nit", "TextViewLogger: null env");
#endif
		return;
	}

	if (env->IsSameObject(_peer, NULL))
	{
#ifndef NIT_NO_LOG
		__android_log_write(ANDROID_LOG_INFO, "nit", "TextViewLogger: peer released");
#endif
		_peer = NULL;
	}

	jstring jstr = env->NewStringUTF(cstr);
	env->CallVoidMethod(_peer, _appendMethod, jstr);
	env->ExceptionClear();

	env->DeleteLocalRef(jstr);
}

void TextViewLogger::setPeer(JNIEnv* env, jobject textView)
{
	AndroidJNI::MethodInfo m;
	if (g_AndroidJNI->getMethodInfo(m, env->GetObjectClass(textView), "append", "(Ljava/lang/CharSequence;)V", env))
	{
		_appendMethod = m.methodID;
		_peer = env->NewWeakGlobalRef(textView);

		env->DeleteLocalRef(m.javaClass);
	}
}

////////////////////////////////////////////////////////////////////////////////

AndroidJNI::AndroidJNI()
{
	_javaVm = NULL;
}

void AndroidJNI::init(ANativeActivity* nativeActivity)
{
	_javaVm = nativeActivity->vm;
	_nativeActivity = nativeActivity;

	_classLoader = NULL;
	_findClassMethod = NULL;
}

jint AndroidJNI::onLoadJNI(JavaVM* vm, void* reserved)
{
	_javaVm = vm;

	return JNI_VERSION_1_4;
}

void AndroidJNI::onUnloadJNI(JavaVM* vm, void* reserved)
{
	_javaVm = NULL;
}

JNIEnv* AndroidJNI::getEnv(JNIEnv* env)
{
	if (env) return env;

	if (_javaVm == NULL) return NULL;

	if (_javaVm->GetEnv((void**)&env, JNI_VERSION_1_4) == JNI_OK)
		return env;

	if (_javaVm->AttachCurrentThread(&env, 0) >= 0)
		return env;

	LOG(0, "!!! Failed to attach jni env to current thread\n");
	env->ExceptionClear();
	return NULL;
}

jclass AndroidJNI::getJavaClass(const char* classname, JNIEnv* env)
{
	env = getEnv(env);
	if (env == NULL)
		return NULL;

	jclass javaClass = env->FindClass(classname);
	LOG(0, ".. initial try javaClass '%s': %08x\n", classname, javaClass);
	if (javaClass)
		return javaClass;

	// Assume that class loader of the native activity can't find the class yet.
	// see: http://blog.tewdew.com/post/6852907694/using-jni-from-a-native-activity
	// Here implements a workaround:

	// we got exception above, so clear it
	// (Otherwise every JNI call will fail and provokes VM abort)
	env->ExceptionClear();

	if (_nativeActivity)
	{
		if (_classLoader == NULL)
		{
			jobject activity = _nativeActivity->clazz;
			jclass activityClass = env->GetObjectClass(activity);
			jmethodID getClassLoader = env->GetMethodID(activityClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
			LOG(0, ".. activityClass: %08x\n", activityClass);
			LOG(0, ".. getClassLoader: %08x\n", getClassLoader);

			_classLoader = env->NewGlobalRef(env->CallObjectMethod(activity, getClassLoader));

			LOG(0, ".. _classLoaders: %08x\n", _classLoader);

			jclass classLoaderClass = env->GetObjectClass(_classLoader);
			_findClassMethod = env->GetMethodID(classLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
			LOG(0, ".. findClass: %08x\n", _findClassMethod);

			env->DeleteLocalRef(activityClass);
			env->DeleteLocalRef(classLoaderClass);
		}

		jstring strClassName = env->NewStringUTF(classname);
		javaClass = (jclass)env->CallObjectMethod(_classLoader, _findClassMethod, strClassName);

		env->DeleteLocalRef(strClassName);

		LOG(0, ".. javaClass: %08x\n", javaClass);

		if (javaClass)
			return javaClass;
	}

	LOG(0, "*** Failed to find java class '%s'\n", classname);
	env->ExceptionClear();
	return NULL;
}

bool AndroidJNI::getStaticMethodInfo(MethodInfo& outInfo, const char* classname, const char* methodname, const char* signature, JNIEnv* env)
{
	env = getEnv(env);
	if (env == NULL)
		return false;

	jclass javaClass = getJavaClass(classname, env);
	if (javaClass == NULL)
		return false;

	jmethodID methodID = env->GetStaticMethodID(javaClass, methodname, signature);
	if (methodID == NULL)
	{
		LOG(0, "*** Failed to find static java method '%s.%s %s'", classname, methodname, signature);
		env->ExceptionClear();
		return false;
	}

	LOG(0, "-- JNI GetStaticMethodInfo(%s) success: %s %s\n", classname, methodname, signature);

	outInfo.javaClass = javaClass;
	outInfo.env = env;
	outInfo.methodID = methodID;

	return true;
}

bool AndroidJNI::getStaticMethodInfo(MethodInfo& outInfo, jclass javaClass, const char* methodname, const char* signature, JNIEnv* env)
{
	env = getEnv(env);
	if (env == NULL)
		return false;

	jmethodID methodID = env->GetStaticMethodID(javaClass, methodname, signature);
	if (methodID == NULL)
	{
		LOG(0, "*** Failed to find static java method '%s %s'", methodname, signature);
		env->ExceptionClear();
		return false;
	}

	LOG(0, "-- JNI GetStaticMethodInfo(jclass) success: %s %s\n", methodname, signature);

	outInfo.javaClass = javaClass;
	outInfo.env = env;
	outInfo.methodID = methodID;

	return true;
}

bool AndroidJNI::getStaticMethodInfo(MethodInfo& outInfo, jobject instance, const char* methodname, const char* signature, JNIEnv* env /*= NULL*/)
{
	env = getEnv(env);
	if (env == NULL)
		return false;

	jclass javaClass = env->GetObjectClass(instance);

	jmethodID methodID = env->GetStaticMethodID(javaClass, methodname, signature);
	if (methodID == NULL)
	{
		LOG(0, "*** Failed to find static java method '%s %s'", methodname, signature);
		env->ExceptionClear();
		return false;
	}

	LOG(0, "-- JNI GetStaticMethodInfo(jobject) success: %s %s\n", methodname, signature);

	outInfo.javaClass = javaClass;
	outInfo.env = env;
	outInfo.methodID = methodID;

	return true;
}

bool AndroidJNI::getMethodInfo(MethodInfo& outInfo, const char* classname, const char* methodname, const char* signature, JNIEnv* env)
{
	env = getEnv(env);
	if (env == NULL)
		return false;

	jclass javaClass = getJavaClass(classname, env);
	if (javaClass == NULL)
		return false;

	jmethodID methodID = env->GetMethodID(javaClass, methodname, signature);
	if (methodID == NULL)
	{
		LOG(0, "*** Failed to find java method '%s.%s %s'", classname, methodname, signature);
		env->ExceptionClear();
		return false;
	}

	LOG(0, "-- JNI GetMethodInfo(%s) success: %s %s\n", classname, methodname, signature);

	outInfo.javaClass = javaClass;
	outInfo.env = env;
	outInfo.methodID = methodID;

	return true;
}

bool AndroidJNI::getMethodInfo(MethodInfo& outInfo, jclass javaClass, const char* methodname, const char* signature, JNIEnv* env)
{
	env = getEnv(env);
	if (env == NULL)
		return false;

	jmethodID methodID = env->GetMethodID(javaClass, methodname, signature);
	if (methodID == NULL)
	{
		LOG(0, "*** Failed to find java method '%s %s'", methodname, signature);
		env->ExceptionClear();
		return false;
	}

	LOG(0, "-- JNI GetMethodInfo(jclass) success: %s %s\n", methodname, signature);

	outInfo.javaClass = javaClass;
	outInfo.env = env;
	outInfo.methodID = methodID;

	return true;
}

bool AndroidJNI::getMethodInfo(MethodInfo& outInfo, jobject instance, const char* methodname, const char* signature, JNIEnv* env /*= NULL*/)
{
	env = getEnv(env);
	if (env == NULL)
		return false;

	jclass javaClass = env->GetObjectClass(instance);

	jmethodID methodID = env->GetMethodID(javaClass, methodname, signature);
	if (methodID == NULL)
	{
		LOG(0, "*** Failed to find java method '%s %s'", methodname, signature);
		env->ExceptionClear();
		return false;
	}

	LOG(0, "-- JNI GetMethodInfo(jobject) success: %s %s\n", methodname, signature);

	outInfo.javaClass = javaClass;
	outInfo.env = env;
	outInfo.methodID = methodID;

	return true;
}

String AndroidJNI::toString(jstring jstr, JNIEnv* env)
{
	env = getEnv(env);

	if (env == NULL)
		return "";

	jboolean isCopy;

	const char* utfChars = env->GetStringUTFChars(jstr, &isCopy);

	String str(utfChars);

	if (isCopy)
		env->ReleaseStringUTFChars(jstr, utfChars);

	return str;
}

////////////////////////////////////////////////////////////////////////////////

AssetArchive::AssetArchive(AAssetManager* assetManager)
: Archive("$asset_root")
{
	_assetManagerRef = NULL;
	_assetManager = assetManager;
	_fixCaseProblem = false;
}

AssetArchive::AssetArchive(JNIEnv* env, jobject javaAssetManager)
: Archive("$asset_root")
{
	_assetManagerRef = env->NewGlobalRef(javaAssetManager);
	_assetManager = AAssetManager_fromJava(env, _assetManagerRef);
	_fixCaseProblem = false;
}

AssetArchive::AssetArchive(const String& name, AssetArchive* parent, const String& dir)
: Archive(name)
{
	JNIEnv* env = g_AndroidJNI->getEnv();

	if (parent->_assetManagerRef)
	{
		_assetManagerRef = env->NewGlobalRef(parent->_assetManagerRef);
		_assetManager = AAssetManager_fromJava(env, _assetManagerRef);
	}
	else
	{
		_assetManagerRef = NULL;
		_assetManager = parent->_assetManager;
	}

	_fixCaseProblem = parent->_fixCaseProblem;
	_dir = dir;

	if (!_dir.empty())
	{
		FileUtil::normalizeSeparator(_dir);

		// Make sure base to be 'path/' from
		if (_dir[_dir.length()-1] != FileUtil::getPathSeparator())
			_dir += FileUtil::getPathSeparator();

		if (StringUtil::endsWith(_dir, FileUtil::getDamnedPattern1()))
			_dir.erase(_dir.size() - FileUtil::getDamnedPattern1().length());
	}

	load();
}

void AssetArchive::load()
{

}

void AssetArchive::unload()
{
	if (_assetManagerRef)
	{
		JNIEnv* env = g_AndroidJNI->getEnv();
		env->DeleteGlobalRef(_assetManagerRef);
		_assetManagerRef = NULL;
		_assetManager = NULL;
	}
}

String AssetArchive::makeUrl(const String& sourceName)
{
	String url = "asset://";
	url += _dir;
	url += sourceName;

	return url;
}

void AssetArchive::findLocal(const String& pattern, StreamSourceMap& varResults)
{
	if (_assetManager == NULL)
		NIT_THROW_FMT(EX_IO, "AssetManager not available");

	String dir = _dir;
	String patternDir;
	String filePattern;

	size_t slashPos = pattern.find_last_of('/');

	if (slashPos != pattern.npos)
	{
		patternDir = pattern.substr(0, slashPos+1);
		dir += patternDir;
		filePattern = pattern.substr(slashPos+1);
	}
	else
	{
		filePattern = pattern;
	}

	// openDir() requires no trailing '/'
	if (!dir.empty() && dir[dir.length()-1] == '/')
		dir.resize(dir.length()-1);

	AAssetDir* assetDir = AAssetManager_openDir(_assetManager, dir.c_str());
	if (assetDir == NULL)
		return;

	while (true)
	{
		const char* filename = AAssetDir_getNextFileName(assetDir);
		if (filename == NULL) break;

		// filename returned as all directory part extracted and only the last component

		if (Wildcard::match(filePattern, filename))
		{
			String streamName = patternDir + filename;
			varResults.insert(std::make_pair(streamName, new Asset(this, streamName)));
		}
	}

	AAssetDir_close(assetDir);
}

bool AssetArchive::fixCase(String& streamName)
{
	String dir = _dir;
	String patternDir;
	String filePattern;

	size_t slashPos = streamName.find_last_of('/');

	if (slashPos != streamName.npos)
	{
		patternDir = streamName.substr(0, slashPos+1);
		dir += patternDir;
		filePattern = streamName.substr(slashPos+1);
	}
	else
	{
		filePattern = streamName;
	}

	// openDir() requires no trailing '/'
	if (!dir.empty() && dir[dir.length()-1] == '/')
		dir.resize(dir.length()-1);

	AAssetDir* assetDir = AAssetManager_openDir(_assetManager, dir.c_str());
	if (assetDir == NULL)
		return false;

	bool fixed = false;
	while (true)
	{
		const char* filename = AAssetDir_getNextFileName(assetDir);
		if (filename == NULL) break;

		// filename returned as all directory part extracted and only the last component

		if (_strcmpi(filename, filePattern.c_str()) == 0)
		{
			LOG(0, "?? fixed case: '%s' -> '%s'\n", makeUrl(streamName).c_str(), makeUrl(patternDir + filename).c_str());
			streamName = patternDir + filename;
			fixed = true;
			break;
		}
	}

	AAssetDir_close(assetDir);

	return fixed;
}

StreamSource* AssetArchive::locateLocal(const String& givenStreamName)
{
	String streamName = givenStreamName;

	// HACK: There's no something like AAssetDir_exist(), so try to open and then close at once
	AAsset* assetPeer = openAsset(streamName);
	if (assetPeer == NULL)
		return NULL;

	AAsset_close(assetPeer);

	return new Asset(this, streamName);
}

StreamReader* AssetArchive::open(const String& givenStreamName)
{
	String streamName = givenStreamName;

	AAsset* assetPeer = openAsset(streamName);
	if (assetPeer == NULL)
		return Archive::open(streamName);

	Ref<Asset> asset = new Asset(this, streamName);
	return asset->openWith(assetPeer);
}

AAsset* AssetArchive::openAsset(String& streamName)
{
	if (_assetManager == NULL)
		NIT_THROW_FMT(EX_IO, "AssetManager not availaible");

	AAsset* peer = AAssetManager_open(_assetManager, (_dir + streamName).c_str(), AASSET_MODE_UNKNOWN);
	if (peer) return peer;

	if (!_fixCaseProblem || !fixCase(streamName))
		return NULL;

	peer = AAssetManager_open(_assetManager, (_dir + streamName).c_str(), AASSET_MODE_UNKNOWN);
	return peer;
}

////////////////////////////////////////////////////////////////////////////////

AssetArchive::Asset::Asset(AssetArchive* arc, const String& name)
: File(arc, name)
{
	_cachedSize = size_t(-1);
}

size_t AssetArchive::Asset::getMemorySize()
{
	if (_cachedSize == size_t(-1))
	{
		AssetArchive* arc = static_cast<AssetArchive*>(getRealLocator());

		AAsset* peer = arc->openAsset(_name);
		if (peer)
		{
			_cachedSize = AAsset_getLength(peer);
			AAsset_close(peer);
		}
	}

	return _cachedSize;
}

StreamReader* AssetArchive::Asset::open()
{
	AssetArchive* arc = static_cast<AssetArchive*>(getRealLocator());

	AAsset* peer = arc->openAsset(_name);
	if (peer == NULL)
		NIT_THROW_FMT(EX_IO, "Can't open asset '%s'", _name.c_str());

	return openWith(peer);
}

StreamReader* AssetArchive::Asset::openRange(size_t offset, size_t size, StreamSource* source)
{
	if (source == NULL)
		source = this;

	AssetArchive* arc = static_cast<AssetArchive*>(getRealLocator());

	AAsset* peer = arc->openAsset(_name);
	if (peer == NULL)
		NIT_THROW_FMT(EX_IO, "Can't open asset '%s'", _name.c_str());

	if (_cachedSize == size_t(-1))
		_cachedSize = AAsset_getLength(peer);

	off_t start, length;
	int fd = AAsset_openFileDescriptor(peer, &start, &length);

	if (fd < 0)
		NIT_THROW_FMT(EX_IO, "Can't open handle asset '%s': asset without file descriptor", _name.c_str());

	FILE* file = fdopen(fd, "r");

	return new AssetFileReader(source, peer, file, start + offset, size);
}

StreamReader* AssetArchive::Asset::openWith(AAsset* peer)
{
	if (_cachedSize == size_t(-1))
		_cachedSize = AAsset_getLength(peer);

	off_t start, length;
	int fd = AAsset_openFileDescriptor(peer, &start, &length);

	if (fd < 0)
	{
		LOG(0, ".. '%s': no fd, using AssetReader\n", getUrl().c_str());

		// Can't access directly - maybe its compressed etc. Use AssetReader
		return new AssetReader(this, peer);
	}
	else
	{
		// We can access directly - it's a memory mapped file
		FILE* file = fdopen(fd, "r");

		return new AssetFileReader(this, peer, file, start, length);
	}
}

////////////////////////////////////////////////////////////////////////////////

AssetArchive::AssetReader::AssetReader(AssetArchive::Asset* asset, AAsset* peer)
: _asset(asset), _peer(peer)
{
}

size_t AssetArchive::AssetReader::readRaw(void* buf, size_t size)
{
	return AAsset_read(_peer, buf, size);
}

void AssetArchive::AssetReader::onDelete()
{
	if (_peer)
	{
		AAsset_close(_peer);
		_peer = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

AssetArchive::AssetFileReader::AssetFileReader(StreamSource* source, AAsset* assetPeer, NIT_FILE_HANDLE fileHandle, size_t offset, size_t size)
: FileReader(source, fileHandle, offset, &size), _assetPeer(assetPeer)
{

}

void AssetArchive::AssetFileReader::onDelete()
{
	FileReader::onDelete();
	if (_assetPeer)
	{
		AAsset_close(_assetPeer);
		_assetPeer = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////

// JNI_OnLoad() / JNI_OnUnload() is not necessary when we use native activity
// If you want to hook loadLibrary() on static initialization, uncomment and add some code

// extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
// {
// 	return g_AndroidJNI->OnLoadJNI(vm, reserved);
// }
// 
// extern "C" void JNI_OnUnload(JavaVM* vm, void* reserved)
// {
// 	return g_AndroidJNI->OnUnloadJNI(vm, reserved);
// }

