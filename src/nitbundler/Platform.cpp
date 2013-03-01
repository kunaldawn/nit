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

#include "nitbundler_pch.h"

#include "nitbundler/Platform.h"
#include "nitbundler/ImageHandler.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Platform::Platform(const String& name, Package* pack)
: Module(name, pack)
{
}

void Platform::onInit()
{
	Register(new IgnoreCommand());
	Register(new CopyCommand());
	Register(new CompileCommand());
	Register(new CodecCommand());
	Register(new ResizeCommand());
	Register(new PayloadCommand());
	Register(new LodCommand());
	Register(new MergeCommand());
	Register(new ExecCommand());
	Register(new ExternCommand());

	_defaultHandler						= createHandler<CopyHandler>;

	_handlerLookup["cfg"]				= createHandler<CopyHandler>;

	_handlerLookup["nit"]				= createHandler<NitScriptHandler>;

	_handlerLookup["bmp"]				= createHandler<ImageHandler>;
	_handlerLookup["gif"]				= createHandler<ImageHandler>;
	_handlerLookup["png"]				= createHandler<ImageHandler>;
	_handlerLookup["jpg"]				= createHandler<ImageHandler>;
	_handlerLookup["jpeg"]				= createHandler<ImageHandler>;
	_handlerLookup["tiff"]				= createHandler<ImageHandler>;
	_handlerLookup["exif"]				= createHandler<ImageHandler>;
	_handlerLookup["wmf"]				= createHandler<ImageHandler>;
	_handlerLookup["emf"]				= createHandler<ImageHandler>;
	_handlerLookup["tga"]				= createHandler<ImageHandler>;
	_handlerLookup["dds"]				= createHandler<ImageHandler>;
	_handlerLookup["ccz"]				= createHandler<ImageHandler>;

	_handlerLookup["material"]			= createHandler<CopyHandler>;
	_handlerLookup["compositor"]		= createHandler<CopyHandler>;
	_handlerLookup["overlay"]			= createHandler<CopyHandler>;
	_handlerLookup["pu"]				= createHandler<CopyHandler>;
	_handlerLookup["pua"]				= createHandler<CopyHandler>;

	_handlerLookup["mesh"]				= createHandler<CopyHandler>;
	_handlerLookup["skeleton"]			= createHandler<CopyHandler>;

	_handlerLookup["program"]			= createHandler<CopyHandler>;
	_handlerLookup["hlsl"]				= createHandler<CopyHandler>;
	_handlerLookup["glsl"]				= createHandler<CopyHandler>;
}

void Platform::Register(Command* cmd)
{
	_commandLookup.erase(cmd->getCommand());
	_commandLookup.insert(std::make_pair(cmd->getCommand(), cmd));
}

Command* Platform::getCommand(const String& command)
{
	CommandLookup::iterator itr = _commandLookup.find(command);

	return itr != _commandLookup.end() ? itr->second : NULL;
}

Handler* Platform::NewHandler(const String& ext)
{
	CreateHandlerFn fn = _defaultHandler;

	HandlerLookup::iterator itr = _handlerLookup.find(ext);

	if (itr != _handlerLookup.end() && itr->second)
		fn = itr->second;

	if (fn)
		return fn(this, ext);
	else
		return NULL;
}

vector<Command*>::type& Platform::getOrderedCommands()
{
	if (!_orderedCommands.empty())
		return _orderedCommands;

	for (CommandLookup::iterator itr = _commandLookup.begin(), end = _commandLookup.end(); itr != end; ++itr)
	{
		_orderedCommands.push_back(itr->second);
	}

	struct LessOrder
	{
		bool operator() (Command* a, Command* b) { return a->getOrder() < b->getOrder(); }
	};

	std::sort(_orderedCommands.begin(), _orderedCommands.end(), LessOrder());

	return _orderedCommands;
}

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
