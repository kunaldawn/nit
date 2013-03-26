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

#include "nit/content/Image.h"

#include "nit/content/ContentManager.h"

#include "nit/io/MemoryBuffer.h"
#include "nit/io/ZStream.h"

#include "libpng/png.h"
#include "libjpeg/jpeglib.h"
#include "libungif/gif_lib.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define RGB_PREMULTIPLY_ALPHA(vr, vg, vb, va) \
	(unsigned)(((unsigned)((unsigned char)(vr) * ((unsigned char)(va) + 1)) >> 8) | \
	((unsigned)((unsigned char)(vg) * ((unsigned char)(va) + 1) >> 8) << 8) | \
	((unsigned)((unsigned char)(vb) * ((unsigned char)(va) + 1) >> 8) << 16) | \
	((unsigned)(unsigned char)(va) << 24))

////////////////////////////////////////////////////////////////////////////////

Image::Image()
: Content(NULL), _pixelBuffer(NULL), _pitch(0), _bitsPerPixel(0)
{
	memset(&_header, 0, sizeof(_header));
}

Image::Image(const Header& header, uint8* pixelBuffer)
: Content(NULL), _header(header), _pixelBuffer(pixelBuffer)
{
	_bitsPerPixel = PixelFormat::calcBitsPerPixel(header.pixelFormat);
	_pitch = PixelFormat::calcPitch(header.pixelFormat, header.width);

	_linked = _loaded = pixelBuffer != NULL;
}

Image::Image(StreamSource* source, ContentType treatAs)
: Content(NULL), _pixelBuffer(NULL), _pitch(0), _bitsPerPixel(0)
{
	memset(&_header, 0, sizeof(_header));

	_source = source;
	_contentType = treatAs;

	_linked = true;
}

Image::Image(const Header& header, uint8* srcBuffer, int srcWidth, int srcHeight, int srcPitch, bool yFlip)
: Content(NULL), _header(header)
{
	_bitsPerPixel = PixelFormat::calcBitsPerPixel(header.pixelFormat);
	_pitch = PixelFormat::calcPitch(header.pixelFormat, header.width);

	uint32 memSize = _header.height * _pitch;
	ASSERT_THROW(memSize == _header.memorySize, EX_INVALID_PARAMS);

	_pixelBuffer = Allocate(memSize);

	int copyPitch = _pitch > srcPitch ? srcPitch : _pitch;
	int copyHeight = _header.height > srcHeight ? srcHeight : _header.height;

	if (!yFlip && srcPitch == copyPitch)
	{
		memcpy(_pixelBuffer, srcBuffer, copyHeight * copyPitch);
	}
	else if (yFlip)
	{
		for (int i = 0; i < copyHeight; ++i)
		{
			uint8* srcRow = &srcBuffer[(srcHeight - i) * srcPitch];
			uint8* dstRow = &_pixelBuffer[i * _pitch];
			memcpy(dstRow, srcRow, copyPitch);
		}
	}
	else
	{
		for (int i = 0; i < copyHeight; ++i)
		{
			uint8* srcRow = &srcBuffer[i * srcPitch];
			uint8* dstRow = &_pixelBuffer[i * _pitch];
			memcpy(dstRow, srcRow, copyPitch);
		}
	}

	_linked = true;
	_loaded = true;
}

void Image::loadHeader()
{
	if (_headerLoaded) return;

	ASSERT_THROW(_source, EX_INVALID_STATE);

	// TODO: implement a codec register mechanism
	if (_source && _contentType == ContentType::UNKNOWN)
		_contentType = _source->getContentType();

	// TODO: check if libjpeg can handle PPM, BMP, TGA

	switch (_contentType)
	{
	case ContentType::IMAGE_NTEX:		loadNtex(_source->open(), true); break;

	case ContentType::IMAGE_PNG:		loadPng(_source->open(), true); break;

	case ContentType::IMAGE_JPEG:		loadJpeg(_source->open(), true); break;

	case ContentType::IMAGE_GIF:		loadGif(_source->open(), true); break;

	case ContentType::IMAGE_CCZ:		
	case ContentType::IMAGE_PVR:		loadPvr(_source->open(), true); break;

	default:
		NIT_THROW(EX_NOT_SUPPORTED);
	}

	_headerLoaded = true;

	if (_pixelBuffer)
	{
		Deallocate(_pixelBuffer, _header.memorySize);
		_pixelBuffer = NULL;
	}
}

void Image::onLoad(bool async)
{
	ASSERT_THROW(_source, EX_INVALID_STATE);

	// TODO: implement a codec register mechanism
	if (_source && _contentType == ContentType::UNKNOWN)
		_contentType = _source->getContentType();

	// TODO: check if libjpeg can handle PPM, BMP, TGA

	switch (_contentType)
	{
	case ContentType::IMAGE_NTEX:		loadNtex(_source->open()); break;

	case ContentType::IMAGE_PNG:		loadPng(_source->open()); break;

	case ContentType::IMAGE_JPEG:		loadJpeg(_source->open()); break;

	case ContentType::IMAGE_GIF:		loadGif(_source->open()); break;

	case ContentType::IMAGE_CCZ:		
	case ContentType::IMAGE_PVR:		loadPvr(_source->open()); break;

	default:
		NIT_THROW(EX_NOT_SUPPORTED);
	}

	_headerLoaded = true;
}

void Image::onUnload()
{
	if (_pixelBuffer)
	{
		Deallocate(_pixelBuffer, _header.memorySize);
		_pixelBuffer = NULL;
	}
}

void Image::onDispose()
{
	unload();

	_contentType = ContentType::UNKNOWN;
}

uint8* Image::Allocate(size_t size)
{
	return (uint8*)NIT_ALLOC(size);
}

void Image::Deallocate(uint8* buffer, size_t size)
{
	NIT_DEALLOC(buffer, size);
}

uint8* Image::getMipData(int mipLevel)
{
	// TODO: Reverse the mipMap order - it must be useful for streamized loading

	if (mipLevel == 0)
		return _pixelBuffer;

	if (mipLevel < 0 || mipLevel >= _header.mipmapCount)
		return NULL;

	int pitch = _pitch;
	int height = _header.height;

	uint8* start = _pixelBuffer;

	for (int i=0; i<mipLevel; ++i)
	{
		start += pitch * height;
		pitch >>= 1;
		height >>= 1;
	}

	return start;
}

void Image::loadNtex(StreamReader* reader, bool onlyHeader)
{
	Ref<StreamReader> safe = reader;

	// Check signature and header
	SigHeader sig = { 0 };

	bool flipEndian = false;

	if (reader->readRaw(&sig, sizeof(sig)) == sizeof(sig))
	{
		if (sig.signature == NIT_NTEX_SIGNATURE_FLIP)
			flipEndian = true;

		if (flipEndian)
			sig.flipEndian();
	}
	else
	{
		sig.signature = 0;
	}

	if (sig.signature != NIT_NTEX_SIGNATURE)
	{
		_error = true;
		NIT_THROW_FMT(EX_READ, "'%s': invalid ntex signature", reader->getUrl().c_str());
	}

	if (sig.version != NIT_NTEX_VERSION)
	{
		_error = true;
		NIT_THROW_FMT(EX_READ, "'%s': not supported ntex version", reader->getUrl().c_str());
	}

	Header hdr = { 0 };

	if (reader->readRaw(&hdr, sizeof(hdr)) != sizeof(hdr))
	{
		_error = true;
		NIT_THROW_FMT(EX_READ, "'%s': invalid ntex header", reader->getUrl().c_str());
	}

	if (flipEndian)
		hdr.flipEndian();

	if (!onlyHeader)
	{
		// load raw pixel data to buffer
		_pixelBuffer = Allocate(hdr.memorySize);

		if (_pixelBuffer == NULL)
		{
			_error = true;
			NIT_THROW_FMT(EX_MEMORY, "'%s': can't allocate %d bytes", reader->getUrl().c_str(), hdr.memorySize);
		}

		if (reader->readRaw(_pixelBuffer, hdr.memorySize) != hdr.memorySize)
		{
			_error = true;
			Deallocate(_pixelBuffer, hdr.memorySize);
			NIT_THROW_FMT(EX_READ, "'%s': data truncated", reader->getUrl().c_str());
		}
	}

	_header = hdr;
	_bitsPerPixel = PixelFormat::calcBitsPerPixel(hdr.pixelFormat);
	_pitch = PixelFormat::calcPitch(hdr.pixelFormat, hdr.width);

	// At this point, image data loaed as is per bundler prepared

	// TODO: perform flipEndian if needed
}

static void _PNG_ReaderCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	StreamReader* reader = (StreamReader*)png_get_io_ptr(png_ptr);

	size_t bytesRead = reader->readRaw(data, length);
	if (bytesRead != length)
		png_error(png_ptr, "file truncated");
}

static void _png_OnError(png_structp png_ptr, png_const_charp errStr)
{
	String* err = (String*)png_get_error_ptr(png_ptr);
	*err = errStr;
}

static void _png_OnWarning(png_structp png_ptr, png_const_charp errStr)
{
	LOG(0, "*** png read error: %s\n", errStr);
}

void Image::loadPng(StreamReader* reader, bool onlyHeader)
{
	Ref<StreamReader> safe = reader;

	// Declare every local variables before encounter to first goto statement. (android GCC needs this)
	uint8* imageData = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	uint32& pf = _header.pixelFormat;
	int pitch;
	int bytesPerPixel;
	png_bytep* rowPointers;
	uint32 memSize;
	int colorType = 0;
	png_uint_32 width = 0;
	png_uint_32 height = 0;
	int bitsPerComponent = 0;

	String errStr;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, &errStr, _png_OnError, _png_OnWarning);
	if (png_ptr == NULL)
		goto error;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		goto error;

	if (setjmp(png_jmpbuf(png_ptr)))
		goto error;

	// setup callback function
	png_set_read_fn(png_ptr, reader, _PNG_ReaderCallback);

	if (onlyHeader)
	{
//		LOG(0, "++ info load: '%s'\n", reader->GetName().c_str());
		png_read_info(png_ptr, info_ptr);
	}
	else
	{
//		LOG(0, "++ full load: '%s'\n", reader->GetName().c_str());

		// read png
		// PNG_TRANSFORM_EXPAND: perform set_expand()
		// PNG_TRANSFORM_PACKING: expand 1, 2 and 4-bit samples to bytes
		// PNG_TRANSFORM_STRIP_16: strip 16-bit samples to 8 bits
		// PNG_TRANSFORM_GRAY_TO_RGB: expand grayscale samples to RGB (or GA to RGBA)
		png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_PACKING 
			| PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_GRAY_TO_RGB, 0);
	}

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitsPerComponent, &colorType, 0, 0, 0);

	if (bitsPerComponent != 8 && bitsPerComponent != 16)
	{
		// NOTE: That's ok for 16 bpp as PNG_TRANSFORM_STRIP_16 specified
		errStr = "bpp must be 8 or 16";
		goto error;
	}

	// init image info
	pf = 0;

	if (png_get_color_type(png_ptr, info_ptr) & PNG_COLOR_MASK_ALPHA)
		pf |= PixelFormat::FLAG_HAS_ALPHA;
	
	// allocate memory and read data
	bytesPerPixel = 3;
	if (pf & PixelFormat::FLAG_HAS_ALPHA)
		bytesPerPixel = 4;

	memSize = height * width * bytesPerPixel;

	if (!onlyHeader)
	{
		imageData = Allocate(memSize);
		if (imageData == NULL) goto error;

		rowPointers = png_get_rows(png_ptr, info_ptr);
	}

	// copy image data to buffer
	pitch = width * bytesPerPixel;

	if (pf & PixelFormat::FLAG_HAS_ALPHA)
	{
		// apply premultiplied alpha
		// TODO: expose an option to user code
		pf |= PixelFormat::RGBA_8888_PA;

		if (!onlyHeader)
		{
			uint* tmp = (uint*)imageData;
			for (uint y = 0; y < height; ++y)
			{
				for (int x = 0; x < pitch; x+=4)
				{
					*tmp++ = RGB_PREMULTIPLY_ALPHA(rowPointers[y][x], rowPointers[y][x+1], rowPointers[y][x+2], rowPointers[y][x+3]);
				}
			}
		}
	}
	else
	{
		pf |= PixelFormat::RGB_888;

		if (!onlyHeader)
		{
			for (uint y = 0; y < height; ++y)
			{
				memcpy(imageData + y * pitch, rowPointers[y], pitch);
			}
		}
	}

	_header.width			= width;
	_header.height			= height;
	_header.memorySize		= memSize;

	_header.sourceWidth	= width;
	_header.sourceHeight	= height;

	_header.contentLeft	= 0;
	_header.contentTop		= 0;
	_header.contentRight	= width;
	_header.contentBottom	= height;

	_header.mipmapCount	= 1;

	_pitch					= pitch;
	_bitsPerPixel			= bytesPerPixel * 8;

	_pixelBuffer			= imageData;

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	return;

error:
	if (imageData)
		Deallocate(imageData, 0);

	if (png_ptr)
		png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : NULL, 0);

	if (errStr.empty())
		errStr = "can't read png";

	_error = true;
	NIT_THROW_FMT(EX_READ, "'%s': %s", reader->getUrl().c_str(), errStr.c_str());
}

static void _jpeg_OnErrorExit(j_common_ptr cinfo)
{
	char buf[512];
	cinfo->err->format_message(cinfo, buf);
	NIT_THROW_FMT(EX_READ, "%s", buf);
}

void Image::loadJpeg(StreamReader* reader, bool onlyHeader)
{
	// TODO: implmement a header only reading

	Ref<StreamReader> safe = reader;

	MemoryBuffer::Access mem(reader->buffer());

	JSAMPROW row_pointer[1] = { NULL };
	uint8* imageData = NULL;
	ulong location;
	uint32 memSize;

	String errStr;

	// Initialize jpeg calling
	jpeg_decompress_struct cinfo = { 0 };
	jpeg_error_mgr jerr = { 0 };
	jpeg_std_error(&jerr);
	jerr.error_exit = _jpeg_OnErrorExit;
	cinfo.err = &jerr;
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, (uint8*)mem.getMemory(), mem.getSize());

	// Determine header
	try
	{
		if (jpeg_read_header(&cinfo, false) != JPEG_HEADER_OK)
			goto error;
	}
	catch (Exception& ex)
	{
		errStr = ex.getFullDescription();
		goto error;
	}

	if (cinfo.jpeg_color_space == JCS_GRAYSCALE || cinfo.jpeg_color_space == JCS_YCbCr)
		cinfo.out_color_space = JCS_RGB;

	if (cinfo.out_color_space != JCS_RGB)
		goto error;

	// Decompress jpeg
	jpeg_start_decompress(&cinfo);

	assert(cinfo.output_components == 3);

	// Initialize image header
	memSize = cinfo.output_width * cinfo.output_height * cinfo.output_components;

	_header.pixelFormat	= PixelFormat::RGB_888;
	_header.width			= cinfo.image_width;
	_header.height			= cinfo.image_height;
	_header.memorySize		= memSize;

	_header.sourceWidth	= _header.width;
	_header.sourceHeight	= _header.height;

	_header.contentLeft	= 0;
	_header.contentTop		= 0;
	_header.contentRight	= _header.width;
	_header.contentBottom	= _header.height;

	_header.mipmapCount	= 1;

	_bitsPerPixel			= cinfo.output_components * 8;
	_pitch					= cinfo.output_width * cinfo.output_components;
	
	row_pointer[0] = (JSAMPROW)NIT_ALLOC(_pitch);
	if (row_pointer[0] == NULL)
		goto error;

	imageData = Allocate(memSize);
	if (imageData == NULL)
		goto error;

	// write to image buffer sequentially by reading scanline
	location = 0;
	while (cinfo.output_scanline < cinfo.image_height)
	{
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		for (uint i=0; i < cinfo.image_width * cinfo.num_components; ++i)
			imageData[location++] = row_pointer[0][i];
	}

	_pixelBuffer = imageData;

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	if (row_pointer[0])
		NIT_DEALLOC(row_pointer[0], _pitch);

	return;

error:
	if (row_pointer[0])
		NIT_DEALLOC(row_pointer[0], 0);
	if (imageData)
		Deallocate(imageData, 0);

	jpeg_destroy_decompress(&cinfo);

	if (errStr.empty())
		errStr = "can't read jpeg";

	_error = true;
	NIT_THROW_FMT(EX_READ, "'%s': %s\n", reader->getUrl().c_str(), errStr.c_str());
}

static int _GIF_ReadFunc(GifFileType* gifFile, GifByteType* buf, int count)
{
	StreamReader* r = (StreamReader*)gifFile->UserData;
	return r->readRaw(buf, count);
}

#define GIF_RGBA_5551(r, g, b, a) ( (((r)>>3)<<11) | (((g)>>3)<<6) | (((b)>>3))<<1 | a)
#define GIF_GAMMA(x)      (x)
#define GIF_TRANSPARENT		0x01
#define GIF_NOT_TRANSPARENT	-1

static const short s_gifInterlacedOffset[] = { 0, 4, 2, 1 }; /* The way Interlaced image should. */
static const short s_gifInterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */

void Image::loadGif(StreamReader* reader, bool onlyHeader)
{
	// TODO: implement a header only reading

	Ref<StreamReader> safe = reader;

	int gifErr = 0;

	GifFileType* gifFile = NULL;
	GifRecordType recordType;
	GifByteType* extension = NULL;
	ColorMapObject* colorMap = NULL;

	int	i, j, row, col, width, height, pitch, count, extCode;
	int transparent = GIF_NOT_TRANSPARENT;

	uint8* lineBuf = NULL;
	uint8* imageData = NULL;
	uint32 memSize;

	bool hasImage = false;

	gifFile = DGifOpen(reader, _GIF_ReadFunc);
	if (gifFile == NULL)
		goto error;

	uint16 palette[256];

	// TODO: nit-ify code

	// GIF file can contain more than one image
	while (!hasImage)
	{
		if (DGifGetRecordType(gifFile, &recordType) != GIF_OK)
			goto error;

		if (recordType == TERMINATE_RECORD_TYPE)
			break;

		switch (recordType)
		{
		case TERMINATE_RECORD_TYPE:
		case UNDEFINED_RECORD_TYPE:
		case SCREEN_DESC_RECORD_TYPE:
			break;

		case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(gifFile) != GIF_OK)
				goto error;

			row = gifFile->Image.Top; // Image position relative to screen
			col = gifFile->Image.Left;
			width = gifFile->Image.Width;
			height = gifFile->Image.Height;

			colorMap = gifFile->Image.ColorMap ? gifFile->Image.ColorMap : gifFile->SColorMap;

			if (colorMap->BitsPerPixel > 8)
			{
				// GIF supports max 8 bpp, so value above is a broken one.
				LOG(0, "*** bits per pixel > 8\n");
				gifErr = D_GIF_ERR_WRONG_RECORD;
				goto error;
			}

			if (gifFile->Image.Left + gifFile->Image.Width > gifFile->SWidth ||
				gifFile->Image.Top + gifFile->Image.Height > gifFile->SHeight)
			{
				gifErr = D_GIF_ERR_WRONG_RECORD;
				goto error;
			}

			// setup ARGB1555 palette
			for (i = colorMap->ColorCount; --i >= 0; )
			{
				GifColorType* color = &colorMap->Colors[i];
				palette[i] = GIF_RGBA_5551(GIF_GAMMA(color->Red), GIF_GAMMA(color->Green), GIF_GAMMA(color->Blue), 1);
			}

			if (0 <= transparent && transparent < 256)
				palette[transparent] = 0;

			lineBuf = Allocate(width);

			pitch = width * 2;
			memSize = pitch * height;
			imageData = Allocate(memSize);

			if (row != 0 || col != 0)
			{
				// We should clear entire buffer - ungif doesn't do it
				memset(imageData, 0, memSize);
			}

			// Initialize image header
			_header.pixelFormat	= PixelFormat::RGBA_5551_PA;
			_header.width			= width;
			_header.height			= height;
			_header.memorySize		= memSize;

			_header.sourceWidth	= _header.width;
			_header.sourceHeight	= _header.height;

			_header.contentLeft	= 0;
			_header.contentTop		= 0;
			_header.contentRight	= _header.width;
			_header.contentBottom	= _header.height;

			_header.mipmapCount	= 1;

			_bitsPerPixel			= 16;
			_pitch					= pitch;

			if (gifFile->Image.Interlace)
			{
				for (count = i = 0; i < 4; ++i)
				{
					for (j = row + s_gifInterlacedOffset[i]; j < row + height; j += s_gifInterlacedJumps[i])
					{
						if (DGifGetLine(gifFile, lineBuf, width) != GIF_OK) 
							goto error;

						uint16* scanLine = (uint16*)&imageData[j * pitch + col];
						for (int p = 0; p < width; ++p)
						{
							*scanLine++ = palette[lineBuf[p]];
						}
					}
				}
			}
			else
			{
				for (i = 0; i < height; ++i)
				{
					if (DGifGetLine(gifFile, lineBuf, width) != GIF_OK) 
						goto error;

					uint16* scanLine = (uint16*)&imageData[i * pitch + col];
					for (int p = 0; p < width; ++p)
					{
						*scanLine++ = palette[lineBuf[p]];
					}
				}
			}

			Deallocate(lineBuf, width);
			_pixelBuffer = imageData;
			hasImage = true;

			lineBuf = NULL;
			imageData = NULL;
			break;

		case EXTENSION_RECORD_TYPE:
			// Transparent pixel colorkey is contained in extension block
			if (DGifGetExtension(gifFile, &extCode, &extension) != GIF_OK)
				goto error;

			if (extCode == GRAPHICS_EXT_FUNC_CODE)
			{
				if (extension[0] != 4)
				{ 
					gifErr = D_GIF_ERR_WRONG_RECORD;
					goto error;
				}

				int flag = extension[1];
				transparent = (flag & GIF_TRANSPARENT) ? extension[4] : GIF_NOT_TRANSPARENT;
			}

			// ignore other informations
			while (extension)
			{
				if (DGifGetExtensionNext(gifFile, &extension) != GIF_OK)
					goto error;
			}
			break;
		}
	}

	if (DGifCloseFile(gifFile) != GIF_OK)
	{
		gifFile = NULL;
		goto error;
	}

	return;

error:
	_error = true;

	const char* errStr = "<unknown>";

	if (gifErr == 0)
		gifErr = GifLastError();

	switch (gifErr)
	{
	case D_GIF_ERR_OPEN_FAILED:			errStr = "open failed"; break;
	case D_GIF_ERR_READ_FAILED:			errStr = "read failed"; break;
	case D_GIF_ERR_NOT_GIF_FILE:		errStr = "not gif file"; break;
	case D_GIF_ERR_NO_SCRN_DSCR:		errStr = "no screen description"; break;
	case D_GIF_ERR_NO_IMAG_DSCR:		errStr = "no image description"; break;
	case D_GIF_ERR_NO_COLOR_MAP:		errStr = "no color map"; break;
	case D_GIF_ERR_WRONG_RECORD:		errStr = "wrong record"; break;
	case D_GIF_ERR_DATA_TOO_BIG:		errStr = "data too big"; break;
	case D_GIF_ERR_NOT_ENOUGH_MEM:		errStr = "not enough memory"; break;
	case D_GIF_ERR_CLOSE_FAILED:		errStr = "close failed"; break;
	case D_GIF_ERR_NOT_READABLE:		errStr = "not readable"; break;
	case D_GIF_ERR_IMAGE_DEFECT:		errStr = "image has defection"; break;
	case D_GIF_ERR_EOF_TOO_SOON:		errStr = "eof too soon"; break;
	}

	if (gifFile)
		DGifCloseFile(gifFile);

	if (lineBuf)
		Deallocate(lineBuf, 0);

	if (imageData)
		Deallocate(imageData, 0);

	NIT_THROW_FMT(EX_READ, "'%s': %s\n", reader->getUrl().c_str(), errStr);
}

struct CCZHeader 
{
	uint32	Signature;
	uint16	CompressionType;
	uint16	Version;
	uint32	Reserved;
	uint32	Len;

	void Flip()
	{
		StreamUtil::flipEndian(Signature);
		StreamUtil::flipEndian(CompressionType);
		StreamUtil::flipEndian(Version);
		StreamUtil::flipEndian(Reserved);
		StreamUtil::flipEndian(Len);
	}
};

struct PVRTexHeader
{
	uint32	HeaderLength;
	uint32	Height;
	uint32	Width;
	uint32	NumMipmaps;
	uint32	Flags;
	uint32	DataLength;
	uint32	Bpp;
	uint32	MaskRed;
	uint32	MaskGreen;
	uint32	MaskBlue;
	uint32	MaskAlpha;
	uint32	Signature;
	uint32	NumSurfaces;

	bool HasMipMap()					{ return (Flags & 0x0000100) != 0; }
	bool IsTwiddled()					{ return (Flags & 0x0000200) != 0; }
	bool HasBumpmap()					{ return (Flags & 0x0000400) != 0; }
	bool IsBordered()					{ return (Flags & 0x0000800) != 0; }
	bool IsCubeMap()					{ return (Flags & 0x0001000) != 0; }
	bool IsMipmapFake()					{ return (Flags & 0x0002000) != 0; }
	bool IsVolume()						{ return (Flags & 0x0004000) != 0; }
	bool HasAlpha()						{ return (Flags & 0x0008000) != 0; }
	bool IsVertFlipped()				{ return (Flags & 0x0010000) != 0; }

	enum Format
	{
		RGBA_4444						= 0x10,
		RGBA_5551,
		RGBA_8888,
		RGB_565,
		RGB_555,						// not supported
		RGB_888,
		I_8,
		AI_88,
		PVRTC_2,
		PVRTC_4,
		BGRA_8888,
		A_8,
	};

	Format GetFormat()					{ return Format(Flags & 0xFF); }

	void Flip()
	{
		StreamUtil::flipEndian(HeaderLength);
		StreamUtil::flipEndian(Height);
		StreamUtil::flipEndian(Width);
		StreamUtil::flipEndian(NumMipmaps);
		StreamUtil::flipEndian(Flags);
		StreamUtil::flipEndian(DataLength);
		StreamUtil::flipEndian(Bpp);
		StreamUtil::flipEndian(MaskRed);
		StreamUtil::flipEndian(MaskGreen);
		StreamUtil::flipEndian(MaskBlue);
		StreamUtil::flipEndian(MaskAlpha);
		StreamUtil::flipEndian(Signature);
		StreamUtil::flipEndian(NumSurfaces);
	}
};

void Image::loadPvr(StreamReader* givenReader, bool onlyHeader)
{
	Ref<StreamReader> safe = givenReader;

	Ref<StreamReader> reader = givenReader;

	String errStr;

	uint32 size = 0;
	uint8* data = NULL;
	PixelFormat fmt = PixelFormat::UNKNOWN;

	if (reader->getSource()->getContentType() == ContentType::IMAGE_CCZ)
	{
		// check CCZ Header
		CCZHeader hdr;
		if (reader->readRaw(&hdr, sizeof(hdr)) != sizeof(hdr))
		{
			errStr = "invalid ccz header";
			goto error;
		}

		if (hdr.Signature == NIT_MAKE_CC('C', 'C', 'Z', '!'))
			hdr.Flip();

		if (hdr.Signature != NIT_MAKE_CC('!', 'Z', 'C', 'C'))
		{
			errStr = "invalid ccz signature";
			goto error;
		}

		if (hdr.Version > 2)
		{
			errStr = "unsupported ccz version";
			goto error;
		}

		if (hdr.CompressionType != 0) // CCZ_COMPRESSION_ZLIB
		{
			errStr = "unsupported ccz compression";
			goto error;
		}

		// change to Zlib decompression stream
		reader = new ZStreamReader(reader);
	}

	PVRTexHeader hdr;

	if (reader->readRaw(&hdr, sizeof(hdr)) != sizeof(hdr))
	{
		errStr = "invalid pvr header";
		goto error;
	}

	if (hdr.Signature == NIT_MAKE_CC('!', 'R', 'V', 'P'))
		hdr.Flip();
	
	if (hdr.Signature != NIT_MAKE_CC('P', 'V', 'R', '!'))
	{
		errStr = "invalid pvr signature";
		goto error;
	}

	switch (hdr.GetFormat())
	{
	case PVRTexHeader::RGBA_4444:		fmt = PixelFormat::RGBA_4444; break;
	case PVRTexHeader::RGBA_5551:		fmt = PixelFormat::RGBA_5551; break;
	case PVRTexHeader::RGBA_8888:		fmt = PixelFormat::RGBA_8888; break;
	case PVRTexHeader::RGB_565:			fmt = PixelFormat::RGB_565; break;
	case PVRTexHeader::RGB_888:			fmt = PixelFormat::RGB_888; break;
	case PVRTexHeader::I_8:				fmt = PixelFormat::L8; break;
	case PVRTexHeader::AI_88:			fmt = PixelFormat::AL_88; break;
	case PVRTexHeader::PVRTC_2:			fmt = PixelFormat::PVR2A; break;	// no meaning on pvr header's alpha. Assume that alpha exists.
	case PVRTexHeader::PVRTC_4:			fmt = PixelFormat::PVR4A; break;	// no meaning on pvr header's alpha. Assume that alpha exists.
	case PVRTexHeader::BGRA_8888:		fmt = PixelFormat::BGRA_8888; break;
	case PVRTexHeader::A_8:				fmt = PixelFormat::A8; break;
	default:
		errStr = "not supported format";
		goto error;
	}

	_header.pixelFormat		= fmt;
	_header.width				= hdr.Width;
	_header.height				= hdr.Height;
	_header.mipmapCount		= hdr.NumMipmaps + 1;
	_header.memorySize			= hdr.DataLength;

	_header.sourceWidth		= hdr.Width;
	_header.sourceHeight		= hdr.Height;

	_header.contentLeft		= 0;
	_header.contentTop			= 0;
	_header.contentRight		= hdr.Width;
	_header.contentBottom		= hdr.Height;

	assert(hdr.Bpp == PixelFormat::calcBitsPerPixel(fmt));
	_bitsPerPixel				= hdr.Bpp;
	_pitch						= PixelFormat::calcPitch(fmt, hdr.Width);

	if (!onlyHeader)
	{
		if (reader->isSized() && reader->isSeekable())
		{
			reader->skip(hdr.HeaderLength - sizeof(hdr));
		}
		else
		{
			for (int i = hdr.HeaderLength - sizeof(hdr); i > 0; --i)
			{
				char ch;
				reader->readRaw(&ch, 1);
			}
		}

		size = hdr.DataLength;
		data = Allocate(size);
		if (reader->readRaw(data, size) != size)
		{
			errStr = "truncated pvr file";
			goto error;
		}

		_pixelBuffer = data;
	}

	return;

error:
	if (errStr.empty())
		errStr = "can't read pvr";

	if (data)
		Deallocate(data, size);

	_error = true;
	NIT_THROW_FMT(EX_READ, "'%s': %s\n", reader->getUrl().c_str(), errStr.c_str());
}

void Image::replaceHeader(const Header& header)
{
	_header = header;
}

template <typename TPixel>
static void PremultiplyAlpha(uint8* buf, size_t size)
{
	TPixel* end = (TPixel*)(buf + size);
	for (TPixel* pix = (TPixel*)buf; pix < end; ++pix)
	{
		int a = pix->a + 1;
		pix->r = (pix->r * a) >> TPixel::A_BITS;
		pix->g = (pix->g * a) >> TPixel::A_BITS;
		pix->b = (pix->b * a) >> TPixel::A_BITS;
	}
}

void Image::makeAlphaPremultiplied()
{
	PixelFormat fmt = PixelFormat(_header.pixelFormat);
	if (fmt.isAlphaPremultiplied()) return;
	ASSERT_THROW(_pixelBuffer, EX_INVALID_STATE);
	ASSERT_THROW(fmt.hasAlpha(), EX_INVALID_STATE);
	ASSERT_THROW(!fmt.isCompressed(), EX_INVALID_STATE);

	// Assume that there's no gap among mipmaps

	// Assume that byte order of alpha components are consistant RGB*A*  BGR*A*
	if (fmt == PixelFormat::RGBA_8888 || fmt == PixelFormat::BGRA_8888)
	{
		PremultiplyAlpha<PixelRGBA_8888>(_pixelBuffer, _header.memorySize);
	}
	else if (fmt == PixelFormat::RGBA_4444 || fmt == PixelFormat::ARGB_4444)
	{
		PremultiplyAlpha<PixelRGBA_4444>(_pixelBuffer, _header.memorySize);
	}
	else
	{
		NIT_THROW(EX_NOT_SUPPORTED);
	}

	_header.pixelFormat |= PixelFormat::FLAG_PMUL_ALPHA;
}

void Image::discardMipmaps()
{
	if (_header.mipmapCount == 1) return;

	ASSERT_THROW(_pixelBuffer, EX_INVALID_STATE);

	size_t newSize = _pitch * _header.height;
	uint8* newBuffer = Allocate(newSize);

	if (newBuffer == NULL)
		NIT_THROW(EX_MEMORY);

	memcpy(newBuffer, getMipData(0), newSize);

	Deallocate(_pixelBuffer, _header.memorySize);
	_pixelBuffer = newBuffer;
	_header.memorySize = newSize;
	_header.mipmapCount = 1;
}

bool Image::makePot(bool square, uint16 min)
{
	uint potWidth	= PixelFormat::calcNextPot(std::max(min, _header.width));
	uint potHeight	= PixelFormat::calcNextPot(std::max(min, _header.height));

	if (potWidth == _header.width && potHeight == _header.height)
		return true;

	PixelFormat fmt = PixelFormat(_header.pixelFormat);
	ASSERT_THROW(!fmt.isCompressed(), EX_NOT_SUPPORTED);
	ASSERT_THROW(!fmt.isTiled(), EX_NOT_SUPPORTED);
	ASSERT_THROW(_header.mipmapCount == 1, EX_NOT_SUPPORTED);

	if (square && potWidth != potHeight)
	{
		int larger = std::max(potWidth, potHeight);
		potWidth = larger;
		potHeight = larger;
	}

	if (potWidth == _header.width && potHeight == _header.height)
		return false;

	uint newPitch = potWidth * (_bitsPerPixel >> 3);
	size_t newSize = potHeight * newPitch;

	if (_pixelBuffer)
	{
		uint8* newBuffer = Allocate(newSize);

		if (newBuffer == NULL)
			NIT_THROW(EX_MEMORY);

		// fill 0x00000000 within extended canvas
		memset(newBuffer, 0, newSize);

		// copy to buffer by scanline
		for (uint y=0; y<_header.height; ++y)
		{
			memcpy(newBuffer + y * newPitch, _pixelBuffer + y * _pitch, _pitch);
		}

		Deallocate(_pixelBuffer, _header.memorySize);
		_pixelBuffer = newBuffer;
	}

	_pitch = newPitch;
	_header.memorySize = newSize;
	_header.mipmapCount = 1;
	_header.width = potWidth;
	_header.height = potHeight;

	return true;
}

template <typename TPixel>
static void _ReorderAsRGBA(uint8* buf, size_t size)
{
	TPixel* end = (TPixel*)(buf + size);
	for (TPixel* pix = (TPixel*)buf; pix < end; ++pix)
	{
		int t = pix->r;
		pix->r = pix->b;
		pix->b = t;
	}
}

void Image::reorderAsRgba()
{
	PixelFormat fmt = PixelFormat(_header.pixelFormat);
	ASSERT_THROW(_pixelBuffer, EX_INVALID_STATE);
	ASSERT_THROW(!fmt.isCompressed(), EX_NOT_SUPPORTED);

	if (fmt.isOrderedAsRgba()) return;

	if (fmt == PixelFormat::ARGB_8888 || fmt == PixelFormat::ARGB_8888_PA)
	{
		_ReorderAsRGBA<PixelRGBA_8888>(_pixelBuffer, _header.memorySize);
	}
	else if (fmt == PixelFormat::ARGB_4444 || fmt == PixelFormat::ARGB_4444_PA)
	{
		_ReorderAsRGBA<PixelRGBA_4444>(_pixelBuffer, _header.memorySize);
	}
	else
	{
		NIT_THROW(EX_NOT_SUPPORTED);
	}

	_header.pixelFormat = (PixelFormat::RGBA_8888 & PixelFormat::FLAG_ID_MASK) | (fmt & !PixelFormat::FLAG_ID_MASK);
}

void Image::makeRgba_4444()
{
	PixelFormat fmt = PixelFormat(_header.pixelFormat);
	ASSERT_THROW(_pixelBuffer, EX_INVALID_STATE);
	ASSERT_THROW(!fmt.isCompressed(), EX_NOT_SUPPORTED);
	ASSERT_THROW(!fmt.isTiled(), EX_NOT_SUPPORTED);

	PixelFormat newFmt;

	switch (fmt)
	{
	case PixelFormat::RGBA_4444:
	case PixelFormat::RGBA_4444_PA:		return;

	case PixelFormat::RGBA_8888:		newFmt = PixelFormat::RGBA_4444; break;
	case PixelFormat::RGBA_8888_PA:		newFmt = PixelFormat::RGBA_4444_PA; break;

	default:							NIT_THROW(EX_NOT_SUPPORTED);
	}

	uint newPitch = _header.width * 2;
	size_t newSize = _header.height * newPitch;
	uint8* newBuffer = Allocate(newSize);

	if (newBuffer == NULL) 
		NIT_THROW(EX_MEMORY);

	PixelRGBA_8888* end = (PixelRGBA_8888*)(_pixelBuffer + _header.memorySize);

	PixelRGBA_4444* dst = (PixelRGBA_4444*)newBuffer;
	for (PixelRGBA_8888* src = (PixelRGBA_8888*)_pixelBuffer; src < end; ++src, ++dst)
	{
		PixelRGBA_8888 s = *src;
		PixelRGBA_4444 d;

		d.a = s.a >> 4;
		d.r = s.r >> 4;
		d.g = s.g >> 4;
		d.b = s.b >> 4;

		*dst = d;
	}

	Deallocate(_pixelBuffer, _header.memorySize);
	_pixelBuffer = newBuffer;
	_header.memorySize = newSize;
	_header.pixelFormat = newFmt;
	_pitch = newPitch;
	_bitsPerPixel = 16;
}

void Image::SaveNtex(StreamWriter* writer, bool flipEndian)
{
	ASSERT_THROW(_pixelBuffer, EX_INVALID_STATE);

	// TODO: write fail possible (out of memory?)

	Ref<StreamWriter> safe = writer;

	SigHeader sig = { 0 };
	sig.signature	= NIT_NTEX_SIGNATURE;
	sig.version		= NIT_NTEX_VERSION;

	if (flipEndian) sig.flipEndian();

	writer->writeRaw(&sig, sizeof(sig));

	Header hdr = _header;

	if (flipEndian) sig.flipEndian();

	writer->writeRaw(&hdr, sizeof(hdr));

	// TODO: flipping on pixel buffer needed when flipEndian set
	writer->writeRaw(_pixelBuffer, _header.memorySize);
}

static void _PNG_WriterCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	StreamWriter* writer = (StreamWriter*)png_get_io_ptr(png_ptr);

	size_t bytesWritten = writer->writeRaw(data, length);

	if (bytesWritten != length)
		png_error(png_ptr, "can't write file");
}

static void _PNG_FlushCallback(png_structp png_ptr)
{
	StreamWriter* writer = (StreamWriter*)png_get_io_ptr(png_ptr);

	if (!writer->flush())
		png_error(png_ptr, "can't flush file");
}

void Image::SavePng(StreamWriter* writer)
{
	Ref<StreamWriter> safe = writer;

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_color dummyPalette = { 0 };
	png_bytep* row_pointers = NULL;

	uint32 width, height;
	bool hasAlpha;
	int colorType;
	int pitch = _pitch;

	String errStr;

	if (_pixelBuffer == NULL)
	{
		errStr = "no pixel buffer";
		goto error;
	}

	// For now, only 32 bpp, 24 bpp supported (RGBA, ARGB, RGB etc)
	if (_bitsPerPixel != 32 && _bitsPerPixel != 24)
	{
		errStr = "not supported bpp";
		goto error;
	}

	// TODO: BGRA format
	// TODO: 16bit format
	// TODO: content rect
	// TODO: pre-multiplied alpha

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, &errStr, _png_OnError, _png_OnWarning);
	if (png_ptr == NULL)
		goto error;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		goto error;

	if (setjmp(png_jmpbuf(png_ptr)))
		goto error;

	// link a callback function
	png_set_write_fn(png_ptr, writer, _PNG_WriterCallback, _PNG_FlushCallback);

	width = _header.width;
	height = _header.height;
	hasAlpha = getPixelFormat().hasAlpha();
	colorType = hasAlpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB;

	png_set_IHDR(
		png_ptr,
		info_ptr,
		width,
		height,
		8,			// bit_depth
		colorType,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE
		);

	// we will not use palette, so make a dummy
	png_set_PLTE(png_ptr, info_ptr, &dummyPalette, 1);
	
	png_write_info(png_ptr, info_ptr);

	png_set_packing(png_ptr);

	// TODO: need this?
// 	if (!GetPixelFormat().IsOrderedAsRGBA())
// 		png_set_bgr(png_ptr);

	row_pointers = (png_bytep*) NIT_ALLOC(height * sizeof(png_bytep));
	if (row_pointers == NULL)
		goto error;

	for (int i = 0; i < (int)height; ++i)
	{
		row_pointers[i] = (png_bytep)&_pixelBuffer[i * pitch];
	}

	png_write_image(png_ptr, row_pointers);

	NIT_DEALLOC(row_pointers, 0);
	row_pointers = NULL;

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	return;

error:
	if (row_pointers)
		NIT_DEALLOC(row_pointers, 0);

	if (png_ptr)
		png_destroy_write_struct(&png_ptr, &info_ptr);

	if (errStr.empty())
		errStr = "can't write png";

	NIT_THROW_FMT(EX_WRITE, "'%s': %s", writer->getUrl().c_str(), errStr.c_str());
}

void Image::SaveJpeg(StreamWriter* writer)
{
	Ref<StreamWriter> safe = writer;

	uint8* buffer = NULL;
	ulong size = 0;

	jpeg_compress_struct cinfo = { 0 };
	jpeg_error_mgr jerr = { 0 };
	JSAMPROW row_pointer[1];
	String errStr;
	int width, height, pitch;

	if (_pixelBuffer == NULL)
	{
		errStr = "no pixel buffer";
		goto error;
	}

	// For now, only 32 bpp, 24 bpp supported (RGBA, ARGB, RGB etc)
	if (_bitsPerPixel != 32 && _bitsPerPixel != 24)
	{
		errStr = "not supported bpp";
		goto error;
	}

	// TODO: BGRA format
	// TODO: 16bit format
	// TODO: content rect
	// TODO: pre-multiplied alpha

	width = _header.width;
	height = _header.height;
	pitch = _pitch;

	jpeg_std_error(&jerr);
	jerr.error_exit = _jpeg_OnErrorExit;
	cinfo.err = &jerr;
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &buffer, &size);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);

	jpeg_start_compress(&cinfo, TRUE);

	if (getPixelFormat().hasAlpha())
	{
		int row_stride = width * 3;
		uint8* rgbRow = (uint8*)NIT_ALLOC(row_stride);
		for (int i=0; i < height; ++i)
		{
			uint8* srcRow = &_pixelBuffer[i * pitch];
			for (int j=0; j < width; ++j)
			{
				rgbRow[j * 3 + 0] = srcRow[j * 4 + 0];
				rgbRow[j * 3 + 1] = srcRow[j * 4 + 1];
				rgbRow[j * 3 + 2] = srcRow[j * 4 + 2];
			}
			row_pointer[0] = &rgbRow[0];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}
		NIT_DEALLOC(rgbRow, row_stride);
	}
	else
	{
		for (int i=0; i < height; ++i)
		{
			row_pointer[0] = &_pixelBuffer[i * pitch];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	writer->writeRaw(buffer, size);

	free(buffer);
	
	return;

error:
	jpeg_destroy_compress(&cinfo);

	if (errStr.empty())
		errStr = "can't write jpeg";

	NIT_THROW_FMT(EX_WRITE, "'%s': %s\n", writer->getUrl().c_str(), errStr.c_str());
}

////////////////////////////////////////////////////////////////////////////////

void Image::SigHeader::flipEndian()
{
	StreamUtil::flipEndian(signature);
	StreamUtil::flipEndian(version);
}

void Image::Header::flipEndian()
{
	StreamUtil::flipEndian(memorySize);
	StreamUtil::flipEndian(width);
	StreamUtil::flipEndian(height);
	StreamUtil::flipEndian(flags);
	StreamUtil::flipEndian(sourceWidth);
	StreamUtil::flipEndian(sourceHeight);
	StreamUtil::flipEndian(contentLeft);
	StreamUtil::flipEndian(contentTop);
	StreamUtil::flipEndian(contentRight);
	StreamUtil::flipEndian(contentBottom);
	StreamUtil::flipEndian(mipmapCount);
	StreamUtil::flipEndian(pixelFormat);
	StreamUtil::flipEndian(_reserved0);
	StreamUtil::flipEndian(_reserved1);
}

////////////////////////////////////////////////////////////////////////////////

ImageManager::ImageManager()
: ContentManager("ImageManager", NULL)
{

}

ImageManager::ImageManager(const String& name, Package* package)
: ContentManager(name, package)
{

}

void ImageManager::onRegister()
{

}

void ImageManager::onUnregister()
{

}

void ImageManager::onInit()
{

}

void ImageManager::onFinish()
{

}

void ImageManager::allContentTypes(vector<ContentType>::type& outResults)
{

}

bool ImageManager::canLink(StreamSource* source)
{
	// TODO: implement this
	return true;
}

void ImageManager::onDispose(Content* content)
{

}

Ref<Content> ImageManager::createFrom(StreamSource* source)
{
	return new Image(source);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
