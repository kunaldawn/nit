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

#include "nit/nit.h"
#include "nit/io/ContentTypes.h"
#include "nit/io/Stream.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class StreamSource;
class StreamReader;
class StreamWriter;
class Settings;

typedef vector<Ref<Settings> >::type SettingsList;

////////////////////////////////////////////////////////////////////////////////

class NIT_API Settings : public RefCounted
{
public:
	Settings(StreamSource* source = NULL);

public:
	const String&						getName()								{ return _name; }
	StreamSource*						getSource()								{ return _source; }
	String								getUrl()								{ return _source->getUrl(); }

public:
	Settings*							getParent()								{ return _parent; }
	Settings*							getRoot()								{ return _parent ? _parent->getRoot() : this; }
	const String&						getPath()								{ return needPath(); }

	Settings*							getBase() const							{ return _base; }
	void								setBase(Settings* base)					{ _base = base; }

public:
	Settings*							getSection(const String& name);
	void								addSection(const String& name, Settings* section);
	void								removeSection(const String& name);
	void								findSections(const String& pattern, SettingsList& varResults, bool recursive = false);

public:
	bool								has(const String& keyPath);
	const String&						get(const String& keyPath, const String& defaultVal = StringUtil::BLANK(), bool warnIfNotFound = true);
	void								set(const String& keyPath, const String& value);
	void								add(const String& keyPath, const String& value);
	void								remove(const String& keyPath);

	void								find(const String& pattern, StringVector& varResults);
	void								findKeys(const String& pattern, StringVector& varKeys);

public:
	typedef multimap<String, String>::type Values;
	typedef Values::iterator Iterator;

	Iterator							begin()									{ return _values.begin(); }
	Iterator							end()									{ return _values.end(); }

public:
	String								expand(const String& orig, bool throwEx=true);

public:
	static Settings*					load(StreamSource* source, ContentType treatAs = ContentType::UNKNOWN);

	void								clear(bool recursive = true);
	void								loadCfg(StreamReader* reader);
	void								loadJson(StreamReader* reader);
	void								loadPlist(StreamReader* reader);

	void								saveCfg(StreamWriter* writer, Settings* root = NULL, String prevPath = StringUtil::BLANK());
	void								saveJson(JsonPrinter* printer);

	void								dump();

protected:
	typedef multimap<String, Ref<Settings> >::type Sections;

	Ref<StreamSource>					_source;
	String								_name;
	String								_path;
	Weak<Settings>						_parent;
	Weak<Settings>						_base;
	Values								_values;
	Sections							_sections;

	class JSONHandler;

	const String&						needPath();
	void								doFind(const String& pattern, StringVector& varResults, bool findKey);
	const String*						doGet(const String& keyPath);
	void								doSet(const String& keyPath, const String* value, bool eraseExisting);
	Settings*							doGetSection(const String& name);
	void								doAddSection(const String& name, Settings* section);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
