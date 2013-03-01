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

#include "nitbundler/Handler.h"

#include "nit/content/Image.h"

#include "freeimage/FreeImage.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class NITBUNDLER_API ImageZen : public RefCounted
{
public:
	ImageZen(StreamSource* source);

	struct Options
	{
		ContentType						contentType;
		PixelFormat						format;
		int								width;
		int								height;
		bool							makePOT;
		bool							makeSquare;
		bool							makeMipmaps;
		bool							sharpen;
		int								minWidthHeight;
		int								alignWidthHeight;
		uint16							pivotX;
		uint16							pivotY;
		bool							flipEndian;
	};

public:
	StreamSource*						getSource()								{ return _source; }
	ImageZen*							clone();

	bool								load();

	FIBITMAP*							getBitmap()								{ return _bitmap; }
	FREE_IMAGE_FORMAT					getSourceFormat()						{ return _sourceFormat; }

	int									getWidth()								{ return FreeImage_GetWidth(_bitmap); }
	int									getHeight()								{ return FreeImage_GetHeight(_bitmap); }

	void								process(Options& options);
	uint32								write(StreamWriter* w, Options& opt, StreamWriter* dumpFile = NULL);

	void								rescale(int dstWidth, int dstHeight, FREE_IMAGE_FILTER filter = FILTER_CATMULLROM);
	void								enlargeCanvas(int left, int top, int right, int bottom, uint32 color = 0);
	void								enlargeCanvas(int width, int height, uint32 color = 0);

	void								write(StreamWriter* w, PixelFormat format);

	static FREE_IMAGE_FORMAT			FIF_From(ContentType ct);
	static ContentType					FIF_To(FREE_IMAGE_FORMAT fif);

protected:
	void*								_handle;

protected:
	Ref<StreamSource>					_source;

	virtual void						onDelete();
	FIBITMAP*							_bitmap;
	FREE_IMAGE_FORMAT					_sourceFormat;
	uint16								_sourceWidth;
	uint16								_sourceHeight;
};

////////////////////////////////////////////////////////////////////////////////

class NITBUNDLER_API ImageHandler : public Handler
{
public:
	ImageHandler();

protected:
	virtual void						setCodec(const String& codec);
	virtual void						setPayload(const String& payload);
	virtual void						setResize(const String& resize);
	virtual void						generate();

protected:
	StreamWriter*						createDumpFile(String ext = StringUtil::BLANK(), String filename = StringUtil::BLANK(), bool convertExt = false);

	ImageZen::Options					_options;
	String								_codec;
	String								_resize;
	bool								_makeDump;
};

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
