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

#import "nitrender/EAGLView_ios.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>

#include "nitrender/GLESRenderView_ios.h"

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

NIT_EVENT_DEFINE(OnIOSTouchesBegan, IOSTouchEvent);
NIT_EVENT_DEFINE(OnIOSTouchesMoved, IOSTouchEvent);
NIT_EVENT_DEFINE(OnIOSTouchesEnded, IOSTouchEvent);
NIT_EVENT_DEFINE(OnIOSTouchesCancelled, IOSTouchEvent);

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////

static EAGLView2* g_SharedView = nil;

@implementation EAGLView2

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

+ (EAGLView2*) sharedView
{
	return g_SharedView;
}

+ (id) viewWithFrame: (CGRect) frame
{
	return [[[self alloc] initWithFrame: frame] autorelease];
}

- (id) initWithFrame: (CGRect) frame
{
	if ((self = [super initWithFrame: frame]))
	{
		g_SharedView = self;
	}
	
	[self setContentScaleFactor: [[UIScreen mainScreen] scale]];
	
	return self;
}

- (id) initWithCoder: (NSCoder*) coder
{
	if ((self = [super initWithCoder: coder]))
	{
		g_SharedView = self;
	}
	
	[self setContentScaleFactor: [[UIScreen mainScreen] scale]];

	return self;
}

- (void) layoutSubviews
{
//	using namespace nit;
//	
//	Ref<GLESRenderView_ios> renderView = new GLESRenderView_ios();
}

- (nit::EventChannel*) channel
{
	return _channel ? _channel : _channel = new nit::EventChannel();
}

- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	using namespace nit;
	
	if (_channel)
		_channel->send(Events::OnIOSTouchesBegan, new IOSTouchEvent(touches, event));
}

- (void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	using namespace nit;

	if (_channel)
		_channel->send(Events::OnIOSTouchesMoved, new IOSTouchEvent(touches, event));
}

- (void) touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	using namespace nit;

	if (_channel)
		_channel->send(Events::OnIOSTouchesEnded, new IOSTouchEvent(touches, event));
}

- (void) touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	using namespace nit;

	if (_channel)
		_channel->send(Events::OnIOSTouchesCancelled, new IOSTouchEvent(touches, event));
}

@end


