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

#import <UIKit/UIKit.h>

#include "nit/event/Event.h"

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

class IOSTouchEvent : public Event
{
public:
	IOSTouchEvent()																{ }
	IOSTouchEvent(NSSet* touches, UIEvent* event)
	{
		_touches = touches;
		_uiEvent = event;
	}
	
public:
	NSSet*								getTouches() const						{ return _touches; }
	UIEvent*							getUIEvent() const						{ return _uiEvent; }
	
private:
	NSSet*								_touches;
	UIEvent*							_uiEvent;
};

NIT_EVENT_DECLARE(NIT_API, OnIOSTouchesBegan, IOSTouchEvent);
NIT_EVENT_DECLARE(NIT_API, OnIOSTouchesMoved, IOSTouchEvent);
NIT_EVENT_DECLARE(NIT_API, OnIOSTouchesEnded, IOSTouchEvent);
NIT_EVENT_DECLARE(NIT_API, OnIOSTouchesCancelled, IOSTouchEvent);

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////

@interface EAGLView2 : UIView
{
	nit::Ref<nit::EventChannel>		_channel;
}

+ (EAGLView2*) sharedView;

+ (id) viewWithFrame: (CGRect) frame;

- (nit::EventChannel*)					channel;

@end