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

class NITBUNDLER_API Command : public RefCounted
{
public:
	class NITBUNDLER_API Param : public RefCounted
	{
	public:
		String							type;
		String							target;
		StringVector					args;
	};

	enum StepOrder
	{
		STEP_INITIALIZE					= 100,
		STEP_POPULATE					= 200,
		STEP_FILTER_OUT					= 300,
		STEP_SPAWN_MERGE				= 400,
		STEP_MUTATE						= 500,
		STEP_FINALIZE					= 600,
	};

protected:
	Command(const String& command);

public:
	virtual int							getOrder() = 0;
	const String&						getCommand()							{ return _command; }

	virtual bool						isSplitArgsNeeded()						{ return true; }

	virtual Param*						parseParam(const String& line);

	virtual void						prepare(PackSource* pack, Param* param) = 0;
	virtual void						execute(Handler* hdlr, Param* param) = 0;

private:
	String								_command;
};

////////////////////////////////////////////////////////////////////////////////

// Basic bundler commands

////////////////////////////////////////////////////////////////////////////////

class CopyCommand : public Command
{
public:
	CopyCommand() : Command("copy") { }

	virtual int							getOrder()								{ return STEP_POPULATE; }
	virtual void						prepare(PackSource* pack, Param* param);
	virtual void						execute(Handler* hdlr, Param* param);
};

////////////////////////////////////////////////////////////////////////////////

class IgnoreCommand : public Command
{
public:
	IgnoreCommand() : Command("ignore") { }

	virtual int							getOrder()								{ return STEP_FILTER_OUT; }
	virtual void						prepare(PackSource* pack, Param* param);
	virtual void						execute(Handler* hdlr, Param* param);
};

////////////////////////////////////////////////////////////////////////////////

class LodCommand : public Command
{
public:
	LodCommand() : Command("lod") { }

	virtual int							getOrder()								{ return STEP_SPAWN_MERGE; }
	virtual void						prepare(PackSource* pack, Param* param);
	virtual void						execute(Handler* hdlr, Param* param);
};

////////////////////////////////////////////////////////////////////////////////

class MergeCommand : public Command
{
public:
	MergeCommand() : Command("merge") { }

	virtual int							getOrder()								{ return STEP_SPAWN_MERGE; }
	virtual void						prepare(PackSource* pack, Param* param);
	virtual void						execute(Handler* hdlr, Param* param);
};

////////////////////////////////////////////////////////////////////////////////

class ExecCommand : public Command
{
public:
	ExecCommand() : Command("exec") { }

	virtual int							getOrder()								{ return STEP_SPAWN_MERGE; }
	virtual void						prepare(PackSource* pack, Param* param);
	virtual void						execute(Handler* hdlr, Param* param);
};

////////////////////////////////////////////////////////////////////////////////

class ExternCommand : public Command
{
public:
	ExternCommand() : Command("extern") { }

	virtual int							getOrder()								{ return STEP_SPAWN_MERGE; }
	virtual void						prepare(PackSource* pack, Param* param);
	virtual void						execute(Handler* hdlr, Param* param);
};

////////////////////////////////////////////////////////////////////////////////

class CompileCommand : public Command
{
public:
	CompileCommand() : Command("compile") { }

	virtual int							getOrder()								{ return STEP_MUTATE; }
	virtual void						prepare(PackSource* pack, Param* param);
	virtual void						execute(Handler* hdlr, Param* param);
};

////////////////////////////////////////////////////////////////////////////////

class CodecCommand : public Command
{
public:
	CodecCommand() : Command("codec") { }

	virtual int							getOrder()								{ return STEP_MUTATE; }
	virtual void						prepare(PackSource* pack, Param* param);
	virtual void						execute(Handler* hdlr, Param* param);
};

////////////////////////////////////////////////////////////////////////////////

class ResizeCommand : public Command
{
public:
	ResizeCommand() : Command("resize") { } 

	virtual int							getOrder()								{ return STEP_MUTATE; }
	virtual void						prepare(PackSource* pack, Param* param);
	virtual void						execute(Handler* hdlr, Param* param);
};

////////////////////////////////////////////////////////////////////////////////

class PayloadCommand : public Command
{
public:
	PayloadCommand() : Command("payload") { }

	virtual int							getOrder()								{ return STEP_MUTATE; }
	virtual void						prepare(PackSource* pack, Param* param);
	virtual void						execute(Handler* hdlr, Param* param);
};

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
