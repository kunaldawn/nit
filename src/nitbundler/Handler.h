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

// The class encapsulates the handling of each file in a pack.
class NITBUNDLER_API Handler : public RefCounted
{
public:
	Handler();

public:
	virtual void						addMergeList(const String& filename);
	virtual void						setCodec(const String& codec);
	virtual void						setResize(const String& resize);
	virtual void						setCompile(const String& compile);
	virtual void						setPayload(const String& payload);

	virtual bool						prepare()								{ return true; }
	virtual void						generate();

protected:
	friend class Packer;
	Weak<Packer::FileEntry>				_entry;

	String								_payloadStr;
	uint16								_payloadType;

	void								generate(StreamSource* source);
};

////////////////////////////////////////////////////////////////////////////////

class NITBUNDLER_API CopyHandler : public Handler
{
protected:
	virtual void						generate();
};

////////////////////////////////////////////////////////////////////////////////

class NITBUNDLER_API PackCfgHandler : public Handler
{
protected:
	virtual void						generate();
};

////////////////////////////////////////////////////////////////////////////////

class NITBUNDLER_API NitScriptHandler : public Handler
{
public:
	NitScriptHandler();

protected:
	virtual void						setCompile(const String& compile);
	virtual void						generate();

	bool								_compile;
};

////////////////////////////////////////////////////////////////////////////////

class NITBUNDLER_API DiffEntryHandler : public Handler
{
public:
	DiffEntryHandler(PackArchive::File* original) : _original(original) { }

protected:
	virtual bool						prepare();
	virtual void						generate();

protected:
	Ref<PackArchive::File>				_original;
};

////////////////////////////////////////////////////////////////////////////////

class NITBUNDLER_API DeletedEntryHandler : public Handler
{
protected:
	virtual bool						prepare();
	virtual void						generate();
};

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
