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

#include "nitbundler/nitbundler.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// The class encapsulates platform-wise specs
class NITBUNDLER_API Platform : public Module
{
public:
	Platform(const String& name, Package* pack);

public:
	virtual bool						isBigEndian() = 0;
	virtual const String&				getPlatformID() = 0;
	virtual uint32						getPlatformCode() = 0;

public:
	Command*							getCommand(const String& command);
	vector<Command*>::type&				getOrderedCommands();

	Handler*							NewHandler(const String& ext);

protected:								// Module Impl
	virtual void						onInit();

protected:
	typedef unordered_map<String, Ref<Command> >::type CommandLookup;
	CommandLookup						_commandLookup;
	vector<Command*>::type				_orderedCommands;

	void								Register(Command* cmd);

protected:
	typedef Handler* (*CreateHandlerFn) (Platform* platform, const String& ext);
	typedef unordered_map<String, CreateHandlerFn>::type HandlerLookup;
	HandlerLookup						_handlerLookup;
	CreateHandlerFn						_defaultHandler;

protected:
	template <typename THandler>
	static Handler*						createHandler(Platform* platform, const String& ext);
};

////////////////////////////////////////////////////////////////////////////////

template <typename THandler>
Handler* Platform::createHandler(Platform* platform, const String& ext)
{
	return new THandler();
}

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
