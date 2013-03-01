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

#include "nit/platform/android/AndroidSupport.h"

#include "nit/io/MemoryBuffer.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API NitRuntime : public NitRuntimeBase
{
public:
	static AndroidJNI*					getJNI()								{ return &_jni; }

public:									// PlatformBase Impl
	virtual void						info(const String& title, const String& message, bool userInfo=false);
	virtual void						alert(const String& title, const String& message, bool fatal=false);
	virtual Archive*					openArchive(const String& name, const String& url);
	virtual bool						onSystemLoop();

public:
	void								linkAndroidApp(android_app* app);

	AssetArchive*						getAssetArchive()						{ return _assetArchive; }
	void								setAssetArchive(AssetArchive* arc);

	android_app*						getAndroidApp()							{ return _androidApp; }
	MemorySource*						getSavedStateSource()					{ return _savedStateSource; }

	bool								waitForWindowInit();

protected:
	NitRuntime();

	virtual bool						initPlatform();
	virtual void						finishPlatform();
	virtual void						updateEnv();
	virtual void						updateNet();

	Ref<LogCatLogger>					_logCat;

private:
	Ref<AssetArchive>					_assetArchive;
	android_app*						_androidApp;
	Ref<MemorySource>					_savedStateSource;
	bool								_javaRequestsStop;
	bool								_javaRequestsRestart;

	static AndroidJNI					_jni;
	friend class AndroidJNI;

public:									// TODO: Refactor to InputService
	void								accelerometerIncUseCount();
	void								accelerometerDecUseCount();

private:								// TODO: Refactor to InputService
	ASensorManager*						_sensorManager;
	const ASensor*						_accelerometerSensor;
	bool								_accelerometerActive;
	int									_accelerometerUseCount;
	ASensorEventQueue*					_sensorEventQueue;

private:
	void								onAppCmd(android_app* app, int32_t cmd);
	int32_t								onInputEvent(android_app* app, AInputEvent* event);

	static void							appCmdCallback(android_app* app, int32_t cmd);
	static int32_t						inputEventCallback(android_app* app, AInputEvent* event);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
