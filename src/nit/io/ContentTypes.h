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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NIT_API ContentType
{
public:
	enum
	{
		UNKNOWN							= 0x0000,
		DELETED							= 0x0001,								// Marker for deleted file on patch

		// MASK starts from 0x0020

		IS_TEXT							= 0x0020,
		IS_SCRIPT						= 0x0040,	
		IS_XML							= 0x0080,
		IS_BINARY						= 0x0100,
		IS_OGRE							= 0x0200,
		IS_NIT							= 0x0400,
		IS_ARCHIVE						= 0x0800 | IS_BINARY,
		IS_IMAGE						= 0x1000 | IS_BINARY,
		IS_AUDIO						= 0x2000 | IS_BINARY,
		IS_VIDEO						= 0x4000 | IS_BINARY,
		IS_COMPRESSED					= 0x8000,

		// 0x01 ~ 0x1F available for each mask combination,

		TEXT_PLAIN						= 0x01 | IS_TEXT,
		TEXT_JSON						= 0x02 | IS_TEXT,
		TEXT_CFG						= 0x01 | IS_TEXT | IS_NIT,
		TEXT_XML						= 0x01 | IS_TEXT | IS_XML,
		TEXT_HTML						= 0x02 | IS_TEXT | IS_XML,
		TEXT_PLIST						= 0x03 | IS_TEXT | IS_XML,

		SCRIPT_NIT						= 0x01 | IS_TEXT | IS_SCRIPT | IS_NIT,
		SCRIPT_HLSL						= 0x01 | IS_TEXT | IS_SCRIPT,
		SCRIPT_GLSL						= 0x02 | IS_TEXT | IS_SCRIPT,

		BINARY_SETTINGS					= 0x01 | IS_BINARY | IS_NIT,
		BINARY_DATA						= 0x02 | IS_BINARY | IS_NIT,
		BINARY_ZDATA					= 0x03 | IS_BINARY | IS_NIT | IS_COMPRESSED,
		BINARY_DLL						= 0x02 | IS_BINARY,
		BINARY_NIT_SCRIPT				= 0x01 | IS_BINARY | IS_SCRIPT | IS_NIT,

		OGRE_PROGRAM					= 0x01 | IS_TEXT | IS_SCRIPT | IS_OGRE,
		OGRE_MATERIAL					= 0x02 | IS_TEXT | IS_SCRIPT | IS_OGRE,
		OGRE_FONTDEF					= 0x03 | IS_TEXT | IS_SCRIPT | IS_OGRE,
		OGRE_COMPOSITOR					= 0x04 | IS_TEXT | IS_SCRIPT | IS_OGRE,
		OGRE_OVERLAY					= 0x05 | IS_TEXT | IS_SCRIPT | IS_OGRE,
		OGRE_PU							= 0x06 | IS_TEXT | IS_SCRIPT | IS_OGRE,
		OGRE_AST_MATERIAL				= 0x02 | IS_BINARY | IS_SCRIPT | IS_OGRE,
		OGRE_AST_FONTDEF				= 0x03 | IS_BINARY | IS_SCRIPT | IS_OGRE,
		OGRE_AST_COMPOSITOR				= 0x04 | IS_BINARY | IS_SCRIPT | IS_OGRE,
		OGRE_AST_OVERLAY				= 0x05 | IS_BINARY | IS_SCRIPT | IS_OGRE,
		OGRE_AST_PU						= 0x06 | IS_BINARY | IS_SCRIPT | IS_OGRE,
		OGRE_MESH						= 0x01 | IS_BINARY | IS_OGRE,
		OGRE_SKELETON					= 0x02 | IS_BINARY | IS_OGRE,
		OGRE_XML_MESH					= 0x01 | IS_TEXT | IS_XML | IS_OGRE,
		OGRE_XML_SKELETON				= 0x02 | IS_TEXT | IS_XML | IS_OGRE,

		ARCHIVE_PACK					= 0x01 | IS_ARCHIVE | IS_NIT,
		ARCHIVE_BUNDLE					= 0x02 | IS_ARCHIVE | IS_NIT,
		ARCHIVE_ZBUNDLE					= 0x03 | IS_ARCHIVE | IS_NIT | IS_COMPRESSED,
		ARCHIVE_ZIP						= 0x01 | IS_ARCHIVE | IS_COMPRESSED,
		ARCHIVE_DB_SQLITE				= 0x03 | IS_ARCHIVE,

		IMAGE_GIF						= 0x01 | IS_IMAGE | IS_COMPRESSED,
		IMAGE_JPEG						= 0x02 | IS_IMAGE | IS_COMPRESSED,
		IMAGE_PNG						= 0x03 | IS_IMAGE | IS_COMPRESSED,
		IMAGE_TGA						= 0x04 | IS_IMAGE,
		IMAGE_DDS						= 0x05 | IS_IMAGE,
		IMAGE_BMP						= 0x06 | IS_IMAGE,
		IMAGE_PVR						= 0x07 | IS_IMAGE,
		IMAGE_PSD						= 0x08 | IS_IMAGE,
		IMAGE_CCZ						= 0x09 | IS_IMAGE | IS_COMPRESSED,
		IMAGE_NTEX						= 0x01 | IS_IMAGE | IS_NIT,

		AUDIO_WAV						= 0x01 | IS_AUDIO,
		AUDIO_MP3						= 0x02 | IS_AUDIO | IS_COMPRESSED,
		AUDIO_OGG						= 0x03 | IS_AUDIO | IS_COMPRESSED,
		AUDIO_WMA						= 0x04 | IS_AUDIO | IS_COMPRESSED,

		VIDEO_MPEG						= 0x01 | IS_VIDEO | IS_COMPRESSED,
		VIDEO_MP4						= 0x02 | IS_VIDEO | IS_COMPRESSED,
		VIDEO_BINK						= 0x03 | IS_VIDEO | IS_COMPRESSED,
		VIDEO_WMV						= 0x04 | IS_VIDEO | IS_COMPRESSED,
	};

public:
	typedef u16 ValueType;

	static ContentType					fromName(const String& name);
	static const String&				toName(ContentType contentType);

	static ContentType					fromExtension(const String& ext);
	static const String&				toExtension(ContentType contentType, const String& unknown = StringUtil::BLANK());

	static ContentType					fromStreamName(const String& streamName);

	static ContentType					fromMimeType(const String& mimeType);
	static const String&				toMimeType(ContentType contentType, const String& unknown = StringUtil::BLANK());

	static void							allContentTypes(vector<ContentType>::type& outResults);

public:
	const String&						getName() const							{ return toName(*this); }
	const String&						getExtension() const					{ return toExtension(*this); }
	const String&						getMimeType() const						{ return toMimeType(*this); }
	ValueType							getValue() const						{ return _value; }

	bool								isUnknown() const						{ return _value == UNKNOWN; }
	bool								isText() const							{ return (_value & IS_TEXT) == IS_TEXT; }
	bool								isScript() const						{ return (_value & IS_SCRIPT) == IS_SCRIPT; }
	bool								isXML() const							{ return (_value & IS_XML) == IS_XML; }
	bool								isBinary() const						{ return (_value & IS_BINARY) == IS_BINARY; }
	bool								isOgre() const							{ return (_value & IS_OGRE) == IS_OGRE; }
	bool								isNit() const							{ return (_value & IS_NIT) == IS_NIT; }
	bool								isArchive() const						{ return (_value & IS_ARCHIVE) == IS_ARCHIVE; }
	bool								isImage() const							{ return (_value & IS_IMAGE) == IS_IMAGE; }
	bool								isAudio() const							{ return (_value & IS_AUDIO) == IS_AUDIO; }
	bool								isVideo() const							{ return (_value & IS_VIDEO) == IS_VIDEO; }
	bool								isCompressed() const					{ return (_value & IS_COMPRESSED) == IS_COMPRESSED; }

public:
	ContentType() : _value(UNKNOWN)												{ }
	ContentType(ValueType value) : _value(value)								{ }

	operator ValueType() const													{ return _value; }

private:
	u16									_value;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
