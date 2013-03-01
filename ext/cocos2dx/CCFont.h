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

#include "CCObject.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

////////////////////////////////////////////////////////////////////////////////

NS_CC_BEGIN;

class CCFontManager;
class CCFontFace;
class CCFont;

////////////////////////////////////////////////////////////////////////////////

class CCFontMgrImpl;

class CC_DLL CCFontManager : public WeakSupported
{
public:
	CCFontManager(unsigned int maxFaces = 4, unsigned int maxSizes = 4, unsigned int maxBytes = 256 * 1024);
	virtual ~CCFontManager();

public:
	static CCFontManager*				getSingleton();
	static void							setSingleton(CCFontManager* man);

public:
	CCFontFace*							install(const String& facename, StreamSource* source, int faceIndex=0);
	CCFontFace*							install(const String& facename, StreamReader* srcReader, int faceIndex=0);

	void								uninstall(const String& facename);

	CCFontFace*							getFontFace(const String& facename);

public:
	// Register: prepare CCFont prior to use
	// Stream-ed overloads auto-installs CCFontFace by alias name (no auto-uninstall)
	CCFont*								Register(const String& alias, CCFont* font);
	CCFont*								Register(const String& alias, StreamSource* source, float defaultSize = 0.0f);
	CCFont*								Register(const String& alias, StreamReader* srcReader, float defaultSize = 0.0f);

	void								unregister(const String& alias);

	CCFont*								getFont(const String& alias); // TODO: size -> float

public:
	CCFont*								getDefault()							{ return _default; }
	void								setDefault(CCFont* font);

public:
	void								compact();

public:
	CCFontMgrImpl*						getImpl()								{ return _impl; }

private:
	typedef map<String, Ref<CCFont> >::type Aliases;
	Aliases								_aliases;

	Ref<CCFont>							_default;
	float								_defaultSize;

	CCFontMgrImpl*						_impl;
};

////////////////////////////////////////////////////////////////////////////////

class CC_DLL CCFontFace : public WeakSupported
{
public:
	const String&						getName()								{ return _name; }

	void								addAlternative(CCFontFace* face, FT_UInt codeBegin, FT_UInt codeEnd, float scaling=1.0f);
	CCFontFace*							getAlternative(FT_UInt code, float& outScaling);

	int									getFlags()								{ return _flags; }
	void								setFlags(int flags);

	int									getLoadOptions()						{ return _loadOptions; }

	enum Flags
	{
		FO_DEFAULT						= 0x0000,
		FO_NO_HINTING					= 0x0001,
		FO_NO_AUTO_HINT					= 0x0002,
		FO_NO_KERNING					= 0x0004,
		FO_NO_BITMAP					= 0x0008,
		FO_NO_ANTIALIAS					= 0x0010,
		FO_LIGHT_HINTING				= 0x0020,
	};

protected:
	CCFontFace(const String& name, StreamReader* srcReader, int fontFaceIndex);
	virtual ~CCFontFace();

	friend class CCFontMgrImpl;
	FT_Face								open(FT_Library ftlib);

private:
	Ref<StreamReader>					_srcReader;
	int									_faceIndex;
	FT_StreamRec						_stream;
	String								_name;
	int									_flags;
	int									_loadOptions;

	struct Alternative
	{
		Weak<CCFontFace>				face;
		FT_UInt							codeBegin;
		FT_UInt							codeEnd;
		float							scaling;
	};

	typedef vector<Alternative>::type	Alternatives;
	Alternatives						_alternatives;

	void								initStream(StreamReader* srcReader);
	static FT_ULong						readStream(FT_Stream stream, FT_ULong offset, FT_Byte* buffer, FT_ULong count);
	static void							closeStream(FT_Stream stream);
};

////////////////////////////////////////////////////////////////////////////////

class CC_DLL CCFont : public RefCounted
{
public:
	explicit CCFont(CCFontFace* face, float defaultSize);
	explicit CCFont(CCFont* deriveFrom);
	explicit CCFont(const String& fromAlias);

public:
	void								addStroke(float strokeWidth, const ccColor4B& color, float offX = 0.0f, float offY = 0.0f);
	void								clearStrokes();

public:
	enum Align
	{
		ALIGN_LEFT						= 0x00,
		ALIGN_CENTER					= 0x01,
		ALIGN_RIGHT						= 0x02,

		ALIGN_BOTTOM					= 0x00,
		ALIGN_MIDDLE					= 0x04,
		ALIGN_TOP						= 0x08,
	};

public:
	CCFontFace*							getFace()								{ return _face; }

public:
	float								getDefaultSize()		/*in point*/	{ return _defaultSize; }
	float								getKerningScale()						{ return _kerningScale; }
	float								getAdvanceScale()						{ return _advanceScale; }
	float								getLineScale()							{ return _lineScale; }
	int									getLineSpacing()		/*in point*/	{ return _lineSpacing; }
	int									getCharSpacing()		/*in point*/	{ return _charSpacing; }
	int									getDpi()								{ return _dpi; }
	const ccColor4B&					getColor()								{ return _color; }

	void								setDefaultSize(float size);
	void								setKerningScale(float scale);
	void								setAdvanceScale(float scale);
	void								setLineScale(float scale);
	void								setLineSpacing(int spacing);
	void								setCharSpacing(int spacing);
	void								setDpi(int dpi);
	void								setColor(const ccColor4B& color);

public:
	inline int							toPixel(int sizeInPoint)				{ return sizeInPoint * _dpi / 72; }
	inline int							toPixel26_6(int sizeInPoint)			{ return (sizeInPoint * _dpi << 6) / 72; }

public:
	class RenderSource;
	RenderSource*						prepare(const char* text, float size=0, int widthLimit=0, int heightLimit=0, int align=0);

private:
	ccColor4B							_color;
	float								_defaultSize;
	float								_kerningScale;
	float								_advanceScale;
	float								_lineScale;
	int									_lineSpacing;
	int									_charSpacing;
	int									_dpi;

	Weak<CCFontFace>					_face;

	struct Stroke
	{
		CCPoint							offset;
		float							width;
		ccColor4B						color;
	};

	typedef list<Stroke>::type			Strokes;
	Strokes								_strokes;
};

////////////////////////////////////////////////////////////////////////////////

class CC_DLL CCFont::RenderSource: public RefCounted
{
public:
	struct LineInfo
	{
		size_t		begin;
		size_t		end;
		FT_BBox		rect;
	};

	struct Glyph
	{
		FT_UInt		index;
		FT_UInt		imgIndex;
		FT_Int		advance;
		FT_Int		kerning;
		FT_Vector	pos;
		void*		cacheHandle;
		int			lineNo;
	};

	Ref<CCFont>							_font;

	vector<LineInfo>::type				_lines;
	vector<Glyph>::type					_glyphs;
	vector<FT_Glyph>::type				_mainImages;
	vector<FT_Glyph>::type				_strokes;

	int									_textWidth;
	int									_textHeight;

	float								_offsetScale;

	void prepare(const char* text, CCFont* font, float size, int widthLimit, int heightLimit, int align);

	void blitBitmap(void* buf, int bpp, int bufWidth, int bufHeight, int ox, int oy);

	virtual void onDelete();
};

////////////////////////////////////////////////////////////////////////////////

NS_CC_END;