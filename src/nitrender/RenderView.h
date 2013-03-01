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

#include "nitrender/nitrender.h"

#include "nit/app/AppBase.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class RenderDevice;
class RenderContext;
class InputDevice;

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API RenderView : public RefCounted
{
public:
	enum Orientation
	{
		ORIENT_PORTRAIT,
		ORIENT_PORTRAIT_FLIP,
		ORIENT_LANDSCAPE,
		ORIENT_LANDSCAPE_FLIP,
	};

public:
	RenderView();
	virtual ~RenderView();

public:
	Orientation							getOrientation()						{ return _orientation; }
	virtual void						setOrientation(Orientation orient) = 0;
	bool								isPortrait()							{ return _orientation == ORIENT_PORTRAIT || _orientation == ORIENT_PORTRAIT_FLIP; }
	bool								isLandscape()							{ return _orientation == ORIENT_LANDSCAPE || _orientation == ORIENT_LANDSCAPE_FLIP; }

public:
	int									getLeft()								{ return _left; }
	int									getTop()								{ return _top; }
	int									getWidth()								{ return _width; }
	int									getHeight()								{ return _height; }

	float								getScale()								{ return _scale; }
	virtual void						setScale(float scale)					{ _scale = scale; }
	float								getScaledWidth()						{ return _width * _scale; }
	float								getScaledHeight()						{ return _height * _scale; }

public:
	EventChannel*						channel()								{ return _channel ? _channel : _channel = new EventChannel(); }

public:
	virtual RenderDevice*				getRenderDevice() = 0;
	virtual InputDevice*				getInputDevice() = 0;

private:
	friend class						RenderService;
	RenderContext*						_newRenderContext()						{ return onNewRenderContext(); }

protected:
	virtual RenderContext*				onNewRenderContext() = 0;

protected:
	int									_left;
	int									_top;
	int									_width;
	int									_height;
	float								_scale;

	Orientation							_orientation;

	Ref<EventChannel>					_channel;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
