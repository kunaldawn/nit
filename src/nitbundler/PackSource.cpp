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

#include "nitbundler/PackSource.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

PackSource::PackSource(Builder* builder, Settings* packCfg, const String& name, bool isAlias)
{
	_builder = builder;
	_packCfg = packCfg;
	_name = name;

	_alias = isAlias;
	_linked = false;
	_ignored = false;
}

bool PackSource::link()
{
	if (_linked) return true;

	_linked = true;

	// It's simpler for a dependancy linking of PackSource than Runtime's one.
	// Mainly reporting and check minimal dependancy cases.

 	const String& platform = _builder->getPlatformID();
 	const String& build = _builder->getBuildTarget();

	_alias = StringUtil::endsWith(_packCfg->getName(), ".pack.cfg");

	if (!_alias)
	{
		String filename, path;
		StringUtil::splitFilename(_packCfg->getUrl(), filename, path);
		_locator = new FileLocator(_name, path);
	}

	StringVector requires;
	_packCfg->find("package/require", requires);
	_packCfg->find(StringUtil::format("package/require@%s_%s", platform.c_str(), build.c_str()), requires);

	StringVector optionals;
	_packCfg->find("package/optional", optionals);
	_packCfg->find(StringUtil::format("package/optional@%s_%s", platform.c_str(), build.c_str()), requires);

	StringVector excludes;
	_packCfg->find("package/exclude", excludes);
	_packCfg->find(StringUtil::format("package/exclude@%s_%s", platform.c_str(), build.c_str()), requires);

	for (uint i=0; i < requires.size(); ++i)
	{
		String& pack = requires[i];
		if (std::find(excludes.begin(), excludes.end(), pack) != excludes.end())
			continue;

		PackSource* required = _builder->getPack(pack);
		if (required == NULL)
		{
			if (StringUtil::endsWith(pack, ".plugin")) // Ignore plugins at this stage
				continue;

			LOG(0, "*** '%s': required '%s' not found\n", _name.c_str(), pack.c_str());
			return false;
		}

		_requires.push_back(required);
		required->_dependants.insert(this);
	}

	for (uint i=0; i < optionals.size(); ++i)
	{
		String& pack = optionals[i];
		if (std::find(excludes.begin(), excludes.end(), pack) != excludes.end())
			continue;

		PackSource* optional = _builder->getPack(pack);
		if (optional == NULL)
			continue;

		_requires.push_back(optional);
		// Don't treat optional as dependants
	}

	return _linked;
}

void PackSource::findSource(const StringVector& patterns, StreamSourceMap& outResults)
{
	if (_locator == NULL) return;

	for (uint i=0; i<patterns.size(); ++i)
	{
		findSource(patterns[i], outResults);
	}
}

void PackSource::findSource(const String& pattern, StreamSourceMap& outResults)
{
	if (_locator == NULL) return;

	_locator->find(pattern, outResults);
}

void PackSource::findTarget(const String& pattern, StringVector& outFilenames)
{
	for (TargetFiles::iterator itr = _targetFiles.begin(), end = _targetFiles.end(); itr != end; ++itr)
	{
		if (Wildcard::match(pattern, itr->first))
			outFilenames.push_back(itr->first);
	}
}

void PackSource::findTarget(const StringVector& patterns, StringVector& outFilenames)
{
	for (uint i=0; i<patterns.size(); ++i)
	{
		findTarget(patterns[i], outFilenames);
	}
}

void PackSource::removeTargets(const String& pattern)
{
	if (Wildcard::has(pattern))
	{
		for (TargetFiles::iterator itr = _targetFiles.begin(); itr != _targetFiles.end(); )
		{
			if (Wildcard::match(pattern, itr->first))
				_targetFiles.erase(itr++);
			else
				++itr;
		}
	}
	else
	{
		_targetFiles.erase(pattern);
	}
}

void PackSource::removeTargets(StringVector& patterns)
{
	for (uint i=0; i<patterns.size(); ++i)
	{
		removeTargets(patterns[i]);
	}
}

void PackSource::addTarget(const String& filename, Command* cmd, Command::Param* param)
{
	TargetFile& target = _targetFiles[filename];
	Commands& cmds = target.commands;
	Commands::iterator itr = cmds.find(cmd);

	if (itr != cmds.end())
		if (resolveConflict(cmd, param, target))
			cmds.erase(itr);

	cmds.insert(std::make_pair(cmd, param));
}

void PackSource::addTarget(StringVector& filenames, Command* cmd, Command::Param* param)
{
	for (uint i=0; i < filenames.size(); ++i)
	{
		addTarget(filenames[i], cmd, param);
	}
}

void PackSource::addTarget(StreamSource* src, Command* cmd, Command::Param* param)
{
	TargetFile& target = _targetFiles[src->getName()];
	target.source = src;
	Commands& cmds = target.commands;
	Commands::iterator itr = cmds.find(cmd);

	if (itr != cmds.end())
	{
		if (resolveConflict(cmd, param, target))
			cmds.erase(itr);
	}

	cmds.insert(std::make_pair(cmd, param));
}

void PackSource::addTarget(StreamSourceMap& files, Command* cmd, Command::Param* param)
{
	for (StreamSourceMap::iterator itr = files.begin(), end = files.end(); itr != end; ++itr)
	{
		addTarget(itr->second, cmd, param);
	}
}

bool PackSource::resolveConflict(Command* cmd, Command::Param* param, TargetFile& target)
{
	// TODO: compare wildcard, etc
	return true;
}

void PackSource::prepareCommands()
{
	Settings* appCfg = _builder->_appCfg;
	Settings* appCfgInc = _builder->_appCfgInclude;

	const String& secName = _builder->_sectionName;
	const String& pfSecName = _builder->_platformSectionName;

	if (appCfgInc)
	{
		prepareCommands(appCfgInc->getSection(secName));
		prepareCommands(appCfgInc->getSection(pfSecName));
	}

	if (appCfg)
	{
		prepareCommands(appCfg->getSection(secName));
		prepareCommands(appCfg->getSection(pfSecName));
	}

	if (_packCfgInclude)
	{
		prepareCommands(_packCfgInclude->getSection(secName));
		prepareCommands(_packCfgInclude->getSection(pfSecName));
	}

	if (_packCfg)
	{
		prepareCommands(_packCfg->getSection(secName));
		prepareCommands(_packCfg->getSection(pfSecName));
	}
}

void PackSource::prepareCommands(Settings* section)
{
	if (section == NULL)
		return;

	// Prepare commands sequentially
	vector<Command*>::type& cmds = _builder->getPlatform()->getOrderedCommands();
	for (uint i=0; i < cmds.size(); ++i)
	{
		Command* cmd = cmds[i];

		prepareCommand(section, cmd);
	}
}

void PackSource::prepareCommand(Settings* section, Command* cmd)
{
	StringVector cmdEntries;

	section->find(cmd->getCommand(), cmdEntries);

	for (uint i=0; i < cmdEntries.size(); ++i)
	{
		Ref<Command::Param> param = cmd->parseParam(cmdEntries[i]);
		cmd->prepare(this, param);
	}
}

void PackSource::executeCommands(Packer* packer)
{
	vector<Command*>::type& cmds = _builder->getPlatform()->getOrderedCommands();

	for (TargetFiles::iterator itr = _targetFiles.begin(), end = _targetFiles.end(); itr != end; ++itr)
	{
		const String& filename = itr->first;
		TargetFile& target = itr->second;
		StreamSource* source = target.source;
		Commands& cmdEntries = target.commands;

		Ref<Handler> handler = packer->assign(filename, source);

		// Execute commands sequentially
		for (uint i=0; i < cmds.size(); ++i)
		{
			Command* cmd = cmds[i];
			Commands::iterator cmdEntry = cmdEntries.find(cmd);
			if (cmdEntry == cmdEntries.end())
				continue;

			Command::Param* param = cmdEntry->second;

			try
			{
				cmd->execute(handler, param);
			}
			catch (Exception& ex)
			{
				LOG(0, "*** %s: %s: %s fail: %s\n",
					_name.c_str(), filename.c_str(), cmd->getCommand().c_str(), ex.getDescription().c_str());
				// TODO: handler->MarkError();
			}
		}
	}
	
	// At this point, packer can start its async jobs.

	// TODO: Setup flags.
}

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
