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

////////////////////////////////////////////////////////////////////////////////

// Unix (mac32, ios) native C Implementation

#include <dirent.h>
#include <unistd.h>
#include <fnmatch.h>

/* Our simplified data entry structure */
struct _finddata_t
{
    char *name;
    int attrib;
    long long size;
	time_t mtime;
};

#define _A_NORMAL 0x00  /* Normalfile-Noread/writerestrictions */
#define _A_RDONLY 0x01  /* Read only file */
#define _A_HIDDEN 0x02  /* Hidden file */
#define _A_SYSTEM 0x04  /* System file */
#define _A_ARCH   0x20  /* Archive file */
#define _A_SUBDIR 0x10  /* Subdirectory */

static long _findfirst(const char *pattern, struct _finddata_t *data);
static int _findnext(long id, struct _finddata_t *data);
static int _findclose(long id);
static bool is_absolute_path(const char* path);

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>

#if !defined(NIT_ANDROID) // Android (linux) does not supports ftw()
#	include <ftw.h>
#endif

struct _find_search_t
{
    char *pattern;
    char *curfn;
    char *directory;
    int dirlen;
    DIR *dirfd;
};

static long _findfirst(const char *pattern, struct _finddata_t *data)
{
    _find_search_t *fs = new _find_search_t;
    fs->curfn = NULL;
    fs->pattern = NULL;
	
    // Separate the mask from directory name
    const char *mask = strrchr (pattern, '/');
    if (mask)
    {
        fs->dirlen = mask - pattern;
        mask++;
        fs->directory = (char *)malloc (fs->dirlen + 1);
        memcpy (fs->directory, pattern, fs->dirlen);
        fs->directory [fs->dirlen] = 0;
    }
    else
    {
        mask = pattern;
        fs->directory = strdup (".");
        fs->dirlen = 1;
    }
	
    fs->dirfd = opendir (fs->directory);
    if (!fs->dirfd)
    {
        _findclose ((long)fs);
        return -1;
    }
	
    /* Hack for "*.*" -> "*' from DOS/Windows */
    if (strcmp (mask, "*.*") == 0)
        mask += 2;
    fs->pattern = strdup (mask);
	
    /* Get the first entry */
    if (_findnext ((long)fs, data) < 0)
    {
        _findclose ((long)fs);
        return -1;
    }
	
    return (long)fs;
}

static int _findnext(long id, struct _finddata_t *data)
{
    _find_search_t *fs = (_find_search_t *)id;
	
    /* Loop until we run out of entries or find the next one */
    dirent *entry;
    for (;;)
    {
        if (!(entry = readdir (fs->dirfd)))
            return -1;
		
        /* See if the filename matches our pattern */
        if (fnmatch (fs->pattern, entry->d_name, 0) == 0)
            break;
    }
	
    if (fs->curfn)
        free (fs->curfn);
    data->name = fs->curfn = strdup (entry->d_name);
	
    size_t namelen = strlen (entry->d_name);
    char *xfn = new char [fs->dirlen + 1 + namelen + 1];
    sprintf (xfn, "%s/%s", fs->directory, entry->d_name);
	
    /* stat the file to get if it's a subdir and to find its length */
    struct stat stat_buf;
    if (stat (xfn, &stat_buf))
    {
        // Hmm strange, imitate a zero-length file then
        data->attrib = _A_NORMAL;
        data->size = 0;
    }
    else
    {
        if (S_ISDIR(stat_buf.st_mode))
            data->attrib = _A_SUBDIR;
        else
		/* Default type to a normal file */
            data->attrib = _A_NORMAL;
		
        data->size = stat_buf.st_size;
		data->mtime = stat_buf.st_mtime;
    }
	
    delete [] xfn;
	
    /* Files starting with a dot are hidden files in Unix */
    if (data->name [0] == '.')
        data->attrib |= _A_HIDDEN;
	
    return 0;
}

static int _findclose(long id)
{
    int ret;
    _find_search_t *fs = (_find_search_t *)id;
    
    ret = fs->dirfd ? closedir (fs->dirfd) : 0;
    free (fs->pattern);
    free (fs->directory);
    if (fs->curfn)
        free (fs->curfn);
    delete fs;
	
    return ret;
}

static bool is_absolute_path(const char* path)
{
	return path[0] == '/';
}

static int do_mkdir(const char* path, mode_t mode)
{
	struct stat st;
	int status = 0;
	
	if (stat(path, &st) != 0)
	{
		mode_t process_mask = umask(0);
		if (mkdir(path, mode) != 0)
			status = -1;
		umask(process_mask);
	}
	else if (!S_ISDIR(st.st_mode))
	{
		errno = ENOTDIR;
		status = -1;
	}
	
	return status;
}

static int mkpath(const char* path, mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO)
{
	char *pp;
	char *sp;
	int status;
	char *copypath = strdup(path);
	
	status = 0;
	pp = copypath;
	while (status == 0 && (sp = strchr(pp, '/')) != 0)
	{
		if (sp != pp)
		{
			// Neither root nor double slash in path
			*sp = 0;
			status = do_mkdir(copypath, mode);
			*sp = '/';
		}
		pp = sp + 1;
		if (status == 0)
			status = do_mkdir(path, mode);
		free(copypath);
		return status;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

char FileUtil::s_PathSeparator = '/';
char FileUtil::s_PathAntiSeparator = '\\';
const char* FileUtil::s_DamnedPattern1 = "/./";

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
	// NOTE: Most unix file systems are case sensitive - so be careful when develop using file system directly!
	return true;
}

bool FileUtil::isReservedName(const String& path)
{
	return path.length() < 1 || path[0] == '.';
}

bool FileUtil::isAbsolutePath(const String& path)
{
	return path.length() >= 1 && (path[0] == s_PathSeparator || path[0] == s_PathAntiSeparator);
}

bool FileUtil::exists(const String& path)
{
	struct stat tagStat;
	return stat(path.c_str(), &tagStat) == 0;
}

bool FileUtil::isDirectory(const String& path)
{
	struct stat tagStat;
	if (stat(path.c_str(), &tagStat)) return false;
	
	return (tagStat.st_mode & S_IFDIR) != 0;
}

void FileUtil::createDir(const String &path, bool throwIfExists)
{
	String fullpath = path;
	FileUtil::normalizeSeparator(fullpath);
	
	int err = mkpath(fullpath.c_str());
	
	if (err)
		NIT_THROW_FMT(EX_IO, "Can't create dir '%s': %s", fullpath.c_str(), strerror(errno));
}

static bool IsDirectory(char path[]) {
    int len = strlen(path);
    if (path[len] == '.') {return true;} // exception for directories
    // such as \. and \..
    for(int i = len - 1; i >= 0; i--) {
        if (path[i] == '.') return false; // if we first encounter a . then it's a file
        else if (path[i] == '\\' || path[i] == '/') return true; // if we first encounter a \ it's a dir
    }
	return false;
}

static bool RemoveDirectory(String path) {
    if (path[path.length()-1] != '\\') path += "\\";
    // first off, we need to create a pointer to a directory
    DIR *pdir = NULL; // remember, it's good practice to initialise a pointer to NULL!
    pdir = opendir (path.c_str());
    struct dirent *pent = NULL;
    if (pdir == NULL) { // if pdir wasn't initialised correctly
        return false; // return false to say "we couldn't do it"
    } // end if
    char file[256];
	
    int counter = 1; // use this to skip the first TWO which cause an infinite loop (and eventually, stack overflow)
    while ((pent = readdir (pdir))) { // while there is still something in the directory to list
        if (counter > 2) {
            for (int i = 0; i < 256; i++) file[i] = '\0';
            strcat(file, path.c_str());
            if (pent == NULL) { // if pent has not been initialised correctly
                return false; // we couldn't do it
            } // otherwise, it was initialised correctly, so let's delete the file~
            strcat(file, pent->d_name); // concatenate the strings to get the complete path
            if (IsDirectory(file) == true) {
                RemoveDirectory(file);
            } else { // it's a file, we can use remove
                remove(file);
            }
        } counter++;
    }
	
    // finally, let's clean up
    closedir (pdir); // close the directory
    if (!rmdir(path.c_str())) return false; // delete the directory
    return true;
}

#if !defined(NIT_ANDROID)

/* Call unlink or rmdir on the path, as appropriate. */
int rm( const char *path, const struct stat *s, int flag, struct FTW *f )
{
	int (*rm_func)( const char * );

	switch( flag ) {
		default:     rm_func = unlink; break;
		case FTW_DP: rm_func = rmdir;
	}

	return rm_func(path);
}

void FileUtil::remove(const String &pattern)
{
	String fullpath = pattern;
	normalizeSeparator(fullpath);

	int err = nftw(fullpath.c_str(), rm, OPEN_MAX, FTW_DEPTH);
	
	if (err && errno != ENOENT)
		NIT_THROW_FMT(EX_IO, "Can't remove '%s': %s", fullpath.c_str(), strerror(errno));
}

#endif // #if !defined(NIT_ANDROID)

#if defined(NIT_ANDROID)
void FileUtil::remove(const String &pattern)
{
	// linux does not support ftw() so implement a similar code using FileLocator
	// TODO: You can't delete directory using this method
	Ref<FileLocator> fl = new FileLocator("$remove", "", false);
	StreamSourceMap files;
	fl->findFiles(pattern, files);

	for (StreamSourceMap::iterator itr = files.begin(), end = files.end(); itr != end; ++itr)
	{
		const String& filename = itr->first;
		fl->remove(filename);
	}
}
#endif

void FileUtil::move(const String &from, const String &to)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void FileUtil::copy(const String &from, const String &to)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void FileUtil::writeFile(const String &path, const void *buf, size_t len)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void FileUtil::writeFile(const String &path, const String &text)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void FileUtil::writeFile(const String &path, MemoryBuffer *buffer)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void FileUtil::writeFile(const String &path, StreamReader *from, size_t bufSize)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void FileUtil::readFile(const String &path, void *buf, size_t len)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void FileUtil::readFile(const String &path, MemoryBuffer *into)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void FileUtil::readFile(const String &path, String &outText)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

void FileUtil::readFile(const String &path, StreamWriter *into, size_t bufSize)
{
	NIT_THROW(EX_NOT_IMPLEMENTED);
}

////////////////////////////////////////////////////////////////////////////////

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
		const char *rp;
		rp = realpath(_baseUrl.c_str(), buf);
		if (rp == NULL)
			NIT_THROW_FMT(EX_NOT_FOUND, "FileLocator '%s': Can't find path '%s' for \n", _name.c_str(), _baseUrl.c_str());

		_baseUrl = rp;

		FileUtil::normalizeSeparator(_baseUrl);

		// Make sure base to be 'path/' from
		if (_baseUrl[_baseUrl.length()-1] != FileUtil::getPathSeparator())
			_baseUrl += FileUtil::getPathSeparator();

		if (StringUtil::endsWith(_baseUrl, FileUtil::getDamnedPattern1()))
			_baseUrl.erase(_baseUrl.size() - FileUtil::getDamnedPattern1().length());
	}
}

void FileLocator::load()
{
	struct stat tagStat;

	if (stat(_baseUrl.c_str(), &tagStat))
	{
		NIT_THROW_FMT(EX_INVALID_PARAMS, "Invalid path : '%s'", _baseUrl.c_str());
	}
	
	if ( (tagStat.st_mode & S_IFDIR) == 0)
	{
		NIT_THROW_FMT(EX_INVALID_PARAMS, "Can't mount a file as archive: '%s'", _baseUrl.c_str());
	}

	if ( (tagStat.st_mode & S_IWUSR) != 0)
		_readOnly = false;
	else
		_readOnly = true;
}

void FileLocator::unload()
{

}

StreamSource* FileLocator::locateLocal(const String& streamName)
{
	if (_filteredOnly)
	{
		return locateFiltered(streamName);
	}

	if (FileUtil::isReservedName(streamName))
		return NULL;

	String filepath = normalizePath(streamName);

	struct stat tagStat;
	if (stat(filepath.c_str(), &tagStat)) return NULL;
	
	if ( (tagStat.st_mode & S_IFDIR) != 0) return NULL;

	if ( (tagStat.st_mode & S_IRUSR) == 0)
	{
#if defined(NIT_IOS)
		// ios: We need user read permission to access a file
		return NULL;
#else
		// Some android devices are missing IRUSR flag (ex. HTC)
		LOG(0, "?? no IRUSR for '%s'\n", filepath.c_str());
#endif
	}

	char buf[MAX_PATH];
	const char* rp = realpath(filepath.c_str(), buf);
	if (rp == NULL)
		rp = filepath.c_str(); // Some android devices are reported that they don't support realpath (ex. HTC)

	filepath = rp;
	
	assert(StringUtil::startsWith(filepath, _baseUrl, false));
	
	String name = filepath.substr(_baseUrl.length());
	String url = filepath;
	size_t streamSize = (size_t)tagStat.st_size;
	time_t timestamp = tagStat.st_mtime;
	
	File* file = new File(this, name);
	file->_streamSize = streamSize;
	file->_timestamp = timestamp;
	
	return file;
}

void FileLocator::findFiles(const String& pattern, StreamSourceMap& varResults, bool recursive)
{
	long lHandle, res;
	struct _finddata_t tagData;
	bool dirs = false;
	bool ignoreHidden = true;
	
	// pattern can contain a directory name, separate it from mask	
	size_t pos1;
	String directory = extractFirstDirectory(pattern, &pos1);
	String full_pattern = normalizePath(pattern);
	
	lHandle = _findfirst(full_pattern.c_str(), &tagData);
	res = 0;
	while (lHandle != -1 && res != -1)
	{
		if ((dirs == ((tagData.attrib & _A_SUBDIR) != 0)) &&
			( !ignoreHidden || (tagData.attrib & _A_HIDDEN) == 0 ) &&
			(!dirs || !FileUtil::isReservedName(tagData.name)))
		{
			String name = directory + tagData.name;
			if (varResults.find(name) == varResults.end())
			{
				size_t streamSize = (size_t)tagData.size;
				time_t timestamp = tagData.mtime;

				File* file = new File(this, name);
				file->_streamSize = streamSize;
				file->_timestamp = timestamp;

				varResults.insert(std::make_pair(name, file));
			}
		}
		res = _findnext( lHandle, &tagData );
	}
	
	// Close if we found any files
	if(lHandle != -1)
		_findclose(lHandle);
	
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
	
	lHandle = _findfirst(base_dir.c_str(), &tagData);
	res = 0;
	while (lHandle != -1 && res != -1)
	{
		if ((tagData.attrib & _A_SUBDIR) &&
			( !ignoreHidden || (tagData.attrib & _A_HIDDEN) == 0 ) &&
			!FileUtil::isReservedName(tagData.name))
		{
			// recurse
			base_dir = directory;
			base_dir.append(tagData.name).append(mask);
			findFiles(base_dir, varResults, recursive);
		}
		res = _findnext( lHandle, &tagData );
	}
	// Close if we found any files
	if(lHandle != -1)
		_findclose(lHandle);
}

void FileLocator::findDirs(const String& pattern, StringVector& varResults)
{
	long lHandle, res;
	struct _finddata_t tagData;
	bool dirs = true;
	bool ignoreHidden = true;
	
	// pattern can contain a directory name, separate it from mask	
	size_t pos1;
	String directory = extractFirstDirectory(pattern, &pos1);
	String full_pattern = normalizePath(pattern);
	
	lHandle = _findfirst(full_pattern.c_str(), &tagData);
	res = 0;
	while (lHandle != -1 && res != -1)
	{
		if ((dirs == ((tagData.attrib & _A_SUBDIR) != 0)) &&
			( !ignoreHidden || (tagData.attrib & _A_HIDDEN) == 0 ) &&
			(dirs || !FileUtil::isReservedName(tagData.name)))
		{
			String dirname = tagData.name;
			varResults.push_back(dirname);
		}
		res = _findnext( lHandle, &tagData );
	}
	
	// Close if we found any files
	if(lHandle != -1)
		_findclose(lHandle);
}

StreamWriter* FileLocator::create(const String& streamName)
{
	String filepath = normalizePath(streamName);

	if (isReadOnly())
		NIT_THROW_FMT(EX_ACCESS, "Can't create '%s' on a readonly archive", filepath.c_str());

	NIT_FILE_HANDLE fileHandle = fopen(filepath.c_str(), "w");
	
	if (fileHandle == NULL)
		NIT_THROW_FMT(EX_IO, "Can't create '%s': %s", filepath.c_str(), strerror(errno));
	
	return new FileWriter(NULL, fileHandle);
}

StreamWriter* FileLocator::modify(const String& streamName)
{
	String filepath = normalizePath(streamName);

	if (isReadOnly())
		NIT_THROW_FMT(EX_ACCESS, "Can't modify '%s' on a readonly archive '", filepath.c_str());

	NIT_FILE_HANDLE fileHandle = fopen(filepath.c_str(), "r+");
	
	if (fileHandle == NULL)
		NIT_THROW_FMT(EX_IO, "Can't modify '%s': %s", filepath.c_str(), strerror(errno));
	
	return new FileWriter(NULL, fileHandle);
}

void FileLocator::remove(const String& streamName)
{
	String filepath = normalizePath(streamName);

	if (isReadOnly())
		NIT_THROW_FMT(EX_ACCESS, "Can't remove '%s' from a readonly archive", filepath.c_str());

	int err = ::unlink(filepath.c_str());
	if (err && err != ENOENT)
		NIT_THROW_FMT(EX_IO, "Can't remove '%s': %s", filepath.c_str(), strerror(errno));
}

void FileLocator::rename(const String& streamName, const String& newName)
{
	String filepath = normalizePath(streamName);

	if (isReadOnly())
		NIT_THROW_FMT(EX_ACCESS, "Can't rename '%s' on a readonly archive", filepath.c_str());

	String newpath = normalizePath(newName);

	if (::rename(filepath.c_str(), newpath.c_str()))
		NIT_THROW_FMT(EX_IO, "Can't rename '%s' to '%s': %s", filepath.c_str(), filepath.c_str(), strerror(errno));
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
	String filepath = getRealLocator()->makeUrl(_name); // We need an actual filepath not a proxied one

	NIT_FILE_HANDLE fileHandle = fopen(filepath.c_str(), "r");

	if (fileHandle == NULL)
		NIT_THROW_FMT(EX_IO, "Can't open '%s': %s", filepath.c_str(), strerror(errno));

	if (outOffset)
		*outOffset = 0;

	if (outSize)
	{
		// TODO: Find a more nit way
		fseek(fileHandle, 0, SEEK_END);
		*outSize = ftell(fileHandle);
		fseek(fileHandle, 0, SEEK_SET);
	}

	return fileHandle;
}

////////////////////////////////////////////////////////////////////////////////

FileReader::FileReader(StreamSource* source, NIT_FILE_HANDLE fileHandle, size_t offset, size_t* inSize)
{
	_source = source;
	_file = fileHandle;
	_offset = offset;
	_pos = 0;

	if (inSize)
	{
		_size = *inSize;
		if (offset)
		{
			_pos = -1;
			seek(0);
		}
	}
	else
	{
		// Determine size
		// TODO: Find a more nit way
		fseek(fileHandle, 0, SEEK_END);
		size_t sz = ftell(fileHandle);

		ASSERT_THROW(sz >= offset, EX_IO);
		_size = sz - offset;
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

	if (fseek(_file, _offset + _pos, SEEK_SET))
		NIT_THROW(EX_IO);
}

void FileReader::seek(size_t pos)
{
	if (_pos == pos) return;

	_pos = pos;
	
	if (fseek(_file, _offset + pos, SEEK_SET))
		NIT_THROW(EX_IO);
}

size_t FileReader::readRaw(void* buf, size_t size)
{
	if (_pos + size >= _size)
		size = _size - _pos;

	if (size > 0)
	{
		size_t bytesRead = fread(buf, 1, size, _file);

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
	FILE* file = _file;
	_file = NULL;
	return file;
}

void FileReader::close()
{
	if (_file != NULL)
		fclose(_file);

	_file = NULL;
}

////////////////////////////////////////////////////////////////////////////////

FileWriter::FileWriter(StreamSource* source, NIT_FILE_HANDLE fileHandle, size_t offset)
{
	_source = source;
	_file = fileHandle;
	_offset = offset;

	if (offset)
		seek(0);
}

size_t FileWriter::getSize()
{
	size_t pos = ftell(_file);
	fseek(_file, 0, SEEK_END);
	size_t sz = ftell(_file);
	fseek(_file, pos, SEEK_SET);
	return sz - _offset;
}

void FileWriter::skip(int count)
{
	if (fseek(_file, count, SEEK_CUR))
		NIT_THROW(EX_IO);
}

void FileWriter::seek(size_t pos)
{
	if (fseek(_file, _offset + pos, SEEK_CUR))
		NIT_THROW(EX_IO);
}

size_t FileWriter::tell()
{
	long pos = ftell(_file);
	if (pos == -1)
		NIT_THROW(EX_IO);
	
	return pos - _offset;
}

size_t FileWriter::writeRaw(const void* buf, size_t size)
{
	size_t bytesWritten = fwrite(buf, 1, size, _file);
	return bytesWritten;
}

bool FileWriter::flush()
{
	return fflush(_file) == 0;
}

void FileWriter::onDelete()
{
	close();
}

NIT_FILE_HANDLE FileWriter::releaseHandle()
{
	FILE* file = _file;
	_file = NULL;
	return file;
}

void FileWriter::close()
{
	if (_file != NULL)
		fclose(_file);

	_file = NULL;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
