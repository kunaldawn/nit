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

#include "nit2d/nit2d_pch.h"

#import "nit2d/Nit2dAppDelegate.h"

#import "nitrender/EAGLView_ios.h"
#import "cocos2d.h"

#include "nit2d/CocosService.h"

#import <CoreFoundation/CoreFoundation.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#include "CCDirector.h"

using namespace nit;

////////////////////////////////////////////////////////////////////////////////

@implementation Nit2dAppDelegate

@synthesize interval = _interval;
@synthesize viewctrl = _viewController;
@synthesize deviceToken = _deviceToken;
@synthesize window = _window;

@synthesize handleURL = _handleURL;
@synthesize handleURLSource = _handleURLSource;
@synthesize handleURLAnnotation = _handleURLAnnotation;

Nit2dAppDelegate* s_SharedAppDelegate = nil;

+ (id) sharedAppDelegate
{
	return s_SharedAppDelegate;
}

- (void) alloc
{
	if (s_SharedAppDelegate == nil)
		s_SharedAppDelegate = self;
}

- (void) dealloc
{
	[_window release];
	
	if (s_SharedAppDelegate == self)
		s_SharedAppDelegate = nil;
	
	[super dealloc];
}

- (BOOL) application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    NSLog(@"Nit2dAppDelegate: application did finish launching with options");
    
	// Add the view controller's view to the window and display
	id mainScreen = [UIScreen mainScreen];
	
	_window = [[UIWindow alloc] initWithFrame: [mainScreen bounds]];
	
	EAGLView2* eaglView = [EAGLView2 viewWithFrame: [_window bounds]];
	
	[eaglView setMultipleTouchEnabled: YES];

	// Use Nit2dViewController manage EAGLView
	_viewController = [[Nit2dViewController alloc] initWithNibName:nil bundle:nil];
	_viewController.wantsFullScreenLayout = YES;
	_viewController.view = eaglView;

    ////////////////////////////////////
	
	// Init nit runtime
	NitRuntime* rt = NitRuntime::getSingleton();
	bool ok = rt->init();
	
	if (!ok)
		return NO;
	
	if (s_SharedAppDelegate == nil)
		s_SharedAppDelegate = self;
	
    ////////////////////////////////////
    
	// Set RootViewController to window
	[_window addSubview: eaglView];
	[_window makeKeyAndVisible];
	
	// Hide status bar
	[[UIApplication sharedApplication] setStatusBarHidden: YES];
	
	// Init displayLink callback
	_displayLink = [mainScreen displayLinkWithTarget:self selector:@selector(onDisplayLinkUpdate:)];
	[_displayLink setFrameInterval: 1];
	[_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	    
    _deviceToken = [NSString stringWithFormat:@"NegativeToken"];
    
	return YES;
}

- (void) applicationWillResignActive:(UIApplication *)application
{
    NSLog(@"Nit2dAppDelegate: application will resign active");

	AppBase* app = AppBase::getCurrent();
	if (app)
	{
		AppBase::ScopedEnter sc(app);
        app->saveNow();
        app->_setSuspended(true);
	}
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
    NSLog(@"Nit2dAppDelegate: application did become active");

	AppBase* app = AppBase::getCurrent();
	if (app)
	{
		AppBase::ScopedEnter sc(app);
        app->_setSuspended(false);
	}
}

- (void) applicationDidEnterBackground:(UIApplication *)application
{
    NSLog(@"Nit2dAppDelegate: application did enter background");

	AppBase* app = AppBase::getCurrent();
	if (app)
	{
		AppBase::ScopedEnter sc(app);
        app->_setActive(false);
	}
}

- (void) applicationWillEnterForeground:(UIApplication *)application
{
    NSLog(@"Nit2dAppDelegate: application will enter foreground");

	AppBase* app = AppBase::getCurrent();
	if (app)
	{
		AppBase::ScopedEnter sc(app);
        app->_setActive(true);
	}
}

- (void) applicationWillTerminate:(UIApplication *)application
{
    NSLog(@"Nit2dAppDelegate: application will terminate");

	AppBase* app = AppBase::getCurrent();
	if (app)
	{
		AppBase::ScopedEnter sc(app);
        app->saveNow();
		app->stop();
	}
}

- (void) applicationDidReceiveMemoryWarning:(UIApplication *)application
{
    NSLog(@"Nit2dAppDelegate: application did receive memory warning");

	AppBase* app = AppBase::getCurrent();
	if (app)
	{
		AppBase::ScopedEnter sc(app);
        app->saveNow();
        app->_notifyLowMemory();
	}
}

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)token 
{
	NSLog(@"deviceToken : %@", token);
    NSLog(@"deviceToken Size: %d", [token length]);

    _deviceToken = [NSString stringWithFormat:@"%@", token];
    [_deviceToken retain];
}

// For push notification : invoked when there's error to register a device to APNS
- (void)application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error 
{
	NSLog(@"deviceToken error : %@", error);
    
    _deviceToken = [NSString stringWithFormat:@"NegativeToken"];
    [_deviceToken retain];    
}

- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)url 
{
	// Deprecated but devices with old SDK may call this - implemented same as below openURL
    _handleURL = url;
    _handleURLSource = @"";
    _handleURLAnnotation = nil;
    
	AppBase* app = AppBase::getCurrent();

	if (app)
	{
		AppBase::ScopedEnter sc(app);
        String urlStr = [[url absoluteString] UTF8String];
        
        app->_notifyHandleURL(urlStr, "");
	}
    
    return true;
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation 
{
    _handleURL = url;
    _handleURLSource = sourceApplication;
    _handleURLAnnotation = annotation;
    
	AppBase* app = AppBase::getCurrent();
    
	if (app)
	{
		AppBase::ScopedEnter sc(app);
        String urlStr = [[url absoluteString] UTF8String];
        String source = [sourceApplication UTF8String];
        DataValue data; // TODO: convert annotation dict to a DataRecord
        
        app->_notifyHandleURL(urlStr, source, &data);
	}
    
    return true;
}

- (void) onDisplayLinkUpdate: (id)sender
{
	NitRuntime* rt = NitRuntime::getSingleton();

	if (rt->isFinished())
		return;
	
	if (rt == NULL)
		return;

	bool running = rt->mainLoop();
	
	if (!running)
	{
		if (rt->isRestarting())
			rt->restart();
	}
}

@end

////////////////////////////////////////////////////////////////////////////////

@implementation Nit2dViewController

- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    // TODO: Handle device orientation change
//    bool forcePortrait = g_Cocos->GetDirector()->isForcePortrait();
//    
//    if (forcePortrait)
//        return UIInterfaceOrientationIsPortrait(toInterfaceOrientation);
//    else
//        return UIInterfaceOrientationIsLandscape(toInterfaceOrientation);
    
    return FALSE;
}

@end