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

#include "nit/content/Content.h"
#include "nit/content/ContentManager.h"

#include "nit/content/PixelFormat.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class Image;
class ImageManager;

////////////////////////////////////////////////////////////////////////////////

#define NIT_NTEX_SIGNATURE				NIT_MAKE_CC('N', 'T', 'E', 'X')
#define NIT_NTEX_SIGNATURE_FLIP			NIT_MAKE_CC('X', 'E', 'T', 'N')
#define NIT_NTEX_VERSION				NIT_MAKE_CC('1', '.', '0', '0')

////////////////////////////////////////////////////////////////////////////////

// NOTE: This class should contain the smallest set of features for runtime
// For complex features, Bundler::Image will implement them

class NIT_API Image : public Content
{
public:
	typedef ImageManager				ManagerClass;

public:
	struct NIT_API SigHeader
	{
		uint32							signature;			// NTEX
		uint32							version;			// Old version detection

		void							flipEndian();
	};

	struct NIT_API Header
	{
		uint8							extHeaderSize;		//  1  more bytes to read for extension header
		uint8							mipmapCount;		//  2  total mipmap count. at least 1.
		uint16							flags;				//  4  extension flag (ex: bordered)

		uint32							memorySize;			//  8  total memory occupation size for every surface + mipmap (except header)
		PixelFormat::ValueType			pixelFormat;		// 12  combination of PixelFormat flags

		// width and height of miplevel #0 (half for miplevel #1, ...)
		uint16							width;				// 14
		uint16							height;				// 16

		// dimension of original image (before resize or pot, as-is per original file)
		uint16							sourceWidth;		// 18
		uint16							sourceHeight;		// 20

		// resized content area (before pot or square)
		// non zero left or top if tex-packer or border applied
		uint16							contentLeft;		// 22
		uint16							contentTop;			// 24
		uint16							contentRight;		// 26
		uint16							contentBottom;		// 28

		// pack alignment to 32 bytes
		uint16							_reserved0;			// 30
		uint16							_reserved1;			// 32

		// ex: when applied to a sprite:
		// quad.rect:      { x, y, x+sw, y+sh }
		// quad.texcoord : { l/w, t/h, r/w, b/h }

		// 18x19 -> 32x32 pot-square
		// rect : {x, y, x+18, y+19}, tc : { 0/32, 0/32, 18/32, 19/32 }

		// 64x64 -> 32x32 resize
		// rect : {x, y, x+64, y+64}, tc : { 0/32, 0/32, 32/32, 32/32 }
		
		// 40x40 -> 20x20 resize -> 32x32 pot
		// rect : {x, y, x+40, y+40}, tc : { 0/32, 0/32, 20/32, 20/32 }

		// 128x128 -> 2x2 resize -> 8x8 pot
		// rect : {x, y, x+128, y+128}, tc : { 0/8, 0/8, 2/8, 2/8 }

		// 128x128 -> border 1
		// rect : {x, y, x+128, y+128}, tc : { 1/128, 1/128, 127/128, 127/128 }

		// 128x128 -> 64x64 resize -> border 1
		// rect : {x, y, x+128, y+128}, tc : { 1/64, 1/64, 63/64, 63/64 }

		// 32x32 -> 16x16 resize -> merge 256x256 (8, 8)
		// rect : {x, y, x+32, y+32}, tc : { 8/256, 8/256, (8+16)/256, (8+16)/256 }

		void							flipEndian();
	};

public:
	Image();
	Image(const Header& header, uint8* pixelBuffer = NULL); // pixelBuffer will be deallocated by Deallocate()
	Image(const Header& header, uint8* srcBuffer, int srcWidth, int srcHeight, int srcPitch, bool yFlip = false); // srcBuffer will be copied
	Image(StreamSource* source, ContentType treatAs = ContentType::UNKNOWN);

public:
	const Header&						getHeader()								{ return _header; }

	const ContentType&					getContentType()						{ return _contentType; }

	PixelFormat							getPixelFormat()						{ return _header.pixelFormat; }

	int									getBitsPerPixel()						{ return _bitsPerPixel; }
	int									getWidth()								{ return _header.width; }
	int									getHeight()								{ return _header.height; }
	int									getPitch()								{ return _pitch; }
	int									getByteCount()							{ return _pitch * _header.height; }
	uint8*								getData()								{ return getMipData(0); }

	uint16								getFlags()								{ return _header.flags; }

	uint16								getSourceWidth()						{ return _header.sourceWidth; }
	uint16								getSourceHeight()						{ return _header.sourceHeight; }

	uint16								getContentLeft()						{ return _header.contentLeft; }
	uint16								getContentTop()							{ return _header.contentTop; }
	uint16								getContentRight()						{ return _header.contentRight; }
	uint16								getContentBottom()						{ return _header.contentBottom; }

	int									getMipCount()							{ return _header.mipmapCount; }
	int									getMipWidth(int mipLevel)				{ return _header.width >> mipLevel; }
	int									getMipHeight(int mipLevel)				{ return _header.height >> mipLevel; }
	int									getMipPitch(int mipLevel)				{ return _pitch >> mipLevel; }
	int									getMipByteCount(int mipLevel)			{ return getMipPitch(mipLevel) * _header.height; }
	uint8*								getMipData(int mipLevel);

public:									// Image Utility 
	void								replaceHeader(const Header& header);

	// TODO: How about to remember each call to following functions and to reapply on reloading?
	void								discardMipmaps();
	virtual void						makeAlphaPremultiplied();
	virtual bool						makePot(bool square, uint16 min=4);
	virtual void						reorderAsRgba();
	virtual void						makeRgba_4444();

public:
	virtual void						loadHeader();

public:
	virtual void						SaveNtex(StreamWriter* writer, bool flipEndian = false);
	virtual void						SavePng(StreamWriter* writer);
	virtual void						SaveJpeg(StreamWriter* writer);

protected:								// Content impl
	virtual size_t						getMemorySize()							{ return _header.memorySize; }
	virtual void						onLoad(bool async);
	virtual void						onUnload();
	virtual void						onDispose();

protected:
	Header								_header;
	ContentType							_contentType;
	uint8*								_pixelBuffer;
	uint16								_pitch;
	uint8								_bitsPerPixel;

protected:								// Alloc customization : allocation into direct HWBuffer (dx?)
	virtual uint8*						Allocate(size_t size);
	virtual void						Deallocate(uint8* buffer, size_t size);

protected:
	void								loadNtex(StreamReader* reader, bool onlyHeader = false);
	void								loadPng(StreamReader* reader, bool onlyHeader = false);
	void								loadJpeg(StreamReader* reader, bool onlyHeader = false);
	void								loadGif(StreamReader* reader, bool onlyHeader = false);
	void								loadPvr(StreamReader* reader, bool onlyHeader = false);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API ImageManager : public ContentManager
{
public:
	const static ManagerID				MANAGER_ID = MGR_IMAGE;

public:
	ImageManager();

protected:
	ImageManager(const String& name, Package* package);

protected:								// Module Impl
	virtual void						onRegister();
	virtual void						onUnregister();

protected:								// ContentManager Impl
	virtual void						onInit();
	virtual void						onFinish();

	virtual ManagerID					getManagerID()							{ return MANAGER_ID; }
	virtual void						allContentTypes(vector<ContentType>::type& outResults);
	virtual float						loadOrder()								{ return 0.0f; }
	virtual float						getPriority()							{ return 0.0f; }
	virtual bool						canLink(StreamSource* source);
	virtual void						onDispose(Content* content);
	virtual Ref<Content>				createFrom(StreamSource* source);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
