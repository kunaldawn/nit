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

#include "nit/content/PixelFormat.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class PixelFormatLib : public TRuntimeSingleton<PixelFormatLib>
{
public:
	virtual void onInit()
	{
		LOG(0, ".. rgba 8888: %d\n", sizeof(PixelRGBA_8888));
		LOG(0, ".. rgba 4444: %d\n", sizeof(PixelRGBA_4444));
		LOG(0, ".. rgb 888: %d\n", sizeof(PixelRGB_888));

		// Check pixel structure sizes critical to runtime
		ASSERT_THROW(PixelRGBA_8888::checkSize(), EX_SYSTEM);
		ASSERT_THROW(PixelRGBA_4444::checkSize(), EX_SYSTEM);
		ASSERT_THROW(PixelRGB_888::checkSize(), EX_SYSTEM);

		// Register pixel formats and their literals
		Register(PixelFormat::UNKNOWN,			"unknown");

		Register(PixelFormat::RGB_888,			"rgb_888");
		Register(PixelFormat::RGB_565,			"rgb_565");
		Register(PixelFormat::A8,				"a8");
		Register(PixelFormat::L8,				"l8");
		Register(PixelFormat::AL_88,			"al_88");
		Register(PixelFormat::AL_88_PA,			"al_88_pa");

		Register(PixelFormat::ARGB_8888,		"argb_8888");
		Register(PixelFormat::ARGB_4444,		"argb_4444");
		Register(PixelFormat::ARGB_1555,		"argb_1555");
		Register(PixelFormat::ARGB_8888_PA,		"argb_8888_pa");
		Register(PixelFormat::ARGB_4444_PA,		"argb_4444_pa");
		Register(PixelFormat::ARGB_1555_PA,		"argb_1555_pa");
		Register(PixelFormat::XRGB_8888,		"xrgb_8888");
		Register(PixelFormat::DXT1,				"dxt1");
		Register(PixelFormat::DXT3,				"dxt3");
		Register(PixelFormat::DXT3_PA,			"dxt3_pa");
		Register(PixelFormat::DXT5,				"dxt5");
		Register(PixelFormat::DXT5_PA,			"dxt5_pa");

		Register(PixelFormat::RGBA_8888,		"rgba_8888");
		Register(PixelFormat::BGRA_8888,		"bgra_8888");
		Register(PixelFormat::RGBA_4444,		"rgba_4444");
		Register(PixelFormat::RGBA_5551,		"rgba_5551");
		Register(PixelFormat::RGBA_8888_PA,		"rgba_8888_pa");
		Register(PixelFormat::BGRA_8888_PA,		"bgra_8888_pa");
		Register(PixelFormat::RGBA_4444_PA,		"rgba_4444_pa");
		Register(PixelFormat::RGBA_5551_PA,		"rgba_5551_pa");
		Register(PixelFormat::PVR2,				"pvr2");
		Register(PixelFormat::PVR2A,			"pvr2a");
		Register(PixelFormat::PVR2A_PA,			"pvr2a_pa");
		Register(PixelFormat::PVR4,				"pvr4");
		Register(PixelFormat::PVR4A,			"pvr4a");
		Register(PixelFormat::PVR4A_PA,			"pvr4a_pa");
		Register(PixelFormat::ETC,				"etc");
	}

	virtual void onFinish()
	{
		_fromName.clear();
		_toName.clear();
	}

	void Register(PixelFormat pf, const String& name)
	{
		_fromName[name] = pf;
		_toName[pf] = name;
	}

	typedef std::map<String, PixelFormat::ValueType> FromStrMap;
	typedef std::map<PixelFormat::ValueType, String> ToStrMap;

	FromStrMap							_fromName;
	ToStrMap							_toName;
};

////////////////////////////////////////////////////////////////////////////////

PixelFormat::PixelFormat(const String& name)
{
	PixelFormatLib& pfl = PixelFormatLib::getSingleton();

	PixelFormatLib::FromStrMap::iterator itr = pfl._fromName.find(name);
	_value = itr != pfl._fromName.end() ? itr->second : UNKNOWN;
}

const String& PixelFormat::getName() const
{
	PixelFormatLib& pfl = PixelFormatLib::getSingleton();

	PixelFormatLib::ToStrMap::iterator itr = pfl._toName.find(_value);
	return itr != pfl._toName.end() ? itr->second : pfl._toName[UNKNOWN];
}

int PixelFormat::calcBitsPerPixel(PixelFormat pixelFormat)
{
	int bpp = (pixelFormat & FLAG_BPP_MASK) >> 16;
	if (bpp == 8)
		return 24;
	return 1 << bpp;
}

int PixelFormat::calcPitch(PixelFormat pixelFormat, int width)
{
	// TODO: handle shortage of bits count & non-aligned bits
	int bpp = calcBitsPerPixel(pixelFormat);
	return width * bpp >> 3;
}

void PixelFormat::allFormats(vector<std::pair<String, PixelFormat> >::type& outResults)
{
	PixelFormatLib& pfl = PixelFormatLib::getSingleton();

	for (PixelFormatLib::FromStrMap::iterator itr = pfl._fromName.begin(), end = pfl._fromName.end(); itr != end; ++itr)
	{
		outResults.push_back(*itr);
	}
}

uint PixelFormat::calcNextPot(uint x)
{
	x = x - 1;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >>16);
	return x + 1;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
