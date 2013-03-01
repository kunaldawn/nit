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

#include "nit/runtime/NitRuntime.h"

#include "nit/data/DataValue.h"
#include "nit/net/Socket.h"

#include <android/asset_manager_jni.h>

////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if_ether.h>
#include <net/if.h>
#include <errno.h>
#include <net/ethernet.h>

#include <sys/system_properties.h>

#define AF_LINK AF_PACKET

char *
ether_ntoa_r (const struct ether_addr *addr, char * buf)
{
	snprintf(buf, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
		addr->ether_addr_octet[0], addr->ether_addr_octet[1],
		addr->ether_addr_octet[2], addr->ether_addr_octet[3],
		addr->ether_addr_octet[4], addr->ether_addr_octet[5]);
	return buf;
}

/*
* Convert Ethernet address to standard hex-digits-and-colons printable form.
*/
char *
ether_ntoa (const struct ether_addr *addr)
{
	static char buf[18];
	return ether_ntoa_r(addr, buf);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

AndroidJNI NitRuntime::_jni;

NitRuntime::NitRuntime()
{
	app_dummy(); // Prevents app glue module to be stripped

	// Initialize platform specific members
	_assetArchive			= NULL;
	_androidApp			= NULL;
	_sensorManager			= NULL;
	_accelerometerSensor	= NULL;
	_sensorEventQueue		= NULL;
	_savedStateSource		= NULL;

#ifndef NIT_NO_LOG
	// Install a Logcat instance
	_logCat = new LogCatLogger();
	_logCat->setLogLevel(LOG_LEVEL_VERBOSE);
	LogManager::getSingleton().attach(_logCat);
#endif
}

bool NitRuntime::initPlatform()
{
	if (_jni.getJavaVM() == NULL)
	{
		alert(getTitle(), "No java vm linked", true);
		return false;
	}

	if (_assetArchive == NULL)
	{
		LOG(0, "?? no asset archive specified\n");
	}

	if (_androidApp == NULL)
	{
		LOG(0, "?? no android app specified\n");
	}

	_config->set("platform", "android");

	char buf[4096];

	// adb shell getprop
	__system_property_get("ro.product.model", buf);
	_config->set("device_model", buf);

	__system_property_get("ro.product.manufacturer", buf);
	_config->set("device_vendor", buf);

	__system_property_get("ro.build.version.release", buf);
	_config->set("os_version", buf);

	__system_property_get("ro.product.cpu.abi", buf);
	_config->set("android/cpu_family", buf);

	// Compose meaningful device_name (android doesn't have a hostname or a device name)
	__system_property_get("net.hostname", buf);
	String hostname = buf;

	String deviceName = 
		_config->get("device_vendor") + "-" + 
		_config->get("device_model") + "-" +
		_config->get("os_version") + "-" + 
		hostname;
	_config->set("device_name", deviceName);
	_config->set("host_name", deviceName);

	// App path is prefixed by 'asset' on android
	_config->set("app_path", "asset:/"); // makes '$(app_path)/bundles' to 'asset://bundles'

	// Have to setup followings at activity.setupPathCallback() callback

	AndroidJNI::MethodInfo m;
	AndroidJNI* jni = g_AndroidJNI;
	jobject javaActivity = jni->getActivityInstance();

	bool ok = jni->getMethodInfo(m, javaActivity, "setupPathCallback", "()V");

	if (ok)
	{
		m.env->CallVoidMethod(javaActivity, m.methodID);
		m.env->DeleteLocalRef(m.javaClass);
		ok = m.env->ExceptionCheck() == false;
	}

	if (!ok)
	{
		m.env->ExceptionClear();
		alert(getTitle(), "NitActivity.setupPathCallback() failed", true);
		return false;
	}

	return true;
}

extern "C" JNIEXPORT void Java_nit_NitActivity_nativeSetupPath(
	JNIEnv* env, jobject thiz,
	jstring jobbDir,
	jstring jfilesDir,
	jstring jcacheDir,
	jstring jextFilesDir,
	jstring jextCacheDir)
{
	using namespace nit;

	Settings* ss = NitRuntime::getSingleton()->getConfig();

	String obbDir = g_AndroidJNI->toString(jobbDir, env);
	String filesDir = g_AndroidJNI->toString(jfilesDir, env);
	String cacheDir = g_AndroidJNI->toString(jcacheDir, env);
	String extFilesDir = g_AndroidJNI->toString(jextFilesDir, env);
	String extCacheDir = g_AndroidJNI->toString(jextCacheDir, env);

	ss->set("android/obb_dir", obbDir);
	ss->set("android/files_dir", filesDir);
	ss->set("android/cache_dir", cacheDir);
	ss->set("android/ext_files_dir", extFilesDir);
	ss->set("android/ext_cache_dir", extCacheDir);

	ss->set("app_data_path", !filesDir.empty() ? filesDir : extFilesDir);
	ss->set("app_cache_path", !cacheDir.empty() ? cacheDir : extCacheDir);
	ss->set("user_data_path", !extFilesDir.empty() ? extFilesDir : filesDir);
	ss->set("user_cache_path", !extCacheDir.empty() ? extCacheDir : cacheDir);
	ss->set("sys_temp_path", !cacheDir.empty() ? cacheDir : extCacheDir);
}

void NitRuntime::finishPlatform()
{
}

Archive* NitRuntime::openArchive(const String& name, const String& url)
{
	// TODO: Adopt an url protocol officially for all platform

	size_t splitPos = url.find("://");

	if (splitPos == url.npos)
		return NitRuntimeBase::openArchive(name, url);

	String protocol = url.substr(0, splitPos);
	String path = url.substr(splitPos + 3);

	if (protocol == "asset")
	{
		if (path.empty())
			return _assetArchive;

		return new AssetArchive(name, _assetArchive, path);
	}
	else
	{
		LOG(0, "*** unknown protocol: '%s'\n", url.c_str());
		return NitRuntimeBase::openArchive(name, url);
	}
}

void NitRuntime::updateEnv()
{
	LOG_TIMESCOPE(0, "++ UpdateEnv()");

	// Update language & country code

	// Set en-us as default
	_config->set("language", "en");
	_config->set("country", "us");

	if (_androidApp)
	{
		// See <android/configuration.h>

		char buf[3] = { 0 };

		// HACK: Some android device gives code in uppercase, so make it lowercase
		AConfiguration_getLanguage(_androidApp->config, buf);
		String lang = buf;
		StringUtil::toLowerCase(lang);
		_config->set("language", lang);

		AConfiguration_getCountry(_androidApp->config, buf);
		String country = buf;
		StringUtil::toLowerCase(country);
		_config->set("country", country);

		if (_androidApp->activity->internalDataPath)
			_config->set("android/internal_data_path", _androidApp->activity->internalDataPath);

		if (_androidApp->activity->externalDataPath)
			_config->set("android/external_data_path", _androidApp->activity->externalDataPath);

		_config->set("android/mcc", DataValue(AConfiguration_getMcc(_androidApp->config)).toString());
		_config->set("android/mnc", DataValue(AConfiguration_getMnc(_androidApp->config)).toString());

		const char* val = "";

		switch (AConfiguration_getOrientation(_androidApp->config))
		{
		case ACONFIGURATION_ORIENTATION_ANY:		val = "any"; break;
		case ACONFIGURATION_ORIENTATION_PORT:		val = "port"; break;
		case ACONFIGURATION_ORIENTATION_LAND:		val = "land"; break;
		case ACONFIGURATION_ORIENTATION_SQUARE:		val = "square"; break;
		default:									val = "<unknown>"; break;
		}
		_config->set("android/orientation", val);

		switch (AConfiguration_getTouchscreen(_androidApp->config))
		{
		case ACONFIGURATION_TOUCHSCREEN_ANY:		val = "any"; break;
		case ACONFIGURATION_TOUCHSCREEN_NOTOUCH:	val = "notouch"; break;
		case ACONFIGURATION_TOUCHSCREEN_STYLUS:		val = "stylus"; break;
		case ACONFIGURATION_TOUCHSCREEN_FINGER:		val = "finger"; break;
		default:									val = "<unknown>"; break;
		}
		_config->set("android/touchscreen", val);

		_config->set("android/density", DataValue(AConfiguration_getDensity(_androidApp->config)).toString());

		switch (AConfiguration_getKeyboard(_androidApp->config))
		{
		case ACONFIGURATION_KEYBOARD_ANY:			val = "any"; break;
		case ACONFIGURATION_KEYBOARD_NOKEYS:		val = "nokeys"; break;
		case ACONFIGURATION_KEYBOARD_QWERTY:		val = "qwerty"; break;
		case ACONFIGURATION_KEYBOARD_12KEY:			val = "12key"; break;
		default:									val = "<unknown>"; break;
		}
		_config->set("android/keyboard", val);

		switch (AConfiguration_getNavigation(_androidApp->config))
		{
		case ACONFIGURATION_NAVIGATION_ANY:			val = "any"; break;
		case ACONFIGURATION_NAVIGATION_NONAV:		val = "nonav"; break;
		case ACONFIGURATION_NAVIGATION_DPAD:		val = "dpad"; break;
		case ACONFIGURATION_NAVIGATION_TRACKBALL:	val = "trackball"; break;
		case ACONFIGURATION_NAVIGATION_WHEEL:		val = "wheel"; break;
		default:									val = "<unknown>"; break;
		}
		_config->set("android/navigation", val);

		switch (AConfiguration_getKeysHidden(_androidApp->config))
		{
		case ACONFIGURATION_KEYSHIDDEN_ANY:			val = "any"; break;
		case ACONFIGURATION_KEYSHIDDEN_NO:			val = "no"; break;
		case ACONFIGURATION_KEYSHIDDEN_YES:			val = "yes"; break;
		case ACONFIGURATION_KEYSHIDDEN_SOFT:		val = "soft"; break;
		default:									val = "<unknown>"; break;
		}
		_config->set("android/keys_hidden", val);

		_config->set("android/sdk_version", DataValue(AConfiguration_getSdkVersion(_androidApp->config)).toString());

		const char* deviceForm = "unknown";

		switch (AConfiguration_getScreenSize(_androidApp->config))
		{
		case ACONFIGURATION_SCREENSIZE_ANY:			val = "any"; break;
		case ACONFIGURATION_SCREENSIZE_SMALL:		val = "small"; deviceForm = "pocket"; break;
		case ACONFIGURATION_SCREENSIZE_NORMAL:		val = "normal"; deviceForm = "pocket"; break;
		case ACONFIGURATION_SCREENSIZE_LARGE:		val = "large"; deviceForm = "pocket"; break;
		case ACONFIGURATION_SCREENSIZE_XLARGE:		val = "xlarge"; deviceForm = "tablet"; break;
		default:									val = "<unknown>"; break;
		}
		_config->set("android/screen_size", val);
		_config->set("device_form", deviceForm);

		switch (AConfiguration_getScreenLong(_androidApp->config))
		{
		case ACONFIGURATION_SCREENLONG_ANY:			val = "any"; break;
		case ACONFIGURATION_SCREENLONG_NO:			val = "no"; break;
		case ACONFIGURATION_SCREENLONG_YES:			val = "yes"; break;
		default:									val = "<unknown>"; break;
		}
		_config->set("android/screen_long", val);

		switch (AConfiguration_getUiModeType(_androidApp->config))
		{
		case ACONFIGURATION_UI_MODE_TYPE_ANY:		val = "any"; break;
		case ACONFIGURATION_UI_MODE_TYPE_NORMAL:	val = "normal"; break;
		case ACONFIGURATION_UI_MODE_TYPE_DESK:		val = "desk"; break;
		case ACONFIGURATION_UI_MODE_TYPE_CAR:		val = "car"; break;
		default:									val = "<unknown>"; break;
		}
		_config->set("android/ui_mode_type", val);

		switch (AConfiguration_getUiModeNight(_androidApp->config))
		{
		case ACONFIGURATION_UI_MODE_NIGHT_ANY:		val = "any"; break;
		case ACONFIGURATION_UI_MODE_NIGHT_NO:		val = "no"; break;
		case ACONFIGURATION_UI_MODE_NIGHT_YES:		val = "yes"; break;
		default:									val = "<unknown>"; break;
		}
		_config->set("android/ui_mode_night", val);
	}

	// TODO: Investigate what happens when eject / insert sdcard

	_config->dump();
}

void NitRuntime::updateNet()
{
	LOG_TIMESCOPE(0, "++ UpdateNet()");

	_ipAddrs.clear();
	
	if (!SocketBase::initialize())
	{
		LOG(0, "*** Can't initialize socket\n");
		return;
	}
	
	StringVector if_names;
	StringVector ip_names;
	StringVector hw_addrs;
	vector<uint>::type ip_addrs;
	
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		LOG(0, "*** can't create socket\n");
		return;
	}
	
	ifconf ifc;
	char buffer[4096];

	// Examine ip address
	ifc.ifc_len = sizeof(buffer);
	ifc.ifc_buf = buffer;
	
	if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)
	{
		LOG(0, "*** can't set ioctl to SIOCGIFCONF\n");
		return;
	}
	
	char lastname[IFNAMSIZ];
	lastname[0] = 0;
		
	// step 1. examine AF_INET and generate list
	for (char* ptr = buffer; ptr < buffer + ifc.ifc_len; )
	{
		ifreq* ifr = (ifreq*) ptr;
		int len = sizeof(ifr->ifr_addr);
		ptr += sizeof(ifr->ifr_name) + len; // for next one in buffer
		
		if (ifr->ifr_addr.sa_family != AF_INET)
			continue;
		
		// extract ip address
		char* cptr; // colon position
		if ((cptr = (char*)strchr(ifr->ifr_name, ':')) != NULL)
			*cptr = 0; // replace colon with null
		
		if (strncmp(lastname, ifr->ifr_name, IFNAMSIZ) == 0)
			continue; // already processes this interface
		
		memcpy(lastname, ifr->ifr_name, IFNAMSIZ);
		
		ifreq ifrcopy = *ifr;
		ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy);
		int flags = ifrcopy.ifr_flags;
		if ((flags & IFF_UP) == 0)
			continue; // ignore if interface not up

		sockaddr_in* sin = (sockaddr_in*)&ifr->ifr_addr;

		if_names.push_back(ifr->ifr_name);
		ip_names.push_back(inet_ntoa(sin->sin_addr));
		ip_addrs.push_back(sin->sin_addr.s_addr);
		hw_addrs.push_back(String());
	}

	// step 2. 
	// Android provides if_dl no more since ndk r8_b
	// So we can't obtain mac address
	
	close(sockfd);
	
	// step 3. store as nit format
	
	int otherCount = 0;
	int enCount = 0;
	int pppCount = 0;
	int loopbackCount = 0;
	int wifiCount = 0;
	
	for (uint i = 0; i < if_names.size(); ++i)
	{
		_ipAddrs.insert(std::make_pair(if_names[i].c_str(), ip_names[i].c_str()));
		
		LOG(0, "++ %s (%s) = %s\n", hw_addrs[i].c_str(), if_names[i].c_str(), ip_names[i].c_str());
	}
	
	// set main adapter
	if (_ipAddrs.find("wlan0") != _ipAddrs.end())
		_ipAddrs["main"] = _ipAddrs["wlan0"];

	if (if_names.empty())
		LOG(0, "*** UpdateNet failed\n");
}

static void ShowMessageBox(const char* title, const char* msg, const char* btn = NULL)
{
	AndroidJNI::MethodInfo m;

	bool ok = g_AndroidJNI->getMethodInfo(m,
		g_AndroidJNI->getActivityInstance(),
		"showMessageBox",
		"(Ljava/lang/String;Ljava/lang/String;)V");

	if (!ok) return;

	if (title == NULL) title = "";

	jstring titleStr = m.env->NewStringUTF("");
	jstring msgStr = m.env->NewStringUTF(msg);

	m.env->CallVoidMethod(g_AndroidJNI->getActivityInstance(), m.methodID, titleStr, msgStr);

	m.env->DeleteLocalRef(titleStr);
	m.env->DeleteLocalRef(msgStr);
	m.env->DeleteLocalRef(m.javaClass);
}

void NitRuntime::info(const String &title, const String &message, bool userInfo)
{
	LOG(0, "++ %s: %s\n", title.c_str(), message.c_str());
	if (userInfo || _infoEnabled)
		ShowMessageBox(title.c_str(), message.c_str());
}

void NitRuntime::alert(const String &title, const String &message, bool fatal)
{
	LOG(0, "!!! %s: %s\n", title.c_str(), message.c_str());
	if (fatal || _alertEnabled)
		ShowMessageBox(title.c_str(), message.c_str());
}

void NitRuntime::setAssetArchive(AssetArchive* arc)
{
	_assetArchive = arc;
	// TODO: Treat as EnvChanged
}

void NitRuntime::linkAndroidApp(android_app* app)
{
	_androidApp = app;

	_assetArchive = new AssetArchive(app->activity->assetManager);

	app->onAppCmd = appCmdCallback;
	app->onInputEvent = inputEventCallback;

	// Install an accelerometer
	// TODO: Refactor to InputService
	_sensorManager = ASensorManager_getInstance();
	_accelerometerSensor = ASensorManager_getDefaultSensor(_sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	_sensorEventQueue = ASensorManager_createEventQueue(_sensorManager, app->looper, LOOPER_ID_USER, NULL, NULL); // TODO: add LOOPER_ID_SENSOR instead of LOOPER_ID_USER

	if (app->savedState != NULL)
	{
		// app->savedState can be vaporized at any time by android after this function returns.
		// So make a copy as soon as possible.
		_savedStateSource = new MemorySource("$saved_state", new MemoryBuffer(app->savedState, app->savedStateSize));

		// TODO: Send some event for this saved_state data
	}
}

static bool g_AndroidAppRequestsStop = false;
static bool g_AndroidAppRequestsRestart = false;

extern "C" JNIEXPORT void Java_nit_NitActivity_nitRestart(JNIEnv* env, jobject thiz)
{
	using namespace nit;

	g_AndroidAppRequestsRestart = true;
}

extern "C" JNIEXPORT void Java_nit_NitActivity_nitStop(JNIEnv* env, jobject thiz)
{
	using namespace nit;

	g_AndroidAppRequestsStop = true;
}

bool NitRuntime::onSystemLoop()
{
	// Settle all pending events here

	while (true)
	{
		// To avoid tight loop and preserve battery
		if (_androidApp == NULL || _androidApp->window == NULL)
		{
			struct timespec ts;
			ts.tv_sec  = (long) 0;
			ts.tv_nsec = (long) 100 * 1000000;
			int rc = ::nanosleep(&ts, 0);
		}

		if (_androidApp == NULL)
			return true;

		int looperID, events;
		android_poll_source* source;

		looperID = ALooper_pollAll(0, NULL, &events, (void**)&source);
		if (looperID < 0)
			return true;

		// Handle andorid app events
		if (source != NULL)
			source->process(_androidApp, source);

		// Handle LOOPER_ID_USER
		if (looperID == LOOPER_ID_USER)
		{
			if (_sensorEventQueue && ASensorEventQueue_hasEvents(_sensorEventQueue))
			{
				// Exhaust sensor events piled on sensor queue, just leave final value
				// TODO: Apply some filter (ex: avg)?
				ASensorEvent sensorEvent;
				while (ASensorEventQueue_getEvents(_sensorEventQueue, &sensorEvent, 1) > 0);

				// Send the final value
				if (_channel)
					_channel->send(Events::OnAndroidSensor, new AndroidSensorEvent(&sensorEvent));
			}
		}

		// TODO: handle sensor information

		// handle restart / destroy / stop
		if (g_AndroidAppRequestsStop || g_AndroidAppRequestsRestart || _androidApp->destroyRequested)
		{
			_restarting = g_AndroidAppRequestsRestart;

			if (_androidApp->destroyRequested)
			{
				LOG(0, "++ AndroidApp requested destroy\n");
			}

			if (g_AndroidAppRequestsRestart)
			{
				LOG(0, "++ AndroidApp requested restart\n");
			}

			if (g_AndroidAppRequestsStop)
			{
				LOG(0, "++ AndroidApp requested stop\n");
			}

			g_AndroidAppRequestsStop = false;
			g_AndroidAppRequestsRestart = false;
			return false;
		}
	}
}

bool NitRuntime::waitForWindowInit()
{
	LOG_TIMESCOPE(0, "++ Waiting for app window init");

	while (true)
	{
		if (onSystemLoop() == false)
			return false;

		if (_androidApp->window != NULL)
			return true;
	}
}

void NitRuntime::appCmdCallback(android_app* app, int32_t cmd)
{
	NitRuntime* rt = getSingleton();

	if (rt && rt->_androidApp == app)
		rt->onAppCmd(app, cmd);
}

int32_t NitRuntime::inputEventCallback(android_app* app, AInputEvent* event)
{
	NitRuntime* rt = getSingleton();

	if (rt && rt->_androidApp == app)
		return rt->onInputEvent(app, event);

	return 0;
}

void NitRuntime::accelerometerIncUseCount()
{
	// TODO: IMPL THIS
}

void NitRuntime::accelerometerDecUseCount()
{
	// TODO: IMPL THIS
}

void NitRuntime::onAppCmd(android_app* app, int32_t cmd)
{
	// Runtime handles first then user-code via channel

	switch (cmd)
	{
	case APP_CMD_INPUT_CHANGED:			
		LOG(0, "++ APP_CMD_INPUT_CHANGED\n");
		break;

	case APP_CMD_INIT_WINDOW:			
		LOG(0, "++ APP_CMD_INIT_WINDOW\n");
		break;

	case APP_CMD_TERM_WINDOW:
		LOG(0, "++ APP_CMD_TERM_WINDOW\n");
		break;

	case APP_CMD_WINDOW_RESIZED:		
		// TODO: handle within nit
		break;

	case APP_CMD_WINDOW_REDRAW_NEEDED:	
		break;

	case APP_CMD_CONTENT_RECT_CHANGED:	
		break;

	case APP_CMD_GAINED_FOCUS:
		LOG(0, "++ APP_CMD_GAINED_FOCUS\n");
		if (_accelerometerSensor)
		{
			// TODO: refactor to InputService
			ASensorEventQueue_enableSensor(_sensorEventQueue, _accelerometerSensor);
			// TODO: expose param to app.cfg
			ASensorEventQueue_setEventRate(_sensorEventQueue, _accelerometerSensor, (1000L/60)*1000);
			LOG(0, ".. Accelerometer enabled\n");
		}
		// TODO: handle within nit
		break;

	case APP_CMD_LOST_FOCUS:
		LOG(0, "++ APP_CMD_LOST_FOCUS\n");
		if (_accelerometerSensor)
		{
			ASensorEventQueue_disableSensor(_sensorEventQueue, _accelerometerSensor);
			LOG(0, ".. Accelerometer disabled\n");
		}
		// TODO: handle within nit
		break;

	case APP_CMD_CONFIG_CHANGED:
		LOG(0, "++ APP_CMD_CONFIG_CHANGED\n");
		updateEnv();
		break;

	case APP_CMD_LOW_MEMORY:			
		LOG(0, "++ APP_CMD_LOW_MEMORY\n");
		break;

	case APP_CMD_START:					
		// TODO: handle within nit
		LOG(0, "++ APP_CMD_START\n");
		break;

	case APP_CMD_RESUME:				
		// TODO: handle within nit
		LOG(0, "++ APP_CMD_RESUME\n");
		break;

	case APP_CMD_SAVE_STATE:			
		// TODO: handle within nit
		LOG(0, "++ APP_CMD_SAVE_STATE\n");
		break;

	case APP_CMD_PAUSE:					
		// TODO: handle within nit
		LOG(0, "++ APP_CMD_PAUSE\n");
		break;

	case APP_CMD_STOP:					
		// TODO: handle within nit
		LOG(0, "++ APP_CMD_STOP\n");
		break;

	case APP_CMD_DESTROY:				
		// TODO: handle within nit
		LOG(0, "++ APP_CMD_DESTROY\n");
		break;
	}

	// Send to channel
	if (_channel)
		_channel->send(Events::OnAndroidAppCmd, new AndroidAppCmdEvent(app, cmd));
}

int32_t NitRuntime::onInputEvent(android_app* app, AInputEvent* input)
{
	if (_channel)
	{
		Ref<AndroidInputEvent> evt = new AndroidInputEvent(app, input);
		_channel->send(Events::OnAndroidInput, evt);

		// treat as handled if consumed
		return evt->isConsumed();
	}

	// if there's no channel, treat as not handled
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
