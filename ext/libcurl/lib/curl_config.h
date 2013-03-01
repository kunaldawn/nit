#pragma once

#if defined(ANDROID)
/*  Âü°í: http://thesoftwarerogue.blogspot.com/2010/05/porting-of-libcurl-to-android-os-using.html */
#	include "curl_config_android.h"
#endif

#if defined(_IOS)
#	include "curl_config_ios.h"
#endif