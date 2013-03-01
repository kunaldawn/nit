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

#include "nitbundler/Command.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NITBUNDLER_API PackSource : public RefCounted
{
public:
	PackSource(Builder* builder, Settings* packCfg, const String& name, bool isAlias);

public:
	const String&						getName()								{ return _name; }
	FileLocator*						getLocator()							{ return _locator; }
	Builder*							getBuilder()							{ return _builder; }

	Settings*							getPackCfg()							{ return _packCfg; }
	Settings*							getPackCfgInclude()						{ return _packCfgInclude; }

	bool								isAlias()								{ return _alias; }
	bool								isLinked()								{ return _linked; }
	bool								isIgnored()								{ return _ignored; }

public:
	typedef vector<Ref<PackSource> >::type Requires;
	typedef set<Weak<PackSource> >::type Dependants;

	Requires&							getRequires()							{ return _requires; }
	Dependants&							getDependants()							{ return _dependants; }

public:
	bool								link();

public:
	void								findSource(const StringVector& patterns, StreamSourceMap& outResults);
	void								findSource(const String& pattern, StreamSourceMap& outResults);

	void								findTarget(const String& pattern, StringVector& outFilenames);
	void								findTarget(const StringVector& patterns, StringVector& outFilenames);

	void								addTarget(const String& filename, Command* cmd, Command::Param* param);
	void								addTarget(StringVector& filenames, Command* cmd, Command::Param* param);
	void								addTarget(StreamSource* src, Command* cmd, Command::Param* param);
	void								addTarget(StreamSourceMap& files, Command* cmd, Command::Param* param);

	void								removeTargets(const String& pattern);
	void								removeTargets(StringVector& patterns);

private:
	friend class Builder;

	String								_name;
	Weak<Builder>						_builder;
	Ref<FileLocator>					_locator;
	Ref<Settings>						_packCfg;
	Ref<Settings>						_packCfgInclude;

	Requires							_requires;
	Dependants							_dependants;

	bool								_alias;
	bool								_linked;
	bool								_ignored;

	typedef map<Command*, Ref<Command::Param> >::type Commands;

	struct TargetFile
	{
		Ref<StreamSource>				source;
		Commands						commands;
	};

	typedef unordered_map<String, TargetFile>::type TargetFiles;

	TargetFiles							_targetFiles;

	void								prepareCommands();
	void								prepareCommands(Settings* section);
	void								prepareCommand(Settings* section, Command* cmd);

	void								executeCommands(Packer* packer);

	bool								resolveConflict(Command* cmd, Command::Param* param, TargetFile& target);
};

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
