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

#include "nitbundler/Command.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Command::Command(const String& command)
: _command(command)
{

}

Command::Param* Command::parseParam(const String& line)
{
	Param* param = new Param();

	// cmd = [type:] [target <] arg1 [arg2 [...]]

	size_t colonPos = line.find(':');
	size_t begin = 0;

	if (colonPos != line.npos)
	{
		param->type = line.substr(0, colonPos);
		StringUtil::trim(param->type);
		begin = colonPos + 1;
	}

	size_t ltPos = line.find('<', begin);

	if (ltPos != line.npos)
	{
		param->target = line.substr(begin, ltPos - begin);
		StringUtil::trim(param->target);
		begin = ltPos + 1;
	}

	if (isSplitArgsNeeded())
	{
		while (begin < line.size())
		{
			size_t nextArgPos = line.find_first_of(" \t", begin);

			if (nextArgPos == line.npos)
				nextArgPos = line.size();

			nextArgPos = line.find_first_not_of(" \t", nextArgPos);
			if (nextArgPos == line.npos)
				nextArgPos = line.size();

			param->args.push_back(line.substr(begin, nextArgPos - begin));
			StringUtil::trim(param->args.back());
			if (param->args.back().empty())
				param->args.pop_back();

			begin = nextArgPos;
		}
	}
	else
	{
		param->args.push_back(line.substr(begin));
		StringUtil::trim(param->args.back());
		if (param->args.back().empty())
			param->args.pop_back();
	}

	return param;
}

////////////////////////////////////////////////////////////////////////////////

void CopyCommand::prepare(PackSource* pack, Param* param)
{
	StreamSourceMap files;
	pack->findSource(param->args, files);
	pack->addTarget(files, this, NULL);
}

void CopyCommand::execute(Handler* hdlr, Param* param)
{
	// NOTHING TO DO
}

////////////////////////////////////////////////////////////////////////////////


void IgnoreCommand::prepare(PackSource* pack, Param* param)
{
	StringVector files;
	pack->findTarget(param->args, files);
	pack->removeTargets(files);
}

void IgnoreCommand::execute(Handler* hdlr, Param* param)
{
	// NOTHING TO DO
}

////////////////////////////////////////////////////////////////////////////////

void LodCommand::prepare(PackSource* pack, Param* param)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void LodCommand::execute(Handler* hdlr, Param* param)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

////////////////////////////////////////////////////////////////////////////////

void MergeCommand::prepare(PackSource* pack, Param* param)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void MergeCommand::execute(Handler* hdlr, Param* param)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

////////////////////////////////////////////////////////////////////////////////

void ExecCommand::prepare(PackSource* pack, Param* param)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void ExecCommand::execute(Handler* hdlr, Param* param)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

////////////////////////////////////////////////////////////////////////////////

void ExternCommand::prepare(PackSource* pack, Param* param)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void ExternCommand::execute(Handler* hdlr, Param* param)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

////////////////////////////////////////////////////////////////////////////////

void CompileCommand::prepare(PackSource* pack, Param* param)
{
	StringVector files;
	pack->findTarget(param->args, files);
	pack->addTarget(files, this, param);
}

void CompileCommand::execute(Handler* hdlr, Param* param)
{
	hdlr->setCompile(param->type);
}

////////////////////////////////////////////////////////////////////////////////

void CodecCommand::prepare(PackSource* pack, Param* param)
{
	StringVector files;
	pack->findTarget(param->args, files);
	pack->addTarget(files, this, param);
}

void CodecCommand::execute(Handler* hdlr, Param* param)
{
	hdlr->setCodec(param->type);
}

////////////////////////////////////////////////////////////////////////////////

void ResizeCommand::prepare(PackSource* pack, Param* param)
{
	StringVector files;
	pack->findTarget(param->args, files);
	pack->addTarget(files, this, param);
}

void ResizeCommand::execute(Handler* hdlr, Param* param)
{
	hdlr->setResize(param->type);
}

////////////////////////////////////////////////////////////////////////////////

void PayloadCommand::prepare(PackSource* pack, Param* param)
{
	StringVector files;
	pack->findTarget(param->args, files);
	pack->addTarget(files, this, param);
}

void PayloadCommand::execute(Handler* hdlr, Param* param)
{
	hdlr->setPayload(param->type);
}

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
