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

#include "nitnet_pch.h"

#include "nitnet/HttpRequest.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class HttpRequest::MultipartForm : public PooledAlloc
{
public:
	MultipartForm()
	{
		_post = NULL;
		_last = NULL;
	}

	~MultipartForm()
	{
		if (_post)
			curl_formfree(_post);
	}

	struct curl_httppost*				_post;
	struct curl_httppost*				_last;

	struct StreamEntry
	{
		HttpRequest*					request;
		Ref<StreamReader>				reader;
	};

	list<String>::type					_strings;
	list<StreamEntry>::type				_streamList;

	const char* remember(const String& str)
	{
		if (str.empty()) return NULL;

		_strings.push_back(str);
		return _strings.back().c_str();
	}

	StreamEntry* remember(HttpRequest* req, StreamReader* reader)
	{
		if (reader == NULL) return NULL;

		Ref<StreamReader> safe = reader;

		if (!reader->isSized())
			reader = new MemoryBuffer::Reader(reader->buffer(), reader->getSource());

		_streamList.push_back(StreamEntry());

		StreamEntry* e = &_streamList.back();
		e->request = req;
		e->reader = reader;

		return e;
	}
};

////////////////////////////////////////////////////////////////////////////////

HttpRequest::HttpRequest(const String& url, bool encoded, bool setFields)
{
	_url = url;

	size_t split = url.find_first_of('?');
	if (split != url.npos && setFields)
	{
		_baseURL = url.substr(0, split);
		_fields = url.substr(split + 1);

		if (!encoded)
		{
			_baseURL = urlEncode(_url.c_str(), _url.length());
			_fields = urlEncode(_fields.c_str(), _fields.length(), true);
		}
	}
	else
	{
		_baseURL = encoded ? url : urlEncode(url.c_str(), url.length());
	}

	_requestType = HTTP_GET;
	_multipartForm = NULL;
	_headerSList = NULL;

	_downloadProgress = 0.0f;
	_uploadProgress = 0.0f;
}

const char* HttpRequest::getTypeName()
{
	switch (_requestType)
	{
	case HTTP_GET:						return "HttpGet";
	case HTTP_POST:						return "HttpPost";
	case HTTP_MULTIPART:				return "HttpMultipart";
	default:							assert(0); return "HttpUnknown";
	}
}

void HttpRequest::setBaseUrl(const String& baseURL, bool encoded)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());
	_baseURL = encoded ? baseURL : urlEncode(baseURL.c_str(), baseURL.length());
}

bool HttpRequest::get(StreamWriter* downloadWriter)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());

	if (_requestType == HTTP_MULTIPART)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' can't get with multipart", getTypeName(), _url.c_str());

	_requestType = HTTP_GET;

	_response = NULL;

	if (downloadWriter)
	{
		_response = new HttpResponse();
		_response->_downloadWriter = downloadWriter;
	}

	return doRequest();
}

bool HttpRequest::post(StreamWriter* downloadWriter)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());

	if (_requestType == HTTP_GET)
		_requestType = HTTP_POST;

	_response = NULL;

	if (downloadWriter)
	{
		_response = new HttpResponse();
		_response->_downloadWriter = downloadWriter;
	}

	return doRequest();
}

void HttpRequest::addFields(const String& fieldsStr, bool encoded)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());

	if (!_fields.empty())
		_fields.push_back('&');

	_fields.append(encoded ? fieldsStr : urlEncode(fieldsStr.c_str(), fieldsStr.length(), true));
}

void HttpRequest::addFields(const StringTable& fields, bool encoded)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());

	for (StringTable::const_iterator itr = fields.begin(), end = fields.end(); itr != end; ++itr)
	{
		String key = encoded ? itr->first : urlEncode(itr->first.c_str(), itr->first.length());
		String value = encoded ? itr->first : urlEncode(itr->second.c_str(), itr->second.length());

		if (!_fields.empty())
			_fields.push_back('&');

		_fields.append(key);
		_fields.push_back('=');
		_fields.append(value);
	}
}

void HttpRequest::addFields(HSQUIRRELVM v, SQInteger tableIdx, bool encoded)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());

	for (NitIterator itr(v, tableIdx); itr.hasNext(); itr.next())
	{
		sq_tostring(v, itr.keyIndex());
		String key = encoded ? sqx_getstring(v, -1) : urlEncode(sqx_getstring(v, -1));
		sq_poptop(v);

		sq_tostring(v, itr.valueIndex());
		String value = encoded ? sqx_getstring(v, -1) : urlEncode(sqx_getstring(v, -1));
		sq_poptop(v);

		if (!_fields.empty())
			_fields.push_back('&');

		_fields.append(key);
		_fields.push_back('=');
		_fields.append(value);
	}
}

void HttpRequest::setHeader(const String& header)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());

	_header = header;
}

void HttpRequest::setUserId(const String& id)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());

	_userID = id;
}

void HttpRequest::setUserPassword(const String& pw)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());

	_userPassword = pw;
}

bool HttpRequest::onStart(CURL* handle)
{
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);

	if (_userID.length() > 0 && _userPassword.length() > 0)
	{
		curl_easy_setopt(handle, CURLOPT_USERPWD, NULL);

		String userIDPassword;
		userIDPassword = _userID;
		userIDPassword += ":";
		userIDPassword += _userPassword;

		curl_easy_setopt(handle, CURLOPT_USERPWD, userIDPassword.c_str());
	}

	if (_header.length() > 0)
	{
		if (_headerSList)
		{
			curl_slist_free_all(_headerSList);
			_headerSList = NULL;
		}

		_headerSList = curl_slist_append( _headerSList, _header.c_str() );
		if (_headerSList)
			curl_easy_setopt(handle, CURLOPT_HTTPHEADER, _headerSList);
	}

	// TODO: Will a POST changed to a GET automatically when we reuse the handle?
	if (_requestType == HTTP_GET)
	{
		curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);

		_url = _baseURL;
		if (!_fields.empty())
		{
			_url.push_back('?');
			_url.append(_fields);
		}
		curl_easy_setopt(handle, CURLOPT_URL, _url.c_str());
	}
	else if (_requestType == HTTP_POST)
	{
		curl_easy_setopt(handle, CURLOPT_POST, 1);

		_url = _baseURL;
		curl_easy_setopt(handle, CURLOPT_URL, _url.c_str());

		// It'll loop forever unless we set POSTFIELDS! (even if it's empty)
		curl_easy_setopt(handle, CURLOPT_POSTFIELDS, _fields.c_str());
		curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, _fields.length());
	}
	else if (_requestType == HTTP_MULTIPART)
	{
		curl_easy_setopt(handle, CURLOPT_HTTPPOST, _multipartForm->_post);

		_url = _baseURL;
		curl_easy_setopt(handle, CURLOPT_URL, _url.c_str());

		if (!_fields.empty())
		{
			oonvertToMultipart(handle, _fields);
			_fields.clear();
		}

		if (!_multipartForm->_streamList.empty())
			curl_easy_setopt(handle, CURLOPT_READFUNCTION, multipartReadCallback);
	}

	_downloadProgress = 0.0f;
	_uploadProgress = 0.0f;

	return true;
}

size_t HttpRequest::onReceiveHeader(char* ptr, size_t size)
{
	if (_response == NULL)
		_response = new HttpResponse();

	return _response->receiveHeader(ptr, size);
}

size_t HttpRequest::onReceiveData(char* ptr, size_t size)
{
	if (_response == NULL)
		_response = new HttpResponse();

	return _response->receiveData(ptr, size);
}

int HttpRequest::onProgress(double dltotal, double dlnow, double ultotal, double ulnow)
{
	_downloadProgress = dltotal ? float(dlnow / dltotal) : 1.0f;
	_uploadProgress = ultotal ? float(ulnow / ultotal) : 1.0f;

	return 0;
}

void HttpRequest::onDone()
{
	if (_response)
		_response->requestComplete(_handle);

	if (_headerSList)
	{
		curl_slist_free_all(_headerSList);
		_headerSList = NULL;
	}
}

void HttpRequest::onError(CURLcode err)
{
	_response = NULL;

	if (_headerSList)
	{
		curl_slist_free_all(_headerSList);
		_headerSList = NULL;
	}
}

void HttpRequest::onDelete()
{
	safeDelete(_multipartForm);
}

size_t HttpRequest::multipartReadCallback(char* buffer, size_t size, size_t nitems, void* up)
{
	MultipartForm::StreamEntry* entry = static_cast<MultipartForm::StreamEntry*>(up);

	if (entry->request->_canceled) return 0; // signal curl to abort the transfer
	return entry->reader->readRaw(buffer, size * nitems);
}

void HttpRequest::oonvertToMultipart(CURL* handle, const String& fieldStr)
{
	MultipartForm* mf = _multipartForm ? _multipartForm : _multipartForm = new MultipartForm();
	_requestType = HTTP_MULTIPART;

	StringVector fields = StringUtil::split(_fields, "&");

	for (uint i=0; i<fields.size(); ++i)
	{
		size_t eqPos = fields[i].find_first_of('=');
		if (eqPos != fields[i].npos)
		{
			int keyLen;
			char* key = curl_easy_unescape(handle, fields[i].substr(0, eqPos).c_str(), eqPos, &keyLen);

			int valueLen;
			char* value = curl_easy_unescape(handle, fields[i].substr(eqPos+1).c_str(), fields[i].length()-eqPos-1, &valueLen);

			curl_formadd(&mf->_post, &mf->_last, 
				CURLFORM_COPYNAME, key, CURLFORM_NAMELENGTH, keyLen,
				CURLFORM_COPYCONTENTS, value, CURLFORM_CONTENTSLENGTH, valueLen,
				CURLFORM_END);

			curl_free(key);
			curl_free(value);
		}
	}
}

void HttpRequest::addMultipart(const String& name, const String& value, const String& filename, const String& mimeType)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());

	MultipartForm* mf = _multipartForm ? _multipartForm : _multipartForm = new MultipartForm();
	_requestType = HTTP_MULTIPART;

	curl_formadd(&mf->_post, &mf->_last, 
		CURLFORM_PTRNAME, mf->remember(name), CURLFORM_NAMELENGTH, name.length(),
		CURLFORM_PTRCONTENTS, mf->remember(value), CURLFORM_CONTENTSLENGTH, value.length(),
		CURLFORM_FILENAME, mf->remember(filename),
		CURLFORM_CONTENTTYPE, mf->remember(mimeType),
		CURLFORM_END);
}

void HttpRequest::addMultipart(const String& name, StreamReader* reader, const String& filename, const String& mimeType)
{
	if (_handle != NULL)
		NIT_THROW_FMT(EX_INVALID_STATE, "%s '%s' already started", getTypeName(), _url.c_str());

	MultipartForm* mf = _multipartForm ? _multipartForm : _multipartForm = new MultipartForm();
	_requestType = HTTP_MULTIPART;

	MultipartForm::StreamEntry* e = mf->remember(this, reader);

	curl_formadd(&mf->_post, &mf->_last, 
		CURLFORM_PTRNAME, mf->remember(name), CURLFORM_NAMELENGTH, name.length(),
		CURLFORM_STREAM, e, CURLFORM_CONTENTSLENGTH, reader->getSize(),
		CURLFORM_FILENAME, mf->remember(filename),
		CURLFORM_CONTENTTYPE, mf->remember(mimeType),
		CURLFORM_END);
}

////////////////////////////////////////////////////////////////////////////////

HttpResponse::HttpResponse()
: StreamSource(NULL, "", ContentType::UNKNOWN)
{

}

size_t HttpResponse::receiveHeader(char* ptr, size_t size)
{
	if (_header == NULL)
		_header = new MemoryBuffer();

	_header->pushBack(ptr, size);
	return size;
}

size_t HttpResponse::receiveData(char* ptr, size_t size)
{
	if (_downloadWriter)
	{
		size_t bytesWritten = _downloadWriter->writeRaw(ptr, size);

		// TODO: Handle disk full
		return bytesWritten;
	}
	else
	{
		if (_data == NULL)
			_data = new MemoryBuffer();

		_data->pushBack(ptr, size);
		return size;
	}
}

void HttpResponse::requestComplete(CURL* handle)
{
	_downloadWriter = NULL; // release and may flush

	char* ptr = 0;
	curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &ptr);
	if (ptr) 
	{
		int urlLen = 0;
		ptr = curl_easy_unescape(handle, ptr, 0, &urlLen);

		String baseurl(ptr, urlLen);
		_url = baseurl;
		curl_free(ptr);

		size_t qPos = baseurl.find_last_of('?');
		if (qPos != baseurl.npos)
		{
			size_t split = baseurl.find_last_of('/', qPos);
			if (split != baseurl.npos)
			{
				_name = baseurl.substr(split+1, qPos);
				baseurl.resize(split);
			}
			else
			{
				_name = baseurl;
				baseurl.clear();
			}
		}
		else
		{
			size_t split = baseurl.find_last_of('/');
			if (split != baseurl.npos)
			{
				_name = baseurl.substr(split+1);
				baseurl.resize(split);
			}
			else
			{
				_name = baseurl;
				baseurl.clear();
			}
		}
	}

	_timestamp = Timestamp::now();
	curl_easy_getinfo(handle, CURLINFO_FILETIME, &_timestamp);

	_code = 0;
	curl_easy_getinfo(handle, CURLINFO_HTTP_CONNECTCODE, &_code);
	if (_code == 0)
		curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &_code);

	ptr = 0;
	curl_easy_getinfo(handle, CURLINFO_CONTENT_TYPE, &ptr);
	if (ptr) 
	{
		setMimeType(ptr);
	}
	else
	{
		setContentType(ContentType::fromStreamName(_name));
	}
}

StreamReader* HttpResponse::open()
{
	if (_data)
		return new MemoryBuffer::Reader(_data, this);

	if (_downloadWriter)
		NIT_THROW_FMT(EX_NET, "Response from '%s' downloaded to a writer", _url.c_str());
	else
		NIT_THROW_FMT(EX_NET, "Response from '%s' got no data", _url.c_str());

	return NULL;
}

void HttpResponse::setContentType(const ContentType& ct)
{
	_contentType = ContentType::fromStreamName(_name);
	_mimeType = _contentType.getMimeType();
}

void HttpResponse::setMimeType(const String& mimeType)
{
	_mimeType = mimeType;
	_contentType = ContentType::fromMimeType(_mimeType);
	if (_contentType == ContentType::UNKNOWN)
		_contentType = ContentType::fromStreamName(_name);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////

static void makeform_sample(CURL* curl, void* record, long record_length)
{
	struct curl_httppost* post = NULL;  
	struct curl_httppost* last = NULL;  
	char namebuffer[] = "name buffer";  
	long namelength = strlen(namebuffer);  
	char buffer[] = "test buffer";  
	char htmlbuffer[] = "<HTML>test buffer</HTML>";  
	long htmlbufferlength = strlen(htmlbuffer);  
	struct curl_forms forms[3];  
	char file1[] = "my-face.jpg";  
	char file2[] = "your-face.jpg";  

	/* add null character into htmlbuffer, to demonstrate that   transfers of buffers containing null characters actually work  */  
	htmlbuffer[8] = '\0';

	/* Add simple name/content section */  
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "name",   CURLFORM_COPYCONTENTS, "content", CURLFORM_END);

	/* Add simple name/content/contenttype section */  
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "htmlcode",   CURLFORM_COPYCONTENTS, "<HTML></HTML>",   CURLFORM_CONTENTTYPE, "text/html", CURLFORM_END);

	/* Add name/ptrcontent section */  
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "name_for_ptrcontent",   CURLFORM_PTRCONTENTS, buffer, CURLFORM_END);

	/* Add ptrname/ptrcontent section */  
	curl_formadd(&post, &last, CURLFORM_PTRNAME, namebuffer,   CURLFORM_PTRCONTENTS, buffer, CURLFORM_NAMELENGTH,   namelength, CURLFORM_END);

	/* Add name/ptrcontent/contenttype section */  
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "html_code_with_hole",   CURLFORM_PTRCONTENTS, htmlbuffer,   CURLFORM_CONTENTSLENGTH, htmlbufferlength,   CURLFORM_CONTENTTYPE, "text/html", CURLFORM_END);

	/* Add simple file section */  
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "picture",   CURLFORM_FILE, "my-face.jpg", CURLFORM_END);

	/* Add file/contenttype section */  
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "picture",   CURLFORM_FILE, "my-face.jpg",   CURLFORM_CONTENTTYPE, "image/jpeg", CURLFORM_END);

	/* Add two file section */  
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "pictures",   CURLFORM_FILE, "my-face.jpg",   CURLFORM_FILE, "your-face.jpg", CURLFORM_END);

	/* Add two file section using CURLFORM_ARRAY */  
	forms[0].option = CURLFORM_FILE;  forms[0].value = file1;  
	forms[1].option = CURLFORM_FILE;  forms[1].value = file2;  
	forms[2].option = CURLFORM_END;

	/* Add a buffer to upload */  
	curl_formadd(&post, &last,   CURLFORM_COPYNAME, "name",   CURLFORM_BUFFER, "data",   CURLFORM_BUFFERPTR, record,   CURLFORM_BUFFERLENGTH, record_length,   CURLFORM_END);

	/* no option needed for the end marker */  
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "pictures",   CURLFORM_ARRAY, forms, CURLFORM_END);  

	/* Add the content of a file as a normal post text value */  
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "filecontent",   CURLFORM_FILECONTENT, ".bashrc", CURLFORM_END);  

	/* Set the form info */  
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
}
