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

#include "nit_pch.h"

#include "nit/data/Settings.h"
#include "nit/io/MemoryBuffer.h"
#include "nit/data/ParserUtil.h"
#include "nit/data/DataValue.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Settings::Settings(StreamSource* source)
: _source(source)
{

}

Settings* Settings::getSection(const String& path)
{
	Settings* target = this;

	size_t beginPos = 0;
	size_t splitPos = path.find_first_of('/');

	while (target && splitPos != path.npos)
	{
		String secName = path.substr(beginPos, splitPos - beginPos);
		if (splitPos == 0)
			target = target->getRoot();
		else if (secName == "..")
			target = target->getParent();
		else
			target = target->doGetSection(secName);

		beginPos = splitPos + 1;
		splitPos = path.find_first_of('/', beginPos);
	}

	if (target == NULL)
		return _base ? _base->getSection(path) : NULL;

	return beginPos ? target->doGetSection(path.substr(beginPos)) : target->doGetSection(path);
}

Settings* Settings::doGetSection(const String& name)
{
	Sections::iterator itr = _sections.find(name);

	if (itr != _sections.end())
		return itr->second;

	if (_base)
		return _base->doGetSection(name);

	return NULL;
}

void Settings::addSection(const String& path, Settings* section)
{
	Settings* target = this;

	size_t beginPos = 0;
	size_t splitPos = path.find_first_of('/');

	while (target && splitPos != path.npos)
	{
		Settings* next = NULL;
		String secName = path.substr(beginPos, splitPos - beginPos);
		if (splitPos == 0)
			next = target->getRoot();
		else if (secName == "..")
			next = target->getParent();
		else
		{
			next = target->doGetSection(secName);
			if (next == NULL)
			{
				next = new Settings(_source);
				target->doAddSection(secName, next);
			}
		}

		beginPos = splitPos + 1;
		splitPos = path.find_first_of('/', beginPos);

		if (next == NULL)
			NIT_THROW_FMT(EX_INVALID_PARAMS, "'%s': invalid key path", path.c_str());

		target = next;
	}

	target->doAddSection(path.substr(beginPos), section);
}

void Settings::doAddSection(const String& name, Settings* section)
{
	_sections.insert(std::make_pair(name, section));
	section->_name = name;
	section->_parent = this;
}

void Settings::removeSection(const String& name)
{
	Sections::iterator itr = _sections.find(name);
	if (itr != _sections.end())
	{
		Settings* section = itr->second;
		section->_parent = NULL;
		_sections.erase(itr);
	}
}

void Settings::findSections(const String& pattern, SettingsList& varResults, bool recursive)
{
	bool hasWildcard = Wildcard::has(pattern);

	if (recursive)
	{
		for (Sections::iterator itr = _sections.begin(); itr != _sections.end(); ++itr)
		{
			if (hasWildcard && Wildcard::match(pattern, itr->first, false))
				varResults.push_back(itr->second);
			else if (pattern == itr->first)
				varResults.push_back(itr->second);

			itr->second->findSections(pattern, varResults, recursive);
		}
	}
	else
	{
		if (!hasWildcard)
		{
			// If no wildcard, search using equal_range
			std::pair<Sections::iterator, Sections::iterator> range = _sections.equal_range(pattern);

			for (Sections::iterator itr = range.first; itr != range.second; ++itr)
				varResults.push_back(itr->second);
		}
		else
		{
			// If wildcard, search linearly using wildcard
			for (Sections::iterator itr = _sections.begin(); itr != _sections.end(); ++itr)
			{
				if (Wildcard::match(pattern, itr->first, false))
					varResults.push_back(itr->second);
			}
		}
	}

	if (_base)
		_base->findSections(pattern, varResults, recursive);
}

bool Settings::has(const String& keyPath)
{
	return doGet(keyPath) != NULL;
}

const String& Settings::get(const String& keyPath, const String& defaultVal, bool warnIfNotFound)
{
	const String* val = doGet(keyPath);

	if (val) return *val;

	if (warnIfNotFound)
	{
		LOG(0, "?? '%s': no '%s', using default '%s'\n",
			getPath().c_str(), keyPath.c_str(), defaultVal.c_str());
	}

	return defaultVal;
}

const String* Settings::doGet(const String& keyPath)
{
	Settings* target = this;

	size_t beginPos = 0;
	size_t splitPos = keyPath.find_first_of('/');

	while (target && splitPos != keyPath.npos)
	{
		String secName = keyPath.substr(beginPos, splitPos - beginPos);
		if (splitPos == 0)
			target = target->getRoot();
		else if (secName == "..")
			target = target->getParent();
		else
			target = target->doGetSection(secName);

		beginPos = splitPos + 1;
		splitPos = keyPath.find_first_of('/', beginPos);
	}

	if (target != NULL)
	{
		Values::iterator itr;
		if (beginPos == 0)
			itr = target->_values.find(keyPath);
		else
			itr = target->_values.find(keyPath.substr(beginPos));

		if (itr != target->_values.end())
			return &itr->second;
	}

	if (_base)
		return _base->doGet(keyPath);

	return NULL;
}

void Settings::doSet(const String& keyPath, const String* value, bool eraseExisting)
{
	Settings* target = this;

	size_t beginPos = 0;
	size_t splitPos = keyPath.find_first_of('/');

	while (target && splitPos != keyPath.npos)
	{
		Settings* next = NULL;
		String secName = keyPath.substr(beginPos, splitPos);
		if (splitPos == 0)
			next = target->getRoot();
		else if (secName == "..")
			next = target->getParent();
		else
		{
			next = target->doGetSection(secName);
			if (next == NULL)
			{
				next = new Settings(_source);
				target->doAddSection(secName, next);
			}
		}

		if (next == NULL)
		{
			NIT_THROW_FMT(EX_INVALID_PARAMS, "'%s': invalid key path", keyPath.c_str());
		}

		beginPos = splitPos + 1;
		splitPos = keyPath.find_first_of('/', beginPos);
		target = next;
	}

	if (beginPos == 0)
	{
		if (eraseExisting)
		{
			std::pair<Values::iterator, Values::iterator> range = _values.equal_range(keyPath);
			_values.erase(range.first, range.second);
		}

		if (value)
			_values.insert(std::make_pair(keyPath, *value));
	}
	else
	{
		if (eraseExisting)
		{
			std::pair<Values::iterator, Values::iterator> range = target->_values.equal_range(keyPath.substr(beginPos));
			target->_values.erase(range.first, range.second);
		}

		if (value)
			target->_values.insert(std::make_pair(keyPath.substr(beginPos), *value));
	}
}

void Settings::set(const String& keyPath, const String& value)
{
	doSet(keyPath, &value, true);
}

void Settings::add(const String& keyPath, const String& value)
{
	doSet(keyPath, &value, false);
}

void Settings::remove(const String& keyPath)
{
	doSet(keyPath, NULL, true);
}

void Settings::find(const String& pattern, StringVector& varResults)
{
	doFind(pattern, varResults, false);

	if (_base)
		_base->find(pattern, varResults);
}

void Settings::findKeys(const String& pattern, StringVector& varResults)
{
	doFind(pattern, varResults, true);

 	std::sort(varResults.begin(), varResults.end());
 	varResults.erase(std::unique(varResults.begin(), varResults.end()), varResults.end());

	if (_base)
		_base->findKeys(pattern, varResults);
}

void Settings::doFind(const String& pattern, StringVector& varResults, bool findKey)
{
	String localPattern;
	String childPattern;

	// Check if contains '/' for optimization
	size_t dpos = pattern.find('/');
	if (dpos == pattern.npos)
	{
		localPattern = pattern;
	}
	else
	{
		localPattern = pattern.substr(0, dpos++);
		childPattern = pattern.substr(dpos);
	}

	bool hasWildcard = Wildcard::has(localPattern);

	if (childPattern.empty())
	{
		// Search within current section

		if (!hasWildcard)
		{
			// If no wildcard, search using equal_range
			std::pair<Values::iterator, Values::iterator> range = _values.equal_range(localPattern);

			for (Values::iterator itr = range.first; itr != range.second; ++itr)
            {
				if (findKey)
					varResults.push_back(itr->first);
				else
					varResults.push_back(itr->second);
            }
			return;
		}
		else
		{
			// If wildcard, search linearly using wildcard
			for (Values::iterator itr = _values.begin(); itr != _values.end(); ++itr)
			{
				if (Wildcard::match(localPattern, itr->first, false))
                {
					if (findKey)
						varResults.push_back(itr->first);
					else
						varResults.push_back(itr->second);
                }
			}
		}
	}
	else
	{
		// Search within child sections

		if (!hasWildcard)
		{
			// If no wildcard, search using equal_range
			std::pair<Sections::iterator, Sections::iterator> range = _sections.equal_range(localPattern);

			for (Sections::iterator itr = range.first; itr != range.second; ++itr)
				itr->second->doFind(childPattern, varResults, findKey);
		}
		else
		{
			// If wildcard, search linearly using wildcard
			for (Sections::iterator itr = _sections.begin(); itr != _sections.end(); ++itr)
			{
				if (Wildcard::match(localPattern, itr->first, false))
					itr->second->doFind(childPattern, varResults, findKey);
			}
		}
	}
}

String Settings::expand(const String& aliased, bool throwEx)
{
	size_t next = 0;

	String str = aliased;
	
	while (true)
	{
		size_t dollar = str.find("$(", next);
		if (dollar == str.npos)
			break;

		size_t begin = dollar + 2;

		size_t end = str.find_first_of(')', begin);
		if (end == str.npos)
		{
			if (throwEx)
				NIT_THROW_FMT(EX_SYNTAX, "invalid alias: '%s'\n", aliased.c_str());

			next = begin;
			continue;
		}

		String alias = str.substr(begin, end-begin);
		next = end;

		const String* replace = doGet(alias);

		if (replace == NULL)
		{
			if (throwEx)
				NIT_THROW_FMT(EX_SYNTAX, "unknown alias '%s' in '%s'\n", alias.c_str(), aliased.c_str());

			next = dollar + alias.length();
		}
		else
		{
			str.replace(dollar, end+1 - dollar, *replace);
			next = dollar;
		}
	}

	return str;
}

const String& Settings::needPath()
{
	if (!_path.empty()) return _path;

	if (_parent)
	{
		_path = _parent->getPath();
		_path += "/";
		_path += _name;
	}
	else
	{
		_path = _name;
	}

	return _path;
}

void Settings::clear(bool recursive)
{
	_values.clear();

	if (recursive)
		_sections.clear();
}

Settings* Settings::load(StreamSource* source, ContentType treatAs)
{
	Ref<StreamSource> safe = source;
	Ref<StreamReader> reader;

	if (treatAs.isUnknown())
		treatAs = source->getContentType();

	String name;
	name += '[';
	name += source->getUrl();
	name += ']';

	if (treatAs == ContentType::TEXT_CFG)
	{
		reader = source->open();
		Settings* ret = new Settings(source);
		ret->_name = name;
		ret->loadCfg(reader);
		return ret;
	}

	if (treatAs == ContentType::TEXT_JSON)
	{
		reader = source->open();
		Settings* ret = new Settings(source);
		ret->_name = name;
		ret->loadJson(reader);
		return ret;
	}

	if (treatAs == ContentType::TEXT_PLIST)
	{
		reader = source->open();
		Settings* ret = new Settings(source);
		ret->_name = name;
		ret->loadPlist(reader);
		return ret;
	}

	NIT_THROW_FMT(EX_NOT_SUPPORTED, "Unknown settings format: '%s'", source->getUrl().c_str());
}

void Settings::loadCfg(StreamReader* reader)
{
	// TODO: Reimplement using ParserUtil

	Ref<StreamReader> safe = reader;

	String separators = "\t=";
	bool trimWhitespace = true;

	TBinaryReader<StreamReader> r = new MemoryBuffer::Reader(reader->buffer(), reader->getSource());

	enum CFGEncoding
	{
		UNKNOWN,
		UCS2_BE,
		UCS2_LE,
		UTF_16,

		DEFAULT = UTF_16
	};

	CFGEncoding encoding = UNKNOWN;

	// determine encoding
	uint16 us;
	uint8 uc;
	
	size_t ret = r.readRaw(&us, sizeof(us));
	if (ret == 2)
	{
		switch (us)
		{
		case 0xFFFE: encoding = UCS2_BE; break;
		case 0xFEFF: encoding = UCS2_LE; break;
		case 0xBBEF:
			r.read(&uc, sizeof(uc));
			if (uc == 0xBF) encoding = UTF_16; 
			break;
		default:
			encoding = UNKNOWN;
		}
	}

	if (encoding == UNKNOWN)
	{
		encoding = DEFAULT;
		r.getReader()->seek(0);
	}

	Settings* current = this;

	// adopted from OgreConfigFile.cpp v1.7.1
	String line, optName, optVal;
	while (!r.isEof())
	{
		line = r.readLine(false);

		if (line.length() == 0) continue;

		/* Ignore comments & blanks */
		if (line.at(0) == '#' || line.at(0) == '@' || line.at(0) == '\'')
			continue;

		size_t commentBegin = line.find("//");
		if (commentBegin == 0)
			continue;

		if (commentBegin != line.npos)
		{
			// HACK: Handle string starts with 'http://'
			if (line[commentBegin-1] != ':')
				line = line.substr(0, commentBegin);
		};

		StringUtil::trim(line);

		if (line.length() == 0) continue;

		if (line.at(0) == '[' && line.at(line.length()-1) == ']')
		{
			// Section
			String sectionName = line.substr(1, line.length() - 2);
			Settings* section = getSection(sectionName);
			if (section == NULL)
			{
				section = new Settings(_source);
				addSection(sectionName, section);
			}
			current = section;
			continue;
		}

		/* Find the first seperator character and split the string there */
		size_t separator_pos = line.find_first_of(separators, 0);
		if (separator_pos != separators.npos)
		{
			optName = line.substr(0, separator_pos);
			/* Find the first non-seperator character following the name */
			size_t nonseparator_pos = line.find_first_not_of(separators, separator_pos);
			/* ... and extract the value */
			/* Make sure we don't crash on an empty setting (it might be a valid value) */
			optVal = (nonseparator_pos == separators.npos) ? "" : line.substr(nonseparator_pos);
			if (trimWhitespace)
			{
				StringUtil::trim(optVal);
				StringUtil::trim(optName);
			}
			current->add(optName, optVal);
		}
	}
}

void Settings::loadPlist(StreamReader* reader)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

class Settings::JSONHandler : public Json::IHandler
{
public:
	JSONHandler(Settings* document, StreamSource* source)
	{
		_document = document;
		_current = document;
		_source = source;
	}

	virtual void documentBegin() { }
	virtual void documentEnd() { }

	virtual void pairObjectBegin(const String& key)
	{
		Settings* object = new Settings(_source);
		_current->addSection(key, object);
		_current = object;
	}

	virtual void pair(const String& key, const char* value)
	{
		_current->add(key, value);
	}

	virtual void pair(const String& key, int value)
	{
		_current->add(key, DataValue(value).toString());
	}

	virtual void pair(const String& key, float value)
	{
		_current->add(key, DataValue(value).toString());
	}

	virtual void pair(const String& key, bool value)
	{
		_current->add(key, DataValue(value).toString());
	}

	virtual void pairNull(const String& key)
	{
		_current->add(key, "");
	}

	virtual void pairArrayBegin(const String& key)
	{
		_arrayKeys.push_back(key);
	}

	virtual void pairArrayEnd(const String& key)
	{
		_arrayKeys.pop_back();
	}

	virtual void pairObjectEnd(const String& key)
	{
		_current = _current->getParent();
	}

	virtual void elementArrayBegin()
	{
		// do nothing
	}

	virtual void element(const char* value)
	{
		_current->add(_arrayKeys.back(), value);
	}

	virtual void element(int value)
	{
		_current->add(_arrayKeys.back(), DataValue(value).toString());
	}

	virtual void element(float value)
	{
		_current->add(_arrayKeys.back(), DataValue(value).toString());
	}

	virtual void element(bool value)
	{
		_current->add(_arrayKeys.back(), DataValue(value).toString());
	}

	virtual void elementNull()
	{
		_current->add(_arrayKeys.back(), "");
	}

	virtual void elementObjectBegin()
	{
		Settings* object = new Settings(_source);
		_current->addSection(_arrayKeys.back(), object);
		_current = object;
	}

	virtual void elementObjectEnd()
	{
		_current = _current->getParent();
	}

	virtual void elementArrayEnd()
	{
		// do nothing
	}

	Settings*							_document;
	Settings*							_current;
	Ref<StreamSource>					_source;
	StringVector						_arrayKeys;
};

void Settings::loadJson(StreamReader* reader)
{
	JSONHandler handler(this, reader->getSource());
	Json json(&handler);
	json.parse(reader);
}

void Settings::saveJson(JsonPrinter* printer)
{
	printer->beginObject();

	// TODO: convert to array for duplicated sections
	for (Values::iterator itr = _values.begin(), end = _values.end(); itr != end; ++itr)
	{
		printer->key(itr->first);
		printer->value(itr->second);
	}

	// TODO: convert to array for duplicated sections
	for (Sections::iterator itr = _sections.begin(), end = _sections.end(); itr != end; ++itr)
	{
		printer->key(itr->first);
		saveJson(printer);
	}

	printer->endObject();
}

void Settings::saveCfg(StreamWriter* writer, Settings* root, String prevPath)
{
	Ref<StreamWriter> safe = writer;

	if (root == NULL) root = this;

	if (root != this && !_values.empty())
	{
		writer->writeRaw("\n", 1);
		writer->writeRaw("[", 1);

		if (!prevPath.empty())
		{
			writer->writeRaw(prevPath.c_str(), prevPath.length());
			writer->writeRaw("/", 1);
		}

		writer->writeRaw(_name.c_str(), _name.length());
		writer->writeRaw("]", 1);
		writer->writeRaw("\n", 1);
	}

	for (Values::iterator itr = _values.begin(), end = _values.end(); itr != end; ++itr)
	{
		writer->writeRaw(itr->first.c_str(), itr->first.length());
		writer->writeRaw(" = ", 3);
		writer->writeRaw(itr->second.c_str(), itr->second.length());
		writer->writeRaw("\n", 1);
	}

	for (Sections::iterator itr = _sections.begin(), end = _sections.end(); itr != end; ++itr)
	{
		if (root != this)
		{
			if (prevPath.empty())
				itr->second->saveCfg(writer, root, _name);
			else
				itr->second->saveCfg(writer, root, prevPath + "/" + _name);
		}
		else
			itr->second->saveCfg(writer, root, prevPath);
	}
	writer->flush();
}

void Settings::dump()
{
	Ref<LogWriter> w = new LogWriter();

	String title = StringUtil::format("// %s\n", getPath().c_str());

	w->write(title.c_str(), title.length());

	saveCfg(w);

	if (_base)
	{
		title = "\n// base:\n";
		w->write(title.c_str(), title.length());
		_base->dump();
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
