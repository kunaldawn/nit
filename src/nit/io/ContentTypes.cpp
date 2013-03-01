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

#include "nit/io/ContentTypes.h"
#include "nit/io/Stream.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class ContentTypeLib : public TRuntimeSingleton<ContentTypeLib>
{
public:
	virtual void onInit()
	{
		_toName[ContentType::UNKNOWN] = "UNKNOWN";
		_fromName["UNKNOWN"] = ContentType::UNKNOWN;

		_toName[ContentType::DELETED] = "DELETED";
		_fromName["DELETED"] = ContentType::DELETED;

		// reference: http://en.wikipedia.org/wiki/Internet_media_type
		Register(ContentType::TEXT_PLAIN,			"TEXT.PLAIN",			"txt",		"text/plain");
		Register(ContentType::TEXT_JSON,			"TEXT.JSON",			"json", 	"application/json");
		Register(ContentType::TEXT_CFG,				"TEXT.CFG",				"cfg",		"application/x-nit-cfg");
		Register(ContentType::TEXT_CFG,				"TEXT.CFG",				"ini",		"application/x-nit-cfg");
		Register(ContentType::TEXT_XML,				"TEXT.XML",				"xml",		"text/xml");
		Register(ContentType::TEXT_HTML,			"TEXT.HTML",			"htm",		"text/html");
		Register(ContentType::TEXT_HTML,			"TEXT.HTML",			"html", 	"text/html");
		Register(ContentType::TEXT_PLIST,			"TEXT.PLIST",			"plist",	"text/plist");

		Register(ContentType::SCRIPT_NIT,			"SCRIPT.NIT",			"nit",		"application/x-nit-script");
		Register(ContentType::SCRIPT_HLSL,			"SCRIPT.HLSL",			"hlsl",		"application/x-hlsl");
		Register(ContentType::SCRIPT_GLSL,			"SCRIPT.GLSL",			"glsl",		"application/x-glsl");

		Register(ContentType::BINARY_SETTINGS,		"BINARY.SETTINGS",		"nbs",		"application/x-nit-settings");
		Register(ContentType::BINARY_DATA,			"BINARY.DATA",			"data",		"application/x-nit-data");
		Register(ContentType::BINARY_ZDATA,			"BINARY.ZDATA",			"zdata",	"application/x-nit-zdata");
		Register(ContentType::BINARY_DLL,			"BINARY.DLL",			"dll",		"application/x-dll");
		Register(ContentType::BINARY_NIT_SCRIPT,	"BINARY.NIT_SCRIPT",	"ncs",		"application/x-nit-compiled-script");

		Register(ContentType::OGRE_PROGRAM,			"OGRE.PROGRAM",			"program",	"application/x-ogre-program");
		Register(ContentType::OGRE_MATERIAL,		"OGRE.MATERIAL",		"material",	"application/x-ogre-material");
		Register(ContentType::OGRE_FONTDEF,			"OGRE.FONTDEF",			"fontdef",	"application/x-ogre-fontdef");
		Register(ContentType::OGRE_COMPOSITOR,		"OGRE.COMPOSITOR",		"compositor", "application/x-ogre-compositor");
		Register(ContentType::OGRE_OVERLAY,			"OGRE.OVERLAY",			"overlay",	"applciation/x-ogre-overlay");
		Register(ContentType::OGRE_PU,				"OGRE.PU",				"pu",		"application/x-ogre-pu");
		Register(ContentType::OGRE_PU,				"OGRE.PU",				"pua",		"application/x-ogre-pu");
		Register(ContentType::OGRE_AST_MATERIAL,	"OGRE.AST_MATERIAL",	"astpg",	"application/x-ogre-ast-material");
		Register(ContentType::OGRE_AST_FONTDEF,		"OGRE.AST_FONTEDEF",	"astfd",	"application/x-ogre-ast-fontdef");
		Register(ContentType::OGRE_AST_COMPOSITOR,	"OGRE.AST_COMPOSITOR",	"astcp",	"application/x-ogre-ast-compositor");
		Register(ContentType::OGRE_AST_OVERLAY,		"OGRE.AST_OVERLAY",		"astov",	"applciation/x-ogre-ast-overlay");
		Register(ContentType::OGRE_AST_PU,			"OGRE.AST_PU",			"astpu",	"application/x-ogre-ast-pu");
		Register(ContentType::OGRE_MESH,			"OGRE.MESH",			"mesh",		"application/x-ogre-mesh");
		Register(ContentType::OGRE_SKELETON,		"OGRE.SKELETON",		"skeleton",	"application/x-ogre-skeleton");
		Register(ContentType::OGRE_XML_MESH,		"OGRE.XML_MESH",		"meshx",	"application/x-ogre-xml-mesh");
		Register(ContentType::OGRE_XML_SKELETON,	"OGRE.XML_SKELETON",	"skeletonx","application/x-ogre-xml-skeleton");

		Register(ContentType::ARCHIVE_PACK,			"ARCHIVE.PACK",			"pack",		"application/x-nit-pack");
		Register(ContentType::ARCHIVE_BUNDLE,		"ARCHIVE.BUNDLE",		"bundle",	"application/x-nit-bundle");
		Register(ContentType::ARCHIVE_BUNDLE,		"ARCHIVE.BUNDLE",		"rtttl",	"application/x-nit-bundle"); // HACK: Android aapt does not compresses this format so using this.
		Register(ContentType::ARCHIVE_ZBUNDLE,		"ARCHIVE.ZBUNDLE",		"zbundle",	"application/x-nit-zbundle");
		Register(ContentType::ARCHIVE_ZIP,			"ARCHIVE.ZIP",			"zip",		"application/zip");
		Register(ContentType::ARCHIVE_DB_SQLITE,	"ARCHIVE.DB_SQLITE",	"sqlite",	"application/x-db-sqlite");

		Register(ContentType::IMAGE_GIF,			"IMAGE.GIF",			"gif",		"image/gif");
		Register(ContentType::IMAGE_JPEG,			"IMAGE.JPEG",			"jpg",		"image/jpeg");
		Register(ContentType::IMAGE_JPEG,			"IMAGE.JPEG",			"jpeg",		"image/jpeg");
		Register(ContentType::IMAGE_PNG,			"IMAGE.PNG",			"png",		"image/png");
		Register(ContentType::IMAGE_TGA,			"IMAGE.TGA",			"tga",		"image/tga");
		Register(ContentType::IMAGE_DDS,			"IMAGE.DDS",			"dds",		"image/dds");
		Register(ContentType::IMAGE_BMP,			"IMAGE.BMP",			"bmp",		"image/bmp");
		Register(ContentType::IMAGE_PVR,			"IMAGE.PVR",			"pvr",		"application/x-pvr");
		Register(ContentType::IMAGE_PSD,			"IMAGE.PSD",			"psd",		"application/psd");
		Register(ContentType::IMAGE_CCZ,			"IMAGE.CCZ",			"ccz",		"application/x-cocos-ccz");
		Register(ContentType::IMAGE_NTEX,			"IMAGE.NTEX",			"ntex",		"application/x-nit-tex");

		Register(ContentType::AUDIO_WAV,			"AUDIO.WAV",			"wav",		"audio/wav");
		Register(ContentType::AUDIO_MP3,			"AUDIO.MP3",			"mp3",		"audio/mpeg");
		Register(ContentType::AUDIO_OGG,			"AUDIO.OGG",			"ogg",		"audio/ogg");
		Register(ContentType::AUDIO_WMA,			"AUDIO.WMA",			"wma",		"audio/x-ms-wma");

		Register(ContentType::VIDEO_MPEG,			"VIDEO.MPEG",			"mpeg",		"video/mpeg");
		Register(ContentType::VIDEO_MP4,			"VIDEO.MP4",			"mp4",		"video/mp4");
		Register(ContentType::VIDEO_BINK,			"VIDEO.BINK",			"bik",		"application/x-bink");
		Register(ContentType::VIDEO_WMV,			"VIDEO.WMV",			"wmv",		"video/x-ms-wmv");
	}

	virtual void onFinish()
	{
		_fromName.clear();
		_toName.clear();
		_fromExt.clear();
		_toExt.clear();
		_fromMimeType.clear();
		_toMimeType.clear();
	}

	void Register(ContentType contentType, const String& name, const String& ext, const String& mimeType)
	{
		_fromName[name] = contentType;
		_toName[contentType] = name;

		_fromExt[ext] = contentType;
		_toExt[contentType] = ext;

		_fromMimeType[mimeType] = contentType;
		_toMimeType[contentType] = mimeType;
	}

	typedef std::map<String, ContentType::ValueType> StrToTypeMap;
	typedef std::map<ContentType::ValueType, String> TypeToStrMap;

	StrToTypeMap						_fromName;
	TypeToStrMap						_toName;

	StrToTypeMap						_fromExt;
	TypeToStrMap						_toExt;

	StrToTypeMap						_fromMimeType;
	TypeToStrMap						_toMimeType;
};

////////////////////////////////////////////////////////////////////////////////

ContentType ContentType::fromName(const String& name)
{
	ContentTypeLib& ctl = ContentTypeLib::getSingleton();

	ContentTypeLib::StrToTypeMap::iterator itr = ctl._fromName.find(name);
	return itr != ctl._fromName.end() ? itr->second : UNKNOWN;
}

const String& ContentType::toName(ContentType contentType)
{
	ContentTypeLib& ctl = ContentTypeLib::getSingleton();

	ContentTypeLib::TypeToStrMap::iterator itr = ctl._toName.find(contentType);
	return itr != ctl._toName.end() ? itr->second : ctl._toName[UNKNOWN];
}

ContentType ContentType::fromExtension(const String& ext)
{
	ContentTypeLib& ctl = ContentTypeLib::getSingleton();

	ContentTypeLib::StrToTypeMap::iterator itr = ctl._fromExt.find(ext);
	return itr != ctl._fromExt.end() ? itr->second : UNKNOWN;
}

const String& ContentType::toExtension(ContentType contentType, const String& unknown)
{
	ContentTypeLib& ctl = ContentTypeLib::getSingleton();

	ContentTypeLib::TypeToStrMap::iterator itr = ctl._toExt.find(contentType);
	return itr != ctl._toExt.end() ? itr->second : unknown;
}

ContentType ContentType::fromStreamName(const String& streamName)
{
	return fromExtension(StreamSource::getExtension(streamName));
}

ContentType ContentType::fromMimeType(const String& mimeType)
{
	ContentTypeLib& ctl = ContentTypeLib::getSingleton();

	ContentTypeLib::StrToTypeMap::iterator itr = ctl._fromMimeType.find(mimeType);
	return itr != ctl._fromMimeType.end() ? itr->second : UNKNOWN;
}

const String& ContentType::toMimeType(ContentType contentType, const String& unknown)
{
	ContentTypeLib& ctl = ContentTypeLib::getSingleton();

	ContentTypeLib::TypeToStrMap::iterator itr = ctl._toMimeType.find(contentType);
	return itr != ctl._toMimeType.end() ? itr->second : unknown;
}

void ContentType::allContentTypes(vector<ContentType>::type& outResults)
{
	ContentTypeLib& ctl = ContentTypeLib::getSingleton();

	for (ContentTypeLib::StrToTypeMap::iterator itr = ctl._fromName.begin(), end = ctl._fromName.end(); itr != end; ++itr)
	{
		outResults.push_back(itr->second);
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
