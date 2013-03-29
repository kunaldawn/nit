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

#include "nit/io/FileLocator.h"
#include "nit/io/MemoryBuffer.h"

#include "shlobj.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

static void TimetToFileTime( time_t t, LPFILETIME pft )
{
	LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD) ll;
	pft->dwHighDateTime = ll >>32;
}

static const __int64 SECS_BETWEEN_EPOCHS = 11644473600;
static const __int64 SECS_TO_100NS = 10000000; /* 10^7 */

static time_t FileTime2time_t(FILETIME FileTime, long* nsec = NULL)
{
	__int64 UnixTime;

	/* get the full win32 value, in 100ns */
	UnixTime = ((__int64)FileTime.dwHighDateTime << 32) + FileTime.dwLowDateTime;

	/* convert to the Unix epoch */
	UnixTime = UnixTime - (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);

	if ( nsec )
	{
		/* get the number of 100ns, convert to ns */
		*nsec = (UnixTime % SECS_TO_100NS) * 100;
	}

	UnixTime /= SECS_TO_100NS; /* now convert to seconds */

	return (time_t)UnixTime;
}

static HANDLE _OpenFile(const String& path)
{
	String fullpath = path;
	FileUtil::normalizeSeparator(fullpath);

	DWORD accessMode = GENERIC_READ;
	DWORD shareMode = FILE_SHARE_READ;

	HANDLE fileHandle = CreateFileW(
		Unicode::toUtf16(fullpath).c_str(),
		accessMode,
		shareMode,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (fileHandle == INVALID_HANDLE_VALUE)
		NIT_THROW_FMT(EX_IO, "Can't open '%s'", fullpath.c_str());

	return fileHandle;
}

static HANDLE _CreateFile(const String& path)
{
	String fullpath = path;
	FileUtil::normalizeSeparator(fullpath);

	DWORD accessMode = GENERIC_READ | GENERIC_WRITE;
	DWORD shareMode = FILE_SHARE_READ;

	HANDLE fileHandle = CreateFileW(
		Unicode::toUtf16(fullpath).c_str(),
		accessMode,
		shareMode,
		NULL,
		CREATE_ALWAYS,
		0,
		NULL);

	if (fileHandle == INVALID_HANDLE_VALUE)
		NIT_THROW_FMT(EX_IO, "Can't create '%s'", fullpath.c_str());

	return fileHandle;
}

////////////////////////////////////////////////////////////////////////////////

char FileUtil::s_PathSeparator = '\\';
char FileUtil::s_PathAntiSeparator = '/';
const char* FileUtil::s_DamnedPattern1 = "\\.\\";

bool FileUtil::hasFileSystem()
{
	return true;
}

bool FileUtil::isReadOnly()
{
	return false;
}

bool FileUtil::isCaseSensitive()
{
	return false;
}

bool FileUtil::isReservedName(const String& path)
{
	return path.length() < 1 || path[0] == '.';
}

bool FileUtil::isAbsolutePath(const String& path)
{
	if (path.length() >= 1 && path[0] == s_PathSeparator || path[0] == s_PathAntiSeparator)
		return true;

	if (path.length() > 2 && path[1] == ':')
		return true;

	return false;
}

bool FileUtil::exists(const String& path)
{
	DWORD attr = GetFileAttributesW(Unicode::toUtf16(path).c_str());

	if (attr == -1) return false;

	if ( (attr & FILE_ATTRIBUTE_DEVICE) ) return false;

	return true;
}

bool FileUtil::isDirectory(const String& path)
{
	DWORD attr = GetFileAttributesW(Unicode::toUtf16(path).c_str());

	if (attr == -1) return false;

	if ( (attr & FILE_ATTRIBUTE_DEVICE) ) return false;

	return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

void FileUtil::createDir(const String& path, bool throwIfExists)
{
	String fullpath = path;
	FileUtil::normalizeSeparator(fullpath);

	HRESULT hr = SHCreateDirectoryExW(NULL, Unicode::toUtf16(fullpath).c_str(), NULL);

	if (throwIfExists)
	{
		if (hr == ERROR_ALREADY_EXISTS || hr == ERROR_FILE_EXISTS)
			NIT_THROW_FMT(EX_DUPLICATED, "Can't create dir '%s': already exists", fullpath.c_str());
	}

	if (FAILED(hr))
		NIT_THROW_FMT(EX_IO, "Can't create dir '%s'", fullpath.c_str());
}

static int _FileOperation(const UniChar* from, const UniChar* to, int op, bool useRecycleBin)
{
	SHFILEOPSTRUCTW fileop;
	fileop.hwnd		= NULL;				// no status display
	fileop.wFunc	= op;
	fileop.pFrom	= from;
	fileop.pTo		= to;
	fileop.fFlags	= FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_SILENT; // do not prompt the user

	fileop.fAnyOperationsAborted	= FALSE;
	fileop.lpszProgressTitle		= NULL;
	fileop.hNameMappings			= NULL;

	int err = SHFileOperation(&fileop);

	return err;
}

void FileUtil::remove(const String& pattern)
{
	// win32 supports wildcard at last path component
	String fullpath = pattern;
	FileUtil::normalizeSeparator(fullpath);

	UniString uniPath = Unicode::toUtf16(fullpath);
	uniPath.push_back(0); // double null terminated

	int err = _FileOperation(uniPath.c_str(), NULL, FO_DELETE, false);

	if (err && err != ERROR_FILE_NOT_FOUND)
		NIT_THROW_FMT(EX_IO, "can't remove '%s': error code 0x%X", pattern.c_str(), err);
}

void FileUtil::move(const String& from, const String& to)
{
	String fullfrom = from;
	String fullto = to;

	FileUtil::normalizeSeparator(fullfrom);
	FileUtil::normalizeSeparator(fullto);

	UniString uniFrom = Unicode::toUtf16(from);
	uniFrom.push_back(0); // double null terminated

	UniString uniTo = Unicode::toUtf16(to);
	uniTo.push_back(0); // double null terminated

	int err = _FileOperation(uniFrom.c_str(), uniTo.c_str(), FO_MOVE, false);

	if (err)
		NIT_THROW_FMT(EX_IO, "can't move '%s' to '%s': error code 0x%X", from.c_str(), to.c_str(), err);
}

void FileUtil::copy(const String& from, const String& to)
{
	String fullfrom = from;
	String fullto = to;

	FileUtil::normalizeSeparator(fullfrom);
	FileUtil::normalizeSeparator(fullto);

	UniString wFrom = Unicode::toUtf16(from);
	wFrom.push_back(0); // double null terminated

	UniString wTo = Unicode::toUtf16(to);
	wTo.push_back(0); // double null terminated

	int err = _FileOperation(wFrom.c_str(), wTo.c_str(), FO_COPY, false);

	if (err)
		NIT_THROW_FMT(EX_IO, "can't copy '%s' to '%s': error code 0x%X", from.c_str(), to.c_str(), err);
}

void FileUtil::writeFile(const String& path, const void* buf, size_t len)
{
	HANDLE fileHandle = _CreateFile(path);

	DWORD bytesWritten = 0;
	BOOL ok = ::WriteFile(fileHandle, buf, len, &bytesWritten, NULL);

	if (!ok || bytesWritten != len)
	{
		CloseHandle(fileHandle);
		NIT_THROW_FMT(EX_WRITE, "Can't write '%s'", path.c_str());
	}

	CloseHandle(fileHandle);
}

void FileUtil::writeFile(const String &path, const String &text)
{
	writeFile(path, text.c_str(), text.length());
}

void FileUtil::writeFile(const String& path, MemoryBuffer* buffer)
{
	HANDLE fileHandle = _CreateFile(path);

	for (uint i=0; i < buffer->getNumBlocks(); ++i)
	{
		uint8* buf;
		size_t size;

		if (buffer->getBlock(i, buf, size))
		{
			DWORD bytesWritten = 0;
			BOOL ok = ::WriteFile(fileHandle, buf, size, &bytesWritten, NULL);

			if (!ok || bytesWritten != size)
			{
				CloseHandle(fileHandle);
				NIT_THROW_FMT(EX_WRITE, "Can't write '%s'", path.c_str());
			}
		}
	}

	CloseHandle(fileHandle);
}

void FileUtil::writeFile(const String& path, StreamReader* from, size_t bufSize)
{
	HANDLE fileHandle = _CreateFile(path);

	Ref<FileWriter> fw = new FileWriter(NULL, fileHandle);

	fw->copy(from, 0, 0, bufSize);
}

void FileUtil::readFile(const String& path, void* buf, size_t len)
{
	HANDLE fileHandle = _OpenFile(path);

	DWORD bytesRead = 0;
	BOOL ok = ::ReadFile(fileHandle, buf, len, &bytesRead, NULL);

	if (!ok || bytesRead != len)
	{
		CloseHandle(fileHandle);
		NIT_THROW_FMT(EX_WRITE, "Can't read '%s'", path.c_str());
	}

	CloseHandle(fileHandle);

}

void FileUtil::readFile(const String& path, MemoryBuffer* into)
{
	HANDLE fileHandle = _OpenFile(path);

	Ref<FileReader> fr = new FileReader(NULL, fileHandle);

	into->load(fr);
}

void FileUtil::readFile(const String& path, String& outText)
{
	HANDLE fileHandle = _OpenFile(path);

	char buf[4096];
	size_t bufSize = sizeof(buf);

	outText.reserve(GetFileSize(fileHandle, NULL));

	while (true)
	{
		DWORD bytesRead;
		BOOL ok = ::ReadFile(fileHandle, buf, sizeof(buf), &bytesRead, NULL);

		if (!ok)
		{
			CloseHandle(fileHandle);
			NIT_THROW_FMT(EX_READ, "Can't read '%s'", path.c_str());
		}

		if (bytesRead)
			outText.append(buf, bytesRead);

		if (bytesRead < bufSize)
			break;
	}

	CloseHandle(fileHandle);
}

void FileUtil::readFile(const String& path, StreamWriter* into, size_t bufSize)
{
	HANDLE fileHandle = _OpenFile(path);

	Ref<FileReader> fr = new FileReader(NULL, fileHandle);

	into->copy(fr, 0, 0, bufSize);
}

////////////////////////////////////////////////////////////////////////////////

// Win32 Implementation

FileLocator::FileLocator(const String& name, const String& path, bool readOnly, bool findRecursive)
: Archive(name), _readOnly(readOnly), _findRecursive(findRecursive)
{
	init(path);
}

FileLocator::FileLocator(const String& name, const char* path, bool readOnly, bool findRecursive)
: Archive(name), _readOnly(readOnly), _findRecursive(findRecursive)
{
	init(path);
}

FileLocator::FileLocator(const String& path, bool readOnly, bool findRecursive)
: Archive(path), _readOnly(readOnly), _findRecursive(findRecursive)
{
	init(path);

	_name = _baseUrl;
}

void FileLocator::init(const String& path)
{
	_baseUrl = path;
	_filteredOnly = false;

	if (!_baseUrl.empty())
	{
		char buf[MAX_PATH];
		_baseUrl = _fullpath(buf, _baseUrl.c_str(), sizeof(buf));

		FileUtil::normalizeSeparator(_baseUrl);

		// Make sure base to be 'path/' from
		if (_baseUrl[_baseUrl.length()-1] != FileUtil::getPathSeparator())
			_baseUrl.push_back(FileUtil::getPathSeparator());

		if (StringUtil::endsWith(_baseUrl, FileUtil::getDamnedPattern1()))
			_baseUrl.erase(_baseUrl.size() - FileUtil::getDamnedPattern1().length());
	}
	load();
}

void FileLocator::load()
{
	if (!_baseUrl.empty())
	{
		DWORD attr = GetFileAttributesA(_baseUrl.c_str());

		if (attr == -1)
		{
			NIT_THROW_FMT(EX_INVALID_PARAMS, "Invalid path : '%s'", _baseUrl.c_str());
		}

		if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			NIT_THROW_FMT(EX_INVALID_PARAMS, "Can't mount a file as archive: '%s'", _baseUrl.c_str());
		}

		if (attr & FILE_ATTRIBUTE_READONLY)
			_readOnly = true;
		else
			_readOnly = false;
	}
}

void FileLocator::unload()
{

}

StreamSource* FileLocator::locateLocal(const String& streamName)
{
	bool showHidden = false;

	if (_filteredOnly)
	{
		return locateFiltered(streamName);
	}

	String directory = extractFirstDirectory(streamName);
	String fullname = normalizePath(streamName);

// 	LOG(0, ".. FileLocator::LocateLocal: %s\n", fullname.c_str());

	WIN32_FIND_DATAA ff;
	HANDLE hFind = FindFirstFileA(fullname.c_str(), &ff);

	if (hFind == INVALID_HANDLE_VALUE)
		return NULL;

	StreamSource* ret = NULL;

	bool skip = false;
	skip = skip || (!showHidden && (ff.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN));
	skip = skip || (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	skip = skip || FileUtil::isReservedName(ff.cFileName);

	if (!skip)
	{
		String name = directory + ff.cFileName;
		String url = normalizePath(name);
		size_t streamSize = ff.nFileSizeLow;
		size_t memorySize = ff.nFileSizeLow;
		time_t timestamp = FileTime2time_t(ff.ftLastWriteTime);

		File* file = new File(this, name);
		file->_streamSize = streamSize;
		file->_timestamp = timestamp;

		ret = file;
	}

	FindClose(hFind);

	return ret;
}

// TODO: unify find algorithm with Settings and PackArchive
void FileLocator::findFiles(const String& pattern, StreamSourceMap& varResults, bool recursive /*= false*/)
{
	bool showHidden = false;

	size_t pos1;
	String directory = extractFirstDirectory(pattern, &pos1);
	String full_pattern = normalizePath(pattern);

	WIN32_FIND_DATAW ff;
	HANDLE hFind = FindFirstFileW(Unicode::toUtf16(full_pattern).c_str(), &ff);

	for (BOOL ok = hFind != INVALID_HANDLE_VALUE; ok; ok = FindNextFileW(hFind, &ff))
	{
		if (!showHidden && (ff.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			continue;

		if (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		String filename = Unicode::toUtf8(ff.cFileName);

		if (FileUtil::isReservedName(filename))
			continue;

		String name = directory + filename;

		if (varResults.find(name) != varResults.end())
			continue;

		size_t streamSize = ff.nFileSizeLow;
		size_t memorySize = ff.nFileSizeLow;
		time_t timestamp = FileTime2time_t(ff.ftLastWriteTime);

		File* file = new File(this, name);
		file->_streamSize = streamSize;
		file->_timestamp = timestamp;

		varResults.insert(std::make_pair(name, file));
	}

	// Close if we found any files
	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);

	if (!recursive) return;

	// Now find directories
	String base_dir = _baseUrl;
	if (!directory.empty())
	{
		base_dir = normalizePath(directory);
	}
	// append '/*'
	base_dir.push_back(FileUtil::getPathSeparator());
	base_dir.push_back('*');

	// Remove directory name from pattern
	String mask("/");
	if (pos1 != pattern.npos)
		mask.append(pattern.substr(pos1 + 1));
	else
		mask.append(pattern);

	hFind = FindFirstFileW(Unicode::toUtf16(base_dir).c_str(), &ff);

	for (BOOL ok = hFind != INVALID_HANDLE_VALUE; ok; ok = FindNextFileW(hFind, &ff))
	{
		if (!(ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;
		
		if (!showHidden && (ff.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) )
			continue;

		String filename = Unicode::toUtf8(ff.cFileName);
		if (FileUtil::isReservedName(filename))
			continue;

		base_dir = directory;
		base_dir.append(filename).append(mask);
		findFiles(base_dir, varResults, recursive);
	}

	// Close if we found any files
	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);
}

void FileLocator::findDirs(const String& pattern, StringVector& varResults)
{
	bool showHidden = false;

	WIN32_FIND_DATAW ff;
	HANDLE hFind = FindFirstFileW(Unicode::toUtf16(pattern).c_str(), &ff);

	for (BOOL ok = hFind != INVALID_HANDLE_VALUE; ok; ok = FindNextFileW(hFind, &ff))
	{
		if (!showHidden && (ff.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			continue;

		if (!(ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		String dirname = Unicode::toUtf8(ff.cFileName);

		if (FileUtil::isReservedName(dirname))
			continue;

		varResults.push_back(dirname);
	}

	// Close if we found any files
	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);
}

StreamWriter* FileLocator::create(const String& streamName)
{
	String fullpath = normalizePath(streamName);

	if (isReadOnly())
		NIT_THROW_FMT(EX_ACCESS, "Can't create '%s' on a readonly archive", fullpath.c_str());

	String filename;
	String path;

	StringUtil::splitFilename(fullpath, filename, path);

	FileUtil::createDir(path);

	DWORD accessMode = GENERIC_READ | GENERIC_WRITE;
	DWORD shareMode = FILE_SHARE_READ;

	HANDLE fileHandle = CreateFileW(
		Unicode::toUtf16(fullpath).c_str(),
		accessMode,
		shareMode,
		NULL,
		CREATE_ALWAYS,
		0,
		NULL);

	if (fileHandle == INVALID_HANDLE_VALUE)
		NIT_THROW_FMT(EX_IO, "Can't create '%s'", fullpath.c_str());

	return new FileWriter(NULL, fileHandle);
}

StreamWriter* FileLocator::modify(const String& streamName)
{
	String fullpath = normalizePath(streamName);

	if (isReadOnly())
		NIT_THROW_FMT(EX_ACCESS, "Can't modify '%s' on a readonly archive '", fullpath.c_str());

	DWORD accessMode = GENERIC_READ | GENERIC_WRITE;
	DWORD shareMode = FILE_SHARE_READ;

	HANDLE fileHandle = CreateFileW(
		Unicode::toUtf16(fullpath).c_str(),
		accessMode,
		shareMode,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (fileHandle == INVALID_HANDLE_VALUE)
		NIT_THROW_FMT(EX_IO, "Can't modify '%s'", fullpath.c_str());

	return new FileWriter(NULL, fileHandle);
}

void FileLocator::remove(const String& streamName)
{
	String fullpath = normalizePath(streamName);

	if (isReadOnly())
		NIT_THROW_FMT(EX_ACCESS, "Can't remove '%s' from a readonly archive", fullpath.c_str());

	BOOL ok = DeleteFileW(Unicode::toUtf16(fullpath).c_str());

	if (!ok && GetLastError() != ERROR_FILE_NOT_FOUND)
	{
		NIT_THROW_FMT(EX_IO, "Can't remove '%s'", fullpath.c_str());
	}
}

void FileLocator::rename(const String& streamName, const String& newName)
{
	String fullpath = normalizePath(streamName);

	if (isReadOnly())
		NIT_THROW_FMT(EX_ACCESS, "Can't rename '%s' on a readonly archive", fullpath.c_str());

	String newfullpath = normalizePath(newName);

	BOOL ok = MoveFileW(Unicode::toUtf16(fullpath).c_str(), Unicode::toUtf16(newfullpath).c_str());

	if (!ok)
	{
		NIT_THROW_FMT(EX_IO, "Can't rename '%s' to '%s'", fullpath.c_str(), newfullpath.c_str());
	}
}

////////////////////////////////////////////////////////////////////////////////

File::File(StreamLocator* locator, const String& name)
: StreamSource(locator, name)
{

}

StreamReader* File::open()
{
	return new FileReader(this, openHandle());
}

StreamReader* File::openRange(size_t offset, size_t size, StreamSource* source)
{
	if (source == NULL)
		source = this;

	return new FileReader(source, openHandle(), offset, &size);
}

NIT_FILE_HANDLE File::openHandle(size_t* outOffset, size_t* outSize)
{
	DWORD accessMode = GENERIC_READ;
	DWORD shareMode = FILE_SHARE_READ;

	String filepath = getRealLocator()->makeUrl(_name); // We need an actual filepath not a proxied one

	HANDLE fileHandle = CreateFileW(
		Unicode::toUtf16(filepath).c_str(),
		accessMode,
		shareMode,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (fileHandle == INVALID_HANDLE_VALUE)
		NIT_THROW_FMT(EX_IO, "Can't open '%s'", filepath.c_str());

	if (outOffset)
		*outOffset = 0;

	if (outSize)
		*outSize = GetFileSize(fileHandle, NULL);

	return fileHandle;
}

////////////////////////////////////////////////////////////////////////////////

FileReader::FileReader(StreamSource* source, HANDLE fileHandle, size_t offset, size_t* inSize)
{
	_source = source;
	_file = fileHandle;
	_offset = offset;
	_pos = 0;

	if (inSize)
	{
		_size = *inSize;
	}
	else
	{
		DWORD sz = GetFileSize(_file, NULL);
		ASSERT_THROW(sz >= offset, EX_IO);
		_size = sz - offset;
	}

	if (offset)
	{
		_pos = -1;
		seek(0);
	}
}

void FileReader::skip(int count)
{
	if (count == 0) return;

	ASSERT_THROW(!(count < 0 && _pos < (uint)-count), EX_IO);
	_pos += count;
	ASSERT_THROW(!(count > 0 && _pos > _size), EX_IO);

	LONG newPos = SetFilePointer(_file, count, NULL, FILE_CURRENT);

	ASSERT_THROW(newPos != INVALID_SET_FILE_POINTER, EX_IO);
}

void FileReader::seek(size_t pos)
{
	_pos = pos;
	LONG newPos = SetFilePointer(_file, _offset + pos, NULL, FILE_BEGIN);

	ASSERT_THROW(newPos != INVALID_SET_FILE_POINTER, EX_IO);
}

size_t FileReader::readRaw(void* buf, size_t size)
{
	if (_pos + size >= _size)
		size = _size - _pos;

	if (size > 0)
	{
		DWORD bytesRead = 0;
		BOOL ret = ReadFile(_file, buf, size, &bytesRead, NULL);

		if (bytesRead > 0)
			_pos += bytesRead;

		return bytesRead;
	}

	return size;
}

void FileReader::onDelete()
{
	close();
}

NIT_FILE_HANDLE FileReader::releaseHandle()
{
	HANDLE h = _file;
	_file = INVALID_HANDLE_VALUE;
	return h;
}

void FileReader::close()
{
	if (_file != INVALID_HANDLE_VALUE)
		CloseHandle(_file);

	_file = INVALID_HANDLE_VALUE;
}

////////////////////////////////////////////////////////////////////////////////

FileWriter::FileWriter(StreamSource* source, HANDLE fileHandle, size_t offset)
{
	_source = source;
	_file = fileHandle;
	_offset = offset;

	if (offset)
		seek(0);
}

size_t FileWriter::getSize()
{
	return GetFileSize(_file, NULL) - _offset;
}

void FileWriter::skip(int count)
{
	LONG newPos = SetFilePointer(_file, count, NULL, FILE_CURRENT);

	ASSERT_THROW(newPos != INVALID_SET_FILE_POINTER, EX_IO);
}

void FileWriter::seek(size_t pos)
{
	LONG newPos = SetFilePointer(_file, _offset + pos, NULL, FILE_BEGIN);

	ASSERT_THROW(newPos != INVALID_SET_FILE_POINTER, EX_IO);
}

size_t FileWriter::tell()
{
	LONG pos = SetFilePointer(_file, 0, NULL, FILE_CURRENT);

	ASSERT_THROW(pos != INVALID_SET_FILE_POINTER, EX_IO);

	return pos - _offset;
}

size_t FileWriter::writeRaw(const void* buf, size_t size)
{
	DWORD bytesWritten = 0;
	BOOL ret = WriteFile(_file, buf, size, &bytesWritten, NULL);

	return bytesWritten;
}

bool FileWriter::flush()
{
	return FlushFileBuffers(_file) != 0;
}

void FileWriter::onDelete()
{
	close();
}

NIT_FILE_HANDLE FileWriter::releaseHandle()
{
	HANDLE h = _file;
	_file = INVALID_HANDLE_VALUE;
	return h;
}

void FileWriter::close()
{
	if (_file != INVALID_HANDLE_VALUE)
		CloseHandle(_file);

	_file = INVALID_HANDLE_VALUE;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
