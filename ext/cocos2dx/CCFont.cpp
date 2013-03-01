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

#include "nit2d_pch.h"

#include "CCFont.h"

#include FT_CACHE_H

NS_CC_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define FT_ROUND(x)		(x+32) & -64
#define FT_FLOOR(x)		x & -64
#define FT_CEILING(x)	(x+63) & -64

////////////////////////////////////////////////////////////////////////////////

class CCFontMgrImpl
{
public:
	FT_Library							_library;
	FTC_Manager							_cacheMgr;
	FTC_CMapCache						_cMapCache;
	FTC_ImageCache						_imageCache;
	FTC_SBitCache						_sBitCache;

	CCFontMgrImpl(FT_UInt maxFaces, FT_UInt maxSizes, FT_UInt maxBytes)
	{
		FT_Error err;

		err = FT_Init_FreeType(&_library);

		err = FTC_Manager_New(_library, maxFaces, maxSizes, maxBytes, RequestFontFace, this, &_cacheMgr);

		err = FTC_CMapCache_New(_cacheMgr, &_cMapCache);
		
		err = FTC_ImageCache_New(_cacheMgr, &_imageCache);

		err = FTC_SBitCache_New(_cacheMgr, &_sBitCache);
	}

	~CCFontMgrImpl()
	{
		FTC_Manager_Done(_cacheMgr);

		FT_Done_FreeType(_library);
	}

	// CCFontFace == FTC_FaceID

	CCFontFace* Install(const String& facename, StreamReader* srcReader, int faceIndex=0)
	{
		if (m_FaceLookup.find(facename) != m_FaceLookup.end())
			NIT_THROW_FMT(EX_DUPLICATED, "Font face '%s' already installed", facename.c_str());

		CCFontFace* face = new CCFontFace(facename, srcReader, faceIndex);

		m_FaceLookup.insert(std::make_pair(facename, face));

		LOG(0, "-- FontManager: Font face '%s' installed from '%s'\n",
			facename.c_str(), srcReader->getUrl().c_str());

		return face;
	}

	void Uninstall(const String& facename)
	{
		FaceLookup::iterator itr = m_FaceLookup.find(facename);
		if (itr == m_FaceLookup.end()) 
		{
			LOG(0, "*** FontManager: Failed to uninstall font face '%s'\n", facename.c_str());
			return;
		}

		CCFontFace* face = itr->second;
		FTC_Manager_RemoveFaceID(_cacheMgr, (FTC_FaceID)face);

		delete face;

		m_FaceLookup.erase(itr);

		LOG(0, "-- FontManager: Font face '%s' uninstalled\n", facename.c_str());
	}

	CCFontFace* GetFontFace(const String& facename)
	{
		FaceLookup::iterator itr = m_FaceLookup.find(facename);
		return itr != m_FaceLookup.end() ? itr->second : NULL;
	}

	void Compact()
	{
		FTC_Manager_Reset(_cacheMgr);
	}

	typedef map<String, CCFontFace*>::type FaceLookup;
	FaceLookup m_FaceLookup;

	static FT_Error RequestFontFace(FTC_FaceID faceid, FT_Library library, FT_Pointer ud, FT_Face* outFace)
	{
		if (faceid == NULL)
			return FT_Err_Cannot_Open_Resource;

		CCFontMgrImpl* mgr = (CCFontMgrImpl*)ud;
		CCFontFace* face = (CCFontFace*)faceid;

		*outFace = face->open(mgr->_library);

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

static CCFontManager* s_Singleton = NULL;

CCFontManager::CCFontManager(unsigned int maxFaces, unsigned int maxSizes, unsigned int maxBytes)
{
	_defaultSize = 16.0f;

	_impl = NULL;
	_impl = new CCFontMgrImpl(maxFaces, maxSizes, maxBytes);
}

CCFontManager::~CCFontManager()
{
	if (_impl)
		delete _impl;
}

CCFontManager* CCFontManager::getSingleton()
{
	return s_Singleton;
}

void CCFontManager::setSingleton(CCFontManager* man)
{
	s_Singleton = man;
}

void CCFontManager::compact()
{
	_impl->Compact();
}

CCFont* CCFontManager::Register(const String& alias, CCFont* font)
{
	Aliases::iterator itr = _aliases.find(alias);

	if (itr != _aliases.end())
		NIT_THROW_FMT(EX_DUPLICATED, "Font alias '%s' already registered", alias.c_str());

	_aliases.insert(std::make_pair(alias, font));

	LOG(0, "-- FontManager: Font alias '%s' with '%s' %.1f registered\n",
		alias.c_str(),
		font->getFace()->getName().c_str(), font->getDefaultSize());

	if (_default == NULL)
		setDefault(font);

	return font;
}

CCFont* CCFontManager::Register(const String& alias, StreamSource* source, float defaultSize)
{
	return Register(alias, source->open(), defaultSize);
}

CCFont* CCFontManager::Register(const String& alias, StreamReader* srcReader, float defaultSize)
{
	if (defaultSize == 0.0f) 
		defaultSize = _defaultSize;

	CCFontFace* face = _impl->GetFontFace(alias);
	if (face == NULL)
		face = _impl->Install(alias, srcReader);

 	Ref<CCFont> font = new CCFont(face, defaultSize);
	return Register(alias, font);
}

void CCFontManager::unregister(const String& alias)
{
	Aliases::iterator itr = _aliases.find(alias);

	if (itr != _aliases.end())
	{
		if (_default == itr->second)
			_default = NULL;

		_aliases.erase(itr);
		LOG(0, "-- FontManager: Font alias '%s' unregistered\n", alias.c_str());
	}
	else
	{
		LOG(0, "*** FontManager: Failed to unregister font alias '%s'\n", alias.c_str());
	}
}

CCFont* CCFontManager::getFont(const String& alias)
{
	Aliases::iterator itr = _aliases.find(alias);

	if (itr == _aliases.end())
		return NULL;

	CCFont* font = itr->second;

	return font;
}

CCFontFace* CCFontManager::install(const String& facename, StreamSource* source, int faceIndex)
{
	return _impl->Install(facename, source->open(), faceIndex);
}

CCFontFace* CCFontManager::install(const String& facename, StreamReader* srcReader, int faceIndex)
{
	return _impl->Install(facename, srcReader, faceIndex);
}

void CCFontManager::uninstall(const String& facename)
{
	_impl->Uninstall(facename);

	if (_default && _default->getFace() == NULL)
		_default = NULL;
}

CCFontFace* CCFontManager::getFontFace(const String& facename)
{
	return _impl->GetFontFace(facename);
}

void CCFontManager::setDefault(CCFont* font)
{
	_default = font;
}

////////////////////////////////////////////////////////////////////////////////

CCFontFace::CCFontFace(const String& name, StreamReader* srcReader, int fontFaceIndex /*= 0*/)
{
	_name			= name;
	_srcReader		= srcReader;
	_faceIndex		= fontFaceIndex;

	if (!srcReader->isSeekable() || !srcReader->isSized())
	{
		LOG(0, "*** Font '%s': %s not seekable or sized: inefficient buffering used\n", 
			srcReader->getUrl().c_str(), 
			typeid(*srcReader).name());

		_srcReader = new MemoryBuffer::Reader(srcReader);
	}

	setFlags(FO_DEFAULT);
}

CCFontFace::~CCFontFace()
{
}

FT_Face CCFontFace::open(FT_Library ftlib)
{
	FT_Face ret = NULL;
	FT_Open_Args openArgs = { 0 };

	initStream(_srcReader);
	openArgs.stream = &_stream;
	openArgs.flags = FT_OPEN_STREAM;
	FT_Error err = FT_Open_Face(ftlib, &openArgs, _faceIndex, &ret);

	if (err == FT_Err_Unknown_File_Format)
	{
		closeStream(&_stream);
		NIT_THROW_FMT(EX_NOT_SUPPORTED, "%s: not supported format", _srcReader->getUrl().c_str());
	}

	if (err)
	{
		closeStream(&_stream);
		NIT_THROW_FMT(EX_IO, "can't load font from '%s'\n", _srcReader->getUrl().c_str());
	}

	LOG(0, ".. Cached font face '%s' active\n", _name.c_str());

	return ret;
}

void CCFontFace::initStream(StreamReader* srcReader)
{
	if (!srcReader->isSeekable() || !srcReader->isSized())
		NIT_THROW_FMT(EX_INVALID_PARAMS, "Font stream '%s' needs to be seekable & sized", srcReader->getUrl().c_str());

	memset(&_stream, 0, sizeof(_stream));
	_stream.size = srcReader->getSize();
	_stream.descriptor.pointer = srcReader;
	_stream.pathname.pointer = this;
	_stream.read = readStream;
	_stream.close = closeStream;
}

FT_ULong CCFontFace::readStream(FT_Stream stream, FT_ULong offset, FT_Byte* buffer, FT_ULong count)
{
	StreamReader* rdr = (StreamReader*)stream->descriptor.pointer;

	if (count == 0)
	{
		rdr->seek(offset);
		return 0;
	}

	assert(offset == rdr->tell());

	return rdr->readRaw(buffer, count);
}

void CCFontFace::closeStream(FT_Stream stream)
{
	CCFontFace* face = (CCFontFace*)stream->pathname.pointer;
	LOG(0, ".. Cached font face '%s' inactive\n", face->_name.c_str());
}

void CCFontFace::addAlternative(CCFontFace* face, FT_UInt codeBegin, FT_UInt codeEnd, float scaling)
{
	_alternatives.push_back(Alternative());
	Alternative& alt = _alternatives.back();
	alt.face = face;
	alt.codeBegin = codeBegin;
	alt.codeEnd = codeEnd;
	alt.scaling = scaling;
}

CCFontFace* CCFontFace::getAlternative(FT_UInt code, float& outScaling)
{
	for (size_t i=0; i<_alternatives.size(); ++i)
	{
		Alternative& alt = _alternatives[i];
		if (alt.codeBegin <= code && code <= alt.codeEnd && alt.face)
		{
			outScaling = alt.scaling;
			return alt.face;
		}
	}

	outScaling = 1.0f;
	return this;
}

void CCFontFace::setFlags(int flags)
{
	_flags = flags;

	_loadOptions = FT_LOAD_DEFAULT;

	if (flags & FO_NO_HINTING)
		_loadOptions |= FT_LOAD_NO_HINTING;
	else if (flags & FO_LIGHT_HINTING)
		_loadOptions |= FT_LOAD_TARGET_LIGHT;

	if (flags & FO_NO_AUTO_HINT)
		_loadOptions |= FT_LOAD_NO_AUTOHINT;
	else
		_loadOptions |= FT_LOAD_FORCE_AUTOHINT;


	if (flags & FO_NO_BITMAP)
		_loadOptions |= FT_LOAD_NO_BITMAP;
}

////////////////////////////////////////////////////////////////////////////////

CCFont::CCFont(CCFontFace* face, float defaultSize)
{
	_face			= face;
	_defaultSize	= defaultSize;

	// init defaults
	_kerningScale	= 1.0f;
	_advanceScale	= 1.0f;
	_lineScale		= 1.2f;
	_lineSpacing	= 0;
	_charSpacing	= 0;
	_dpi			= 72;
	_color			= ccc4(255, 255, 255, 255);

	if (false)
	{
		// stroke test code
		_color = ccc4(255, 255, 255, 255);
 		addStroke(2.5f, ccc4(0, 0, 255, 128), 2, 2);
 		addStroke(1.2f, ccc4(255, 0, 0, 255), 0, 0);
	}
	else if (false)
	{
		// thick semi-transparent stroke
		addStroke(10, ccc4(0, 0, 128, 128));
	}
	else if (false)
	{
		// thin opaque stroke
		addStroke(1, ccc4(0, 0, 0, 255));
	}
}

CCFont::CCFont(CCFont* from)
{
	// copy attributes
	*this = *from;
}

CCFont::CCFont(const String& fromAlias)
{
	CCFontManager* man = CCFontManager::getSingleton();

	CCFont* from = man->getFont(fromAlias);
	if (from == NULL)
		NIT_THROW_FMT(EX_NOT_FOUND, "Font alias '%s' not registered", fromAlias.c_str());

	*this = *from;
}

CCFont::RenderSource* CCFont::prepare(const char* text, float size, int widthLimit, int heightLimit, int align)
{
	RenderSource* rs = new RenderSource();
	rs->prepare(text, this, size, widthLimit, heightLimit, align);

	return rs;
}

void CCFont::setDefaultSize(float size)
{
	_defaultSize = size;
}

void CCFont::setKerningScale(float scale)
{
	_kerningScale = scale;
}

void CCFont::setAdvanceScale(float scale)
{
	_advanceScale = scale;
}

void CCFont::setLineScale(float scale)
{
	_lineScale = scale;
}

void CCFont::setLineSpacing(int spacing)
{
	_lineSpacing = spacing;
}

void CCFont::setCharSpacing(int spacing)
{
	_charSpacing = spacing;
}

void CCFont::setDpi(int dpi)
{
	_dpi = dpi;
}

void CCFont::setColor(const ccColor4B& color)
{
	_color = color;
}

void CCFont::addStroke(float strokeWidth, const ccColor4B& color, float offX, float offY)
{
	float scale = CC_CONTENT_SCALE_FACTOR(); // TODO: remove

	_strokes.push_back(Stroke());
	Stroke& s = _strokes.back();
	s.width = strokeWidth * scale;
	s.color = color;
	s.offset = CCPointMake(offX, offY);
}

void CCFont::clearStrokes()
{
	_strokes.clear();
}

////////////////////////////////////////////////////////////////////////////////

static void InitBBox(FT_BBox& box)
{
	box.xMin = box.yMin = 32000;
	box.xMax = box.yMax = -32000;
}

static void ExpandBBox(FT_BBox& outer, FT_BBox& inner)
{
	if (outer.xMin > inner.xMin) outer.xMin = inner.xMin;
	if (outer.xMax < inner.xMax) outer.xMax = inner.xMax;
	if (outer.yMin > inner.yMin) outer.yMin = inner.yMin;
	if (outer.yMax < inner.yMax) outer.yMax = inner.yMax;
}

static void FinishBBox(FT_BBox& box)
{
	if (box.xMax <= box.xMin)
	{
		box.xMax = box.xMin = 0;
	}
	else
	{
		// expand left & right by 1 pixel each
		--box.xMin; ++box.xMax;
	}

	if (box.yMax <= box.yMin)
	{
		box.yMax = box.yMin = 0;
	}
	else
	{
		// expand up & down by 1 pixel each
		--box.yMin; ++box.yMax;
	}
}

inline static bool IsIgnored(int ch)
{
	return ch < 32;
}

inline static bool IsNonWord(int ch)
{
	// TODO: implement one referencing http://unicode.org/charts/
	return ch == ' ' || ch > 0x1000;
}

void CCFont::RenderSource::prepare(const char* text, CCFont* font, float size, int widthLimit, int heightLimit, int align)
{
	CCFontMgrImpl* mgr = CCFontManager::getSingleton()->getImpl();

	FT_Library ftlib = mgr->_library;
	FTC_Manager cacheMgr = mgr->_cacheMgr;
	FTC_CMapCache cmapCache = mgr->_cMapCache;
	FTC_ImageCache imageCache = mgr->_imageCache;

	FTC_ScalerRec scaler = { 0 };
	FT_Error err;

	CCFont* defFont = CCFontManager::getSingleton()->getDefault();
	CCFontFace* defFontFace = defFont ? defFont->getFace() : NULL;

	_font = font;

	CCFontFace* baseFace = font->getFace();
	CCFontFace* prevFace = baseFace;

	if (baseFace == NULL)
		baseFace = defFontFace;

	if (baseFace == NULL)
	{
		_font = NULL;
		_textWidth = 0;
		_textHeight = 0;
		return;
	}

	if (size == 0.0f)
		size = font->getDefaultSize();

	scaler.face_id = (FTC_FaceID)baseFace;
	scaler.pixel = 0;
	scaler.height = int(size * 64);
	scaler.width = scaler.height;
	scaler.y_res = font->getDpi();
	scaler.x_res = scaler.y_res;

	FT_Size sizedFace = NULL;
	err = FTC_Manager_LookupSize(cacheMgr, &scaler, &sizedFace);

	UniString uniText = Unicode::toUtf16(text);

	vector<LineInfo>::type& lines = _lines;
	vector<Glyph>::type& glyphs = _glyphs;
	glyphs.reserve(uniText.size()); // never longer than wtext, mostly similar length

	float lineScale = font->getLineScale();
	float kerningScale = font->getKerningScale();
	float advanceScale = font->getAdvanceScale();
	int lineSpacing = font->toPixel26_6(font->getLineSpacing());
	int charSpacing = font->toPixel26_6(font->getCharSpacing());

	widthLimit <<= 6;
	heightLimit <<= 6;

	// Get glyphs for whole string first,
	// and then get gtext[i] glyph for each wtext[i] matching 1:1

	vector<int>::type gtext;
	gtext.resize(uniText.length());

	int prevGlyphIdx = 0;

	for (size_t i=0; i<uniText.length(); ++i)
	{
		int ch = uniText[i];
		gtext[i] = -1;

		if (IsIgnored(ch))
			continue;

		// Select an alternative face by code page
		float scaling;
		CCFontFace* face = baseFace->getAlternative(ch, scaling);

		FT_UInt glyphIdx = FTC_CMapCache_Lookup(cmapCache, (FTC_FaceID)face, -1, ch);

		if (glyphIdx == 0)
		{
			// No such a glyph - select the default font
			if (defFontFace && defFontFace != baseFace)
			{
				face = defFontFace->getAlternative(ch, scaling);
				glyphIdx = FTC_CMapCache_Lookup(cmapCache, (FTC_FaceID)face, -1, ch);
			}
		}

		if (face != prevFace)
		{
			scaler.face_id = (FTC_FaceID)face;
			scaler.height = int(size * scaling * 64);
			scaler.width = scaler.height;
			prevFace = face;
		}

		FT_Glyph glyphImg = NULL;
		FTC_Node glyphImgNode = NULL;

		err = FTC_ImageCache_LookupScaler(
			imageCache, 
			&scaler, 
			face->getLoadOptions() | FT_LOAD_NO_BITMAP,
			glyphIdx,
			&glyphImg,
			&glyphImgNode);

		if (err)
			continue;

		gtext[i] = glyphs.size();
		glyphs.push_back(Glyph());
		Glyph& glyph = glyphs.back();
		glyph.index = glyphIdx;
		glyph.imgIndex = _mainImages.size();
		_mainImages.push_back(glyphImg);
		glyph.cacheHandle = glyphImgNode;

		glyph.advance = int(glyphImg->advance.x * advanceScale);
		glyph.advance >>= 10; // 16.16 -> 26.6

		bool useKerning = false; // TODO: kerning does not work as expected, so do not use it for now.
		useKerning = useKerning && FT_HAS_KERNING(sizedFace->face) && !(face->getFlags() & CCFontFace::FO_NO_KERNING);

		if (useKerning && prevGlyphIdx && glyphIdx)
		{
			FT_Vector delta;
			FT_Get_Kerning(sizedFace->face, prevGlyphIdx, glyphIdx, FT_KERNING_UNFITTED, &delta);

			glyph.kerning = int(delta.x * kerningScale);
			if (glyph.kerning != 0)
			{
				LOG(0, "++ kerning between '%c' and '%c': %d\n", uniText[i-1], uniText[i], glyph.kerning);
			}
		}
		else
		{
			glyph.kerning = 0;
		}

		prevGlyphIdx = glyphIdx;
	}

	// Assign line numbers by word/line breaking and allocate pos.x of each glyph
	int px = 0;
	int lineNo = 1;

	if (widthLimit)
	{
		int textPos = 0;
		int wordEnd = 0;
		int textLen = uniText.size();

		while (textPos < textLen)
		{
			// Process linefeed
			if (uniText[textPos] == '\n')
			{
				++textPos;
				px = 0; ++lineNo;
				continue;
			}

			// Extract a word
			for (wordEnd = textPos; wordEnd < textLen; ++wordEnd)
			{
				int ch = uniText[wordEnd];
				if (IsNonWord(ch) || ch == '\n')
					break;
			}

			bool wordEmpty = textPos == wordEnd;

			// Remember the word start position
			int px0 = px;
			int textPos0 = textPos;

			// Allocate position to each glyph of the word until overflow
			for (; textPos < wordEnd; ++textPos)
			{
				int gpos = gtext[textPos];
				if (gpos < 0) continue;
				Glyph& glyph = glyphs[gpos];

				if (px > 0)
				{
					px += glyph.kerning;
					if (px + glyph.advance >= widthLimit)
						break; // Overflow
				}

				glyph.lineNo = lineNo;
				glyph.pos.x = px >> 6;
				px += glyph.advance;
				px += charSpacing;
			}

			if (textPos < wordEnd)
			{
				// Truncation detected
				// If it's the first word: Write till truncated position
				// Otherwise: Should write whole word at next line

				if (px0 > 0)
					textPos = textPos0; // Rewind to word's start position

				// Feed a line
				px = 0; ++lineNo;
				continue;
			}

			// Process following non-word (or whitespace) - Allocate glyph position until overflow
			for (; textPos < textLen; ++textPos)
			{
				int ch = uniText[textPos];
				if (!IsNonWord(ch))
					break;

				int gpos = gtext[textPos];
				if (gpos < 0) continue;
				Glyph& glyph = glyphs[gpos];

				if (px > 0) 
				{
					px += glyph.kerning;
					if (px + glyph.advance >= widthLimit)
					{
						// Overflow - Feed a line and truncate
						px = 0; ++lineNo;
						break;
					}
				}

				glyph.lineNo = lineNo;
				glyph.pos.x = px >> 6;
				px += glyph.advance;
				px += charSpacing;
			}

			// Here a non-word sequence ends. Continue word extraction at next loop.
			// If a non-word sequence truncated by right margin,
			// process continues as blank word -> non-word sequence at next loop's word extraction part
		}
	}
	else
	{
		// Apply only line break when there's no width limit
		for (size_t i = 0; i < uniText.length(); ++i)
		{
			int ch = uniText[i];
			if (ch == '\n')
			{
				px = 0; ++lineNo;
				continue;
			}

			int gpos = gtext[i];
			if (gpos < 0) continue;
			Glyph& glyph = glyphs[gpos];

			if (px > 0) px += glyph.kerning;
			glyph.lineNo = lineNo;
			glyph.pos.x = px >> 6;
			px += glyph.advance + charSpacing;
		}
	}

	// Split lines
	lines.push_back(LineInfo());
	lines.back().begin = 0;
	
	for (size_t i=0; i<glyphs.size(); ++i)
	{
		while (glyphs[i].lineNo > (int)lines.size())
		{
			lines.back().end = i;

			lines.push_back(LineInfo());
			lines.back().begin = i;
		}
	}
	LineInfo& line = lines.back();
	line.end = glyphs.size();

	// Now we have line & word breaks and allocated pos values

	// Perform stroke prior to obtain a CBox
	bool useStroke = !_font->_strokes.empty();

	_offsetScale = size / _font->getDefaultSize();

	if (useStroke)
	{
		FT_Stroker stroker;
		FT_Stroker_New(ftlib, &stroker);

		for (Strokes::iterator itr = font->_strokes.begin(), end = font->_strokes.end(); itr != end; ++itr)
		{
			int outlineWidth = int(itr->width * 64);

			for (size_t i = 0; i < _mainImages.size(); ++i)
			{
				FT_Glyph stroke = NULL;
				FT_Glyph_Copy(_mainImages[_glyphs[i].imgIndex], &stroke);

				if (outlineWidth > 0)
				{
					FT_Stroker_Set(stroker, outlineWidth, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

					FT_Glyph_StrokeBorder(&stroke, stroker, 0, 1);
				}
				_strokes.push_back(stroke);
			}
		}

		FT_Stroker_Done(stroker);
	}

	// Calc left, right of whole text
	// and left, width, top, height per line
	int textLeft = 32000;
	int textRight = -32000;

	size_t numGlyphs = _glyphs.size();

	for (size_t l = 0; l < lines.size(); ++l)
	{
		LineInfo& line = lines[l];
		InitBBox(line.rect);

		for (size_t i = line.begin; i < line.end; ++i)
		{
			Glyph& glyph = glyphs[i];

			FT_BBox cbox;
			FT_Glyph_Get_CBox(_mainImages[glyph.imgIndex], FT_GLYPH_BBOX_PIXELS, &cbox);

			if (useStroke)
			{
				FT_Glyph_Get_CBox(_mainImages[glyph.imgIndex], FT_GLYPH_BBOX_PIXELS, &cbox);
				int s = 0;
				for (Strokes::iterator itr = _font->_strokes.begin(), end = _font->_strokes.end(); itr != end; ++itr)
				{
					FT_BBox scbox;
					FT_Glyph_Get_CBox(_strokes[numGlyphs * s + glyph.imgIndex], FT_GLYPH_BBOX_PIXELS, &scbox);
					int offX = int(itr->offset.x * _offsetScale);
					int offY = int(itr->offset.y * _offsetScale);
					scbox.xMin += offX; scbox.xMax += offX; 
					scbox.yMin -= offY; scbox.yMax -= offY; 
					ExpandBBox(cbox, scbox);
					++s;
				}
			}

			if (cbox.xMin == cbox.xMax || cbox.yMin == cbox.yMax)
				continue;

			int px = glyph.pos.x;

			cbox.xMin += px; cbox.xMax += px;

			ExpandBBox(line.rect, cbox);
		}

		FinishBBox(line.rect);
	}

	// Now we have setup calculations,
	// Assign glyph.pos.y and calc textHeight
	FT_BBox textRect;
	InitBBox(textRect);

	textRect.yMin = lines[0].rect.yMin;

	int py = 0;

	int minLineHeight = sizedFace->metrics.height >> 6;

	for (size_t l = 0; l < lines.size(); ++l)
	{
		LineInfo& line = lines[l];

		line.rect.yMin += py; line.rect.yMax += py;

		if (line.rect.xMin != line.rect.xMax)
			ExpandBBox(textRect, line.rect);

		for (size_t i = line.begin; i < line.end; ++i)
		{
			Glyph& glyph = glyphs[i];

			glyph.pos.y += py;
		}

		int lineHeight = std::max(int(line.rect.yMax - line.rect.yMin), minLineHeight);

		py -= lineHeight + lineSpacing;
	}

	FinishBBox(textRect);

	if (widthLimit)
		_textWidth = widthLimit >> 6;
	else 
		_textWidth = textRect.xMax - textRect.xMin;

	if (heightLimit)
		_textHeight = heightLimit >> 6;
	else
		_textHeight = textRect.yMax - textRect.yMin;

	// Make width an even value (height also)
	if (_textWidth & 0x01)
		--_textWidth;

	if (_textHeight & 0x01)
		--_textHeight;

	// apply alignment per line, make pos integer, remove min values
	int dy = -textRect.yMin;

	if (align & ALIGN_TOP)
	{
		dy += (_textHeight - (textRect.yMax - textRect.yMin));
		if (textRect.yMax < 0)
			dy += textRect.yMax; // include upperside blank lines
	}
	else if (align & ALIGN_MIDDLE)
	{
		dy += (_textHeight - (textRect.yMax - textRect.yMin)) / 2;
		if (textRect.yMax < 0)
			dy += textRect.yMax; // include upperside blank lines
	}

	for (size_t l = 0; l < lines.size(); ++l)
	{
		LineInfo& line = lines[l];

		int dx = -textRect.xMin;

		if (align & ALIGN_RIGHT)
		{
			dx += (_textWidth - (line.rect.xMax - line.rect.xMin)) - (line.rect.xMin - textRect.xMin);
			if (line.rect.xMin > 0)
				dx += line.rect.xMin; // include leftside whitespaces
		}
		else if (align & ALIGN_CENTER)
		{
			dx += (_textWidth - (line.rect.xMax - line.rect.xMin)) / 2 - (line.rect.xMin - textRect.xMin);
			if (line.rect.xMin > 0)
				dx += line.rect.xMin; // includes leftside whitespaces
		}

		if (dx || dy)
		{
			for (size_t i = line.begin; i < line.end; ++i)
			{
				glyphs[i].pos.x += dx;
				glyphs[i].pos.y += dy;
			}
		}
	}
}

struct Pixel8888
{
	Pixel8888(const ccColor4B& color)
	{
		*this = *(Pixel8888*)&color;
	}

	uint8 r, g, b, a;

	enum { A_BITS = 8, R_BITS = 8, G_BITS = 8, B_BITS = 8 };
	enum { A_MAX = 255, R_MAX = 255, G_MAX = 255, B_MAX = 255 };
};

struct Pixel4444
{
	Pixel4444(const ccColor4B& color)
	{
		r = color.r >> 4;
		g = color.g >> 4;
		b = color.b >> 4;
		a = color.a >> 4;
	}

	uint8 a : 4;
	uint8 b : 4;
	uint8 g : 4;
	uint8 r : 4;

	enum { A_BITS = 4, R_BITS = 4, G_BITS = 4, B_BITS = 4 };
	enum { A_MAX = 15, R_MAX = 15, G_MAX = 15, B_MAX = 15 };
};

template <typename AlphaCalc, typename Blitter, typename TPixel>
inline static void BlitLoop(int l, int t, int r, int b, int bx, int by, unsigned char* srcBuf, size_t srcPitch, unsigned char* dstBuf, size_t dstPitch, TPixel color)
{
	srcBuf += by * srcPitch + bx;
	dstBuf += t * dstPitch + l * sizeof(TPixel);

	for (int iy = t; iy < b; ++iy, ++by)
	{
		unsigned char* src = srcBuf;
		TPixel* dst = (TPixel*)dstBuf;

		for (int ix = l; ix < r; ++ix)
		{
			unsigned char b = (*src++) >> (8 - TPixel::A_BITS);
			TPixel* pd = dst++;

			if (b == 0) continue;

			TPixel s = color;
			int sa = AlphaCalc::CalcAlpha(b, s);

			if (sa == 0) continue;

			s.a = sa;

			// apply premultiplied alpha
			++sa;
			s.r = s.r * sa >> s.A_BITS;
 			s.g = s.g * sa >> s.A_BITS;
 			s.b = s.b * sa >> s.A_BITS;

			Blitter::Blit(s, pd, sa);
		}

		srcBuf += srcPitch;
		dstBuf += dstPitch;
	}
}

struct FullOpacityAlphaCalc
{
	template <typename TPixel>
	inline static int CalcAlpha(unsigned char b, TPixel s)
	{
		return b;
	}
};

struct TransparentAlphaCalc
{
	template <typename TPixel>
	inline static int CalcAlpha(unsigned char b, TPixel s)
	{
		return b * (1 + s.a) >> s.A_BITS;
	}
};

struct SimpleBlit
{
	template <typename TPixel>
	inline static void Blit(TPixel s, TPixel* pd, int sa)
	{
		*pd = s;
	}
};

struct AdditiveBlendBlit
{
	template <typename TPixel>
	inline static void Blit(TPixel s, TPixel* pd, int sa)
	{
		TPixel d = *pd;

		int invsa = (s.A_MAX + 2) - sa; // max(255 or 15) - sa + 1 (sa has already + 1)
		d.a = s.a + (d.a * invsa >> s.A_BITS);
		d.r = s.r + (d.r * invsa >> s.A_BITS);
		d.g = s.g + (d.g * invsa >> s.A_BITS);
		d.b = s.b + (d.b * invsa >> s.A_BITS);

		*pd = d;
	}
};

template <typename TPixel>
static void BlitGlyph(FT_BitmapGlyph glyph, const FT_Vector& pos, void* buf, int bufPitch, int bufHeight, const TPixel& color, bool alphaBlend)
{
	int bufWidth = bufPitch / sizeof(TPixel);

	int t = - pos.y + bufHeight - glyph->top;
	int l = pos.x + glyph->left;

	int b = t + glyph->bitmap.rows;
	int r = l + glyph->bitmap.width;
	int pitch = glyph->bitmap.pitch;

	if (b < 0 || t >= bufHeight || l >= bufPitch || r < 0) 
		return;

	unsigned char* srcBuf = glyph->bitmap.buffer;
	unsigned char* dstBuf = (unsigned char*)buf;

	int bx = 0, by = 0;
	int obx = 0;

	// clamp rect
	if (t < 0) 
	{ 
		by -= t; 
		t = 0; 
	}

	if (b > bufHeight)
		b = bufHeight;

	if (l < 0)
	{
		bx -= l;
		l = 0;
	}

	if (r > bufWidth)
		r = bufWidth;

	bool transparent = color.a < color.A_MAX;
	bool grayScale = color.r == color.g && color.g == color.b;

	if (alphaBlend)
	{
		if (transparent)
			BlitLoop<TransparentAlphaCalc, AdditiveBlendBlit, TPixel>(l, t, r, b, bx, by, srcBuf, pitch, dstBuf, bufPitch, color);
		else
			BlitLoop<FullOpacityAlphaCalc, AdditiveBlendBlit, TPixel>(l, t, r, b, bx, by, srcBuf, pitch, dstBuf, bufPitch, color);
	}
	else
	{
		if (transparent)
			BlitLoop<TransparentAlphaCalc, SimpleBlit, TPixel>(l, t, r, b, bx, by, srcBuf, pitch, dstBuf, bufPitch, color);
		else
			BlitLoop<FullOpacityAlphaCalc, SimpleBlit, TPixel>(l, t, r, b, bx, by, srcBuf, pitch, dstBuf, bufPitch, color);
	}
}

void CCFont::RenderSource::blitBitmap(void* buf, int bpp, int bufWidth, int bufHeight, int ox, int oy)
{
	if (_font == NULL || _font->getFace() == NULL)
		return;

	int bufPitch = bpp * bufWidth;

	FT_Error err;

	bool alphaBlend = false;

	// render strokes first
	size_t numGlyphs = _glyphs.size();

	int ff = _font->getFace()->getFlags();
	FT_Render_Mode renderMode;

	if (ff & CCFontFace::FO_LIGHT_HINTING)
		renderMode = FT_RENDER_MODE_LIGHT;
	else
		renderMode = FT_RENDER_MODE_NORMAL;

	int s = 0;
	for (Strokes::iterator itr = _font->_strokes.begin(), end = _font->_strokes.end(); itr != end; ++itr)
	{
		// Each stroke overlaps others much, so start alphablend turned on.
		alphaBlend = true;

		for (size_t i = 0; i < numGlyphs; ++i)
		{
			Glyph& glyph = _glyphs[i];

			FT_Vector pos = glyph.pos;
			int offX = int(itr->offset.x * _offsetScale);
			int offY = int(itr->offset.y * _offsetScale);

			pos.x += ox + offX;
			pos.y -= oy + offY;

			FT_Glyph& stroke = _strokes[glyph.imgIndex + numGlyphs * s];

			err = FT_Glyph_To_Bitmap(&stroke, renderMode, 0, 1);

			if (err) 
				continue;

			FT_BitmapGlyph bglyph = (FT_BitmapGlyph)stroke;
			if (bglyph == NULL)
				continue;

			if (bpp == 2)
			{
				Pixel4444 color = itr->color;
				BlitGlyph(bglyph, pos, buf, bufPitch, bufHeight, color, alphaBlend);
			}
			else if (bpp == 4)
			{
				Pixel8888 color = itr->color;
				BlitGlyph(bglyph, pos, buf, bufPitch, bufHeight, color, alphaBlend);
			}

			FT_Done_Glyph(stroke);
		}
		++s;
	}

	for (size_t i = 0; i < numGlyphs; ++i)
	{
		Glyph& glyph = _glyphs[i];

		FT_Vector pos = glyph.pos;
		pos.x += ox;
		pos.y -= oy;

		FT_Glyph& mainImg = _mainImages[glyph.imgIndex];

		err = FT_Glyph_To_Bitmap(&mainImg, renderMode, 0, 0);

		if (err) 
			continue;

		FT_BitmapGlyph bglyph = (FT_BitmapGlyph)mainImg;
		if (bglyph == NULL)
			continue;

		if (bpp == 2)
		{
			Pixel4444 color = _font->_color;
			BlitGlyph(bglyph, pos, buf, bufPitch, bufHeight, color, alphaBlend);
		}
		else if (bpp == 4)
		{
			Pixel8888 color = _font->_color;
			BlitGlyph(bglyph, pos, buf, bufPitch, bufHeight, color, alphaBlend);
		}

		FT_Done_Glyph(mainImg);
	}

	_strokes.clear();
	_mainImages.clear();
}

void CCFont::RenderSource::onDelete()
{
	for (size_t i = 0; i < _strokes.size(); ++i)
	{
		FT_Done_Glyph(_strokes[i]);
	}

	CCFontMgrImpl* mgr = CCFontManager::getSingleton()->getImpl();

	FTC_Manager cacheMgr = mgr->_cacheMgr;

	for (size_t i = 0; i < _glyphs.size(); ++i)
	{
		if (_glyphs[i].cacheHandle)
		{
			FTC_Node_Unref((FTC_Node)_glyphs[i].cacheHandle, cacheMgr);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_CC_END;