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

class NIT_API PixelFormat
{
public:
	// ARGB_8888 = 0xAARRGGBB, BB GG RR AA in little endian - dx style
	// RGBA_8888 = RR GG BB AA, 0xAABBGGRR in little endian - gl style

	enum Flags
	{
		FLAG_BPP_MASK					= 0x000F0000,
		FLAG_ID_MASK					= 0x0000FFFF,
		FLAG_FLAGS_MASK					= 0xFFF00000,

		FLAG_1_BPP						= 0x00000000,
		FLAG_2_BPP						= 0x00010000,
		FLAG_4_BPP						= 0x00020000,
		FLAG_8_BPP						= 0x00030000,
		FLAG_16_BPP						= 0x00040000,
		FLAG_32_BPP						= 0x00050000,
		FLAG_64_BPP						= 0x00060000,
		FLAG_128_BPP					= 0x00070000,
		FLAG_24_BPP						= 0x00080000,

		FLAG_HAS_ALPHA					= 0x00100000, // has alpha channel?
		FLAG_PMUL_ALPHA					= 0x00200000, // alpha pre-multiplied?
		FLAG_COMPRESSED					= 0x00400000, // can't access pixel-wise because of compression
		FLAG_GRAYSCALE					= 0x00800000, // is grey-scale?
		FLAG_RGBA_ORDER					= 0x01000000, // 0xBBGGRRAA (gl) or  0xRRGGBBAA (dx) ?
		FLAG_TILED						= 0x02000000, // is tiled format for some console hardwares?
	};

	enum Formats
	{
		// common
		UNKNOWN							= 0,

		RGB_888							= 0x0001 | FLAG_24_BPP,
		RGB_565							= 0x0002 | FLAG_16_BPP,
		A8								= 0x0003 | FLAG_8_BPP | FLAG_HAS_ALPHA | FLAG_GRAYSCALE,
		L8								= 0x0004 | FLAG_8_BPP | FLAG_GRAYSCALE,
		AL_88							= 0x0005 | FLAG_16_BPP | FLAG_HAS_ALPHA | FLAG_GRAYSCALE,

		AL_88_PA						= AL_88 | FLAG_PMUL_ALPHA,

		// DX-style : ARGB -> 0xAARRGGBB, BB GG RR AA in little endian (also web-style)
		ARGB_8888						= 0x0040 | FLAG_32_BPP | FLAG_HAS_ALPHA,					
		ARGB_4444						= 0x0041 | FLAG_16_BPP | FLAG_HAS_ALPHA,
		ARGB_1555						= 0x0042 | FLAG_16_BPP | FLAG_HAS_ALPHA,
		XRGB_8888						= 0x0043 | FLAG_32_BPP,
		XRGB_4444						= 0x0044 | FLAG_16_BPP,
		DXT1							= 0x0046 | FLAG_4_BPP | FLAG_COMPRESSED, 
		DXT3							= 0x0048 | FLAG_8_BPP | FLAG_COMPRESSED | FLAG_HAS_ALPHA,
		DXT5							= 0x004A | FLAG_8_BPP | FLAG_COMPRESSED | FLAG_HAS_ALPHA,

		// NOTE: 
		// DXT1: This does have 1 bit alpha but we don't use it for now.
		// DXT2: This is actually DXT3 + pa, we don't use it for now.
		// DXT4: This is actually DXT5 + pa, we don't use it for now.

		ARGB_8888_PA					= ARGB_8888 | FLAG_PMUL_ALPHA,
		ARGB_4444_PA					= ARGB_4444 | FLAG_PMUL_ALPHA,
		ARGB_1555_PA					= ARGB_1555 | FLAG_PMUL_ALPHA,
		DXT3_PA							= DXT3 | FLAG_PMUL_ALPHA,
		DXT5_PA							= DXT5 | FLAG_PMUL_ALPHA,

		// GL-style : RR GG BB AA, 0xAABBGGRR in little endian
		RGBA_8888						= 0x0080 | FLAG_32_BPP | FLAG_HAS_ALPHA | FLAG_RGBA_ORDER,
		BGRA_8888						= 0x0081 | FLAG_32_BPP | FLAG_HAS_ALPHA,					// power-vr only
		RGBA_4444						= 0x0082 | FLAG_16_BPP | FLAG_HAS_ALPHA | FLAG_RGBA_ORDER,
		RGBA_5551						= 0x0083 | FLAG_16_BPP | FLAG_HAS_ALPHA | FLAG_RGBA_ORDER,
		PVR2							= 0x0084 | FLAG_2_BPP | FLAG_COMPRESSED,					// power-vr only
		PVR2A							= 0x0085 | FLAG_2_BPP | FLAG_COMPRESSED | FLAG_HAS_ALPHA,	// power-vr only
		PVR4							= 0x0086 | FLAG_4_BPP | FLAG_COMPRESSED,					// power-vr only
		PVR4A							= 0x0087 | FLAG_4_BPP | FLAG_COMPRESSED | FLAG_HAS_ALPHA,	// power-vr only
		ETC								= 0x0088 | FLAG_8_BPP | FLAG_COMPRESSED,					// ericsson texture compression

		RGBA_8888_PA					= RGBA_8888 | FLAG_PMUL_ALPHA,
		BGRA_8888_PA					= BGRA_8888 | FLAG_PMUL_ALPHA,
		RGBA_4444_PA					= RGBA_4444 | FLAG_PMUL_ALPHA,
		RGBA_5551_PA					= RGBA_5551 | FLAG_PMUL_ALPHA,
		PVR2A_PA						= PVR2A | FLAG_PMUL_ALPHA,
		PVR4A_PA						= PVR4A | FLAG_PMUL_ALPHA,

		// TODO: Investigate PVRTC4, PVRTC2 if premultiplied (implicit alpha), or there's separate format for alpha
		// TODO: tiled formats for console hardwares (Does ever those good-hell days come back?)
		// TODO: Reinvestigate PF_ETC format
	};

	typedef uint32 ValueType;

public:
	const String&						getName() const;
	uint32								getValue() const						{ return _value; }
	uint16								getId() const							{ return _value & FLAG_ID_MASK; }
	bool								hasAlpha() const						{ return (_value & FLAG_HAS_ALPHA) != 0; }
	bool								isAlphaPremultiplied() const			{ return (_value & FLAG_PMUL_ALPHA) != 0; }
	bool								isCompressed() const					{ return (_value & FLAG_COMPRESSED) != 0; }
	bool								isGrayscale() const						{ return (_value & FLAG_GRAYSCALE) != 0; }
	bool								isOrderedAsRgba() const					{ return (_value & FLAG_RGBA_ORDER) != 0; }
	bool								isTiled() const							{ return (_value & FLAG_TILED) != 0; }

public:
	static int							calcBitsPerPixel(PixelFormat pixelFormat);
	static int							calcPitch(PixelFormat pixelFormat, int width);
	static uint							calcNextPot(uint width);

	static void							allFormats(vector<std::pair<String, PixelFormat> >::type& outResults);

public:
	PixelFormat() : _value(UNKNOWN)												{ }
	PixelFormat(ValueType value) : _value(value)								{ }
	PixelFormat(const String& name);
	
	operator ValueType() const													{ return _value; }

private:
	ValueType							_value;
};

////////////////////////////////////////////////////////////////////////////////

// TODO: Remove all and add a mask calc utility func

struct PixelRGBA_8888
{
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;

	enum { A_BITS = 8, R_BITS = 8, G_BITS = 8, B_BITS = 8 };

	static bool checkSize()				{ return sizeof(PixelRGBA_8888) == 4; }
};

////////////////////////////////////////////////////////////////////////////////

struct PixelARGB_8888
{
	uint8 b;
	uint8 g;
	uint8 r;
	uint8 a;

	enum { A_BITS = 8, R_BITS = 8, G_BITS = 8, B_BITS = 8 };

	static bool checkSize()				{ return sizeof(PixelRGBA_8888) == 4; }
};

////////////////////////////////////////////////////////////////////////////////

struct PixelRGBA_4444
{
	uint8 r : 4;
	uint8 g : 4;
	uint8 b : 4;
	uint8 a : 4;

	enum { A_BITS = 4, R_BITS = 4, G_BITS = 4, B_BITS = 4 };

	static bool checkSize()				{ return sizeof(PixelRGBA_4444) == 2; }
};

////////////////////////////////////////////////////////////////////////////////

// TODO: Investigate big endian platforms

struct PixelARGB_4444
{
	uint8 b : 4;
	uint8 g : 4;
	uint8 r : 4;
	uint8 a : 4;

	enum { A_BITS = 4, R_BITS = 4, G_BITS = 4, B_BITS = 4 };

	static bool checkSize()				{ return sizeof(PixelRGBA_4444) == 2; }
};

////////////////////////////////////////////////////////////////////////////////

// TODO: Specify by platform SysConfig
#pragma pack (push, 1)

struct PixelRGB_888
{
	uint8 r;
	uint8 g;
	uint8 b;

	enum { R_BITS = 8, G_BITS = 8, B_BITS = 8 };

	static bool checkSize()				{ return sizeof(PixelRGB_888) == 3; }
};

#pragma pack (pop)

// RGBA_5551, RGB_555 not supported (some compilers does not support such a packing)

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
