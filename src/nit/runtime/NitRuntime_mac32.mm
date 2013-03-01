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

#include "nit/net/Socket.h"

////////////////////////////////////////////////////////////////////////////////

#import <AppKit/AppKit.h>

#import <mach-o/dyld.h>

#import <sys/utsname.h>

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
#include <sys/sockio.h>
#include <net/if.h>
#include <errno.h>
#include <net/if_dl.h>
#include <net/ethernet.h>

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NitRuntime::NitRuntime()
{
#ifndef NIT_NO_LOG
	LogManager::getSingleton().attach(new StdLogger(stdout, stderr));
#endif
}

bool NitRuntime::initPlatform()
{
	_config->set("platform", "mac32");
	_config->set("device_vendor", "apple");
    // TODO: impl
    _config->set("device_model", "<unknown>");
    _config->set("device_form", "desktop");
    _config->set("os_version", "<unknown>");

	struct utsname u;
	uname(&u);
    
	String deviceModel = u.machine;
	
	// obtain bundle path
	char path[PATH_MAX];
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	assert(mainBundle);
	
	CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
	assert(mainBundleURL);
	
	CFStringRef cfStringRef = CFURLCopyFileSystemPath(mainBundleURL, kCFURLPOSIXPathStyle);
	assert(cfStringRef);
	
	CFStringGetCString(cfStringRef, path, PATH_MAX, kCFStringEncodingUTF8);
	
	String appPath = path;
	
	CFRelease(mainBundleURL);
	CFRelease(cfStringRef);
	
	// obtain library path
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);

	String libPath = [[paths objectAtIndex: 0] UTF8String];
	
	// obtain document path
	paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	String docPath = [[paths objectAtIndex: 0] UTF8String];
	
	// obtain cache path
	paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
	String cachePath = [[paths objectAtIndex: 0] UTF8String];
	
	FileUtil::normalizeSeparator(appPath);
	FileUtil::normalizeSeparator(libPath);
	FileUtil::normalizeSeparator(docPath);
	FileUtil::normalizeSeparator(cachePath);
	
	_config->set("app_path", appPath);
	_config->set("app_data_path", libPath);
	_config->set("app_cache_path", cachePath);
	_config->set("user_data_path", docPath);
	_config->set("user_cache_path", docPath + "/cache");
	_config->set("sys_temp_path", cachePath + "/temp");
	
	return true;
}

void NitRuntime::finishPlatform()
{
}

void NitRuntime::updateEnv()
{
	// Obtain country info
	NSLocale* locale = [NSLocale currentLocale];
	String country = [[locale objectForKey: NSLocaleCountryCode] UTF8String];

	// Obtain language info (The value in NSLocale attached to country, so do it separately)
	NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
	NSArray* languages = [defaults objectForKey:@"AppleLanguages"];
	NSString* currentLanguage = [languages objectAtIndex:0];
	NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:currentLanguage];
	String language = [[temp objectForKey: NSLocaleLanguageCode] UTF8String];
	
	StringUtil::toLowerCase(language);
	StringUtil::toLowerCase(country);
	
	_config->set("language", language);
	_config->set("country", country);
}

void NitRuntime::updateNet()
{
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

	// Examine hostname
	if (gethostname(buffer, sizeof(buffer)) == SOCKET_ERROR)
	{
		LOG(0, "*** can't get local host name\n");
		return;
	}

	_config->set("host_name", buffer);
	LOG(0, "++ Host name: %s\n", buffer);

	// Examine ip address & mac addr
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
		int len = Math::max(sizeof(ifr->ifr_addr), (size_t)ifr->ifr_addr.sa_len);
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
	
	// step 2. examine AF_LINK and associate each to ip address of ifr_name
	for (char* ptr = buffer; ptr < buffer + ifc.ifc_len; )
	{
		ifreq* ifr = (ifreq*) ptr;
		int len = Math::max(sizeof(ifr->ifr_addr), (size_t)ifr->ifr_addr.sa_len);
		ptr += sizeof(ifr->ifr_name) + len; // for next one in buffer
		
		if (ifr->ifr_addr.sa_family != AF_LINK)
			continue;
		
		String if_name = ifr->ifr_name;
		
		StringVector::iterator itr = std::find(if_names.begin(), if_names.end(), if_name);
		
		if (itr == if_names.end())
			continue;
		
		sockaddr_dl *sdl = (sockaddr_dl*)&ifr->ifr_addr;
		int a, b, c, d, e, f;
		String macAddr = ether_ntoa((ether_addr*)LLADDR(sdl));
		sscanf(macAddr.c_str(), "%x:%x:%x:%x:%x:%x", &a, &b, &c, &d, &e, &f);
		macAddr = StringUtil::format("%02X:%02X:%02X:%02X:%02X:%02X",a,b,c,d,e,f);

		size_t index = itr - if_names.begin();
		hw_addrs[index] = macAddr;
	}
	
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
	if (_ipAddrs.find("en0") != _ipAddrs.end())
		_ipAddrs["main"] = _ipAddrs["en0"];
}

String NitRuntime::getExecutablePath()
{
    char path[MAX_PATH];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size))
        NIT_THROW_FMT(EX_OVERFLOW, "buffer too small; need size %u\n", size);
    
    char resolved_path[MAX_PATH];
    if (realpath(path, resolved_path))
        return resolved_path;
    
    NIT_THROW(EX_SYSTEM);
}

static void showMessageBox(const char* pszTitle, const char* pszMsg, const char* pszBtn = NULL)
{
	NSString * title = (pszTitle) ? [NSString stringWithUTF8String : pszTitle] : nil;
    NSString * msg = (pszMsg) ? [NSString stringWithUTF8String : pszMsg] : nil;
	NSString * btn = (pszBtn) ? [NSString stringWithUTF8String:pszBtn] : @"OK";
    
    NSAlert* alert = [NSAlert alertWithMessageText:title
                                     defaultButton:btn
                                   alternateButton:nil
                                       otherButton:nil
                         informativeTextWithFormat:@"%@", msg ];
    
    [alert runModal];
}

void NitRuntime::info(const String &title, const String &message, bool userInfo)
{
	LOG(0, "++ %s: %s\n", title.c_str(), message.c_str());
	if (userInfo || _infoEnabled)
		showMessageBox(title.c_str(), message.c_str());
}

void NitRuntime::alert(const String &title, const String &message, bool fatal)
{
	LOG(0, "!!! %s: %s\n", title.c_str(), message.c_str());
	if (fatal || _alertEnabled)
		showMessageBox(title.c_str(), message.c_str());
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
