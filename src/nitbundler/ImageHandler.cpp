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

#include "nitbundler_pch.h"

#include "nitbundler/ImageHandler.h"

#include "PVRTexLib.h"

NS_BUNDLER_BEGIN;

////////////////////////////////////////////////////////////////////////////////

struct CCZHeader {
	unsigned char			sig[4];				// signature. Should be 'CCZ!' 4 bytes
	unsigned short		    compression_type;	// should 0
	unsigned short		    version;			// should be 2 (although version type==1 is also supported)
	unsigned int 		    reserved;			// Reserverd for users.
	unsigned int		    len;				// size of the uncompressed file
};

enum {
	CCZ_COMPRESSION_ZLIB,				// zlib format.
	CCZ_COMPRESSION_BZIP2,				// bzip2 format (not supported yet)
	CCZ_COMPRESSION_GZIP,				// gzip format (not supported yet)
	CCZ_COMPRESSION_NONE,				// plain (not supported yet)
};

static void SwapRGBA(uint8* buf, int width, int height, int pitch)
{
	for (int y=0; y<height; ++y)
	{
		uint32* pixel = (uint32*)(buf + y * pitch);
		for (int x=0; x<width; ++x)
		{
			uint32 p = *pixel;
			uint32 a = p & 0xFF000000;
			uint32 r = p & 0x00FF0000;
			uint32 g = p & 0x0000FF00;
			uint32 b = p & 0x000000FF;

			*pixel++ = a | (r >> 16) | (g) | (b << 16);
		}
	}
}

pvrtexlib::PixelType ToPvrPixelType(PixelFormat fmt)
{
	pvrtexlib::PixelType pt = pvrtexlib::MGLPT_NOTYPE;

	switch (fmt)
	{
	case PixelFormat::RGB_565:			pt = pvrtexlib::OGL_RGB_565; break;
	case PixelFormat::RGB_888:			pt = pvrtexlib::OGL_RGB_888; break;

	case PixelFormat::A8:				pt = pvrtexlib::OGL_A_8; break;

	case PixelFormat::L8:				pt = pvrtexlib::OGL_I_8; break;

	case PixelFormat::AL_88:
	case PixelFormat::AL_88_PA:			pt = pvrtexlib::OGL_AI_88; break;

	// DX
	case PixelFormat::ARGB_8888:
	case PixelFormat::ARGB_8888_PA:		pt = pvrtexlib::MGLPT_ARGB_8888; break; // TODO: is this correct?

	case PixelFormat::DXT1:				pt = pvrtexlib::D3D_DXT1; break;

	case PixelFormat::DXT3:	
	case PixelFormat::DXT3_PA:			pt = pvrtexlib::D3D_DXT3; break;

	case PixelFormat::DXT5:
	case PixelFormat::DXT5_PA:			pt = pvrtexlib::D3D_DXT5; break;

	// GL
	case PixelFormat::RGBA_8888:
	case PixelFormat::RGBA_8888_PA:		pt = pvrtexlib::OGL_RGBA_8888; break;

	case PixelFormat::BGRA_8888:
	case PixelFormat::BGRA_8888_PA:		pt = pvrtexlib::OGL_BGRA_8888; break;

	case PixelFormat::RGBA_4444:
	case PixelFormat::RGBA_4444_PA:		pt = pvrtexlib::OGL_RGBA_4444; break;

	case PixelFormat::RGBA_5551:
	case PixelFormat::RGBA_5551_PA:		pt = pvrtexlib::OGL_RGBA_5551; break;

	case PixelFormat::PVR2:
	case PixelFormat::PVR2A:
	case PixelFormat::PVR2A_PA:			pt = pvrtexlib::OGL_PVRTC2; break;

	case PixelFormat::PVR4:
	case PixelFormat::PVR4A:
	case PixelFormat::PVR4A_PA:			pt = pvrtexlib::OGL_PVRTC4; break;

	case PixelFormat::ETC:				pt = pvrtexlib::ETC_RGB_4BPP; break;
	}

	return pt;
}

static void SharpenImageRGBA(PixelRGBA_8888* image, uint width, uint height, uint pitch)
{
	int fw, fh;
	float factor, bias;
	float* filter;

	bool strongFilter = false;

	if (strongFilter)
	{	
		// strong sharpen filter
		fw = 3;
		fh = 3;
		float filterConsts[] =
		{
 			-1, -1, -1,
 			-1,  9, -1,
 			-1, -1, -1
		};
		filter = filterConsts;
		factor = 1.0f;
		bias = 0.0f;
	}
	else
	{
		// soft sharpen filter
		fw = 5;
		fh = 5;
		float filterConsts[] =
		{
			-1, -1, -1, -1, -1,
			-1,  2,  2,  2, -1,
			-1,  2, 16,  2, -1,
			-1,  2,  2,  2, -1,
			-1, -1, -1, -1, -1
		};
		filter = filterConsts;
		factor = 1.0f / 16.0f;
		bias = 0.0f;
	}


	size_t tempSize = sizeof(PixelRGBA_8888) * width * height;
	PixelRGBA_8888* tempImage = (PixelRGBA_8888*)NIT_ALLOC(tempSize);

	byte* bytes = (byte*)image;

	for (uint x = 0; x < width; ++x)
	{
		for (uint y = 0; y < height; ++y)
		{
			float a = 0, r = 0, g = 0, b = 0;

			for (int fx = 0; fx < fw; ++fx)
			{
				for (int fy = 0; fy < fh; ++fy)
				{
					int ix = (x - fw / 2 + fx + width) % width;
					int iy = (y - fh / 2 + fy + height) % height;

					int off = iy * pitch + 4 * ix;
					r += bytes[off + 0] * filter[fx + fy * fw];
					g += bytes[off + 1] * filter[fx + fy * fw];
					b += bytes[off + 2] * filter[fx + fy * fw];
					a += bytes[off + 3] * filter[fx + fy * fw];
				}
			}

			PixelRGBA_8888* temp = &tempImage[x + y * width];

			temp->r = (byte)Math::clamp(int(factor * r + bias), 0, 255);
			temp->g = (byte)Math::clamp(int(factor * g + bias), 0, 255);
			temp->b = (byte)Math::clamp(int(factor * b + bias), 0, 255);
			temp->a = (byte)Math::clamp(int(factor * a + bias), 0, 255);
		}
	}

	for (uint x = 0; x < width; ++x)
	{
		for (uint y = 0; y < height; ++y)
		{
			PixelRGBA_8888* tgt = (PixelRGBA_8888*)((char*)image + y * pitch + x * 4);
			*tgt = tempImage[x + y * width];
		}
	}

	NIT_DEALLOC(tempImage, tempSize);
}

////////////////////////////////////////////////////////////////////////////////

class FI_IO
{
public:
	static FIBITMAP* Load(StreamSource* source, FREE_IMAGE_FORMAT* outFormat = NULL)
	{
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

		FreeImageIO io;
		io.read_proc = read;
		io.write_proc = NULL;
		io.seek_proc = seek<StreamReader>;
		io.tell_proc = tell<StreamReader>;

		Ref<StreamReader> r = source->open();

		// Determine the file's format by its signature.
		fif = FreeImage_GetFileTypeFromHandle(&io, (fi_handle)r);
		if (fif == FIF_UNKNOWN)
		{
			// Can't check signature.
			// Determine the extension of the StreamSource.
			fif = FreeImage_GetFIFFromFilename(source->getName().c_str());
		}

		if (outFormat)
			*outFormat = fif;

		if (fif == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif))
		{
			LOG(0, "not supported image format: '%s'\n", source->getUrl().c_str());
			return NULL;
		}

		FIBITMAP* bitmap = FreeImage_LoadFromHandle(fif, &io, (fi_handle)r);
		return bitmap;
	}

	static BOOL Save(FREE_IMAGE_FORMAT fif, FIBITMAP* bitmap, StreamWriter* w)
	{
		Ref<StreamWriter> safe = w;

		FreeImageIO io;
		io.read_proc = NULL;
		io.write_proc = write;
		io.seek_proc = seek<StreamReader>;
		io.tell_proc = tell<StreamReader>;

		return FreeImage_SaveToHandle(fif, bitmap, &io, (fi_handle)w);
	}

	static unsigned DLL_CALLCONV read(void* buffer, unsigned size, unsigned count, fi_handle handle)
	{
		StreamReader* r = (StreamReader*)handle;
		return r->readRaw(buffer, size * count);
	}

	static unsigned DLL_CALLCONV write(void* buffer, unsigned size, unsigned count, fi_handle handle)
	{
		StreamWriter* w = (StreamWriter*)handle;
		return w->writeRaw(buffer, size * count);
	}

	template <typename TStream>
	static int DLL_CALLCONV seek(fi_handle handle, long offset, int origin)
	{
		TStream* s = (TStream*)handle;

		switch (origin)
		{
		case SEEK_SET:
			s->seek(offset);
			return 0;

		case SEEK_CUR:
			s->skip(offset);
			return 0;

		case SEEK_END:
			s->seek(s->getSize() + offset);
			return 0;

		default:
			NIT_THROW(EX_NOT_SUPPORTED);
		}
	}

	template <typename TStream>
	static long DLL_CALLCONV tell(fi_handle handle)
	{
		TStream* s = (TStream*)handle;

		return s->tell();
	}
};

////////////////////////////////////////////////////////////////////////////////

StreamSource* OpenCCZ(Ref<StreamSource> source)
{
//	LOG_TIMESCOPE(0, ".. uncompressing '%s'", source->GetURL().c_str());

	Ref<MemoryBuffer> buf = new MemoryBuffer(source->open(), 4096);

	if (buf->getSize() < sizeof(CCZHeader))
	{
		LOG(0, "++ invalid ccz file: '%s'\n", source->getUrl().c_str());
		return NULL;
	}

	CCZHeader header;
	buf->copyTo(&header, 0, sizeof(header));
	buf->popFront(sizeof(header));

	uint32 sig = *(uint32*)header.sig;

	// Detect endianness
	bool flip = true;
	if (sig == NIT_MAKE_CC('C', 'C', 'Z', '!'))
		flip = true;
	else if (sig != NIT_MAKE_CC('!', 'Z', 'C', 'C'))
	{
		LOG(0, "*** invalid ccz signature: '%s'\n", source->getUrl().c_str());
		return NULL;
	}

	if (flip)
	{
		StreamUtil::flipEndian(header.compression_type);
		StreamUtil::flipEndian(header.compression_type);
		StreamUtil::flipEndian(header.version);
		StreamUtil::flipEndian(header.reserved);
		StreamUtil::flipEndian(header.len);
	}

	if (header.version > 2)
	{
		LOG(0, "*** unsupported ccz version: '%s'\n", source->getUrl().c_str());
		return NULL;
	}

	if (header.compression_type != CCZ_COMPRESSION_ZLIB)
	{
		LOG(0, "*** unsupported ccz compression: '%s'\n", source->getUrl().c_str());
		return NULL;
	}

	try { buf->uncompress(); }
	catch (...)
	{
		LOG(0, "*** ccz uncompress fail: '%s", source->getUrl().c_str());
		return NULL;
	}

	if (header.len != buf->getSize())
	{
		LOG(0, "*** ccz size mismatch: '%s'\n", source->getUrl().c_str());
		return NULL;
	}

	MemorySource* uncompressed = new MemorySource(source, buf);
	if (StreamSource::getExtension(uncompressed->getName()) == "ccz")
	{
		String name = uncompressed->getName();
		if (name.length() > 4)
			name = name.substr(0, name.length() - 4);
		uncompressed->setName(name);
		uncompressed->setContentType(ContentType::fromStreamName(name));
	}

	return uncompressed;
}

////////////////////////////////////////////////////////////////////////////////

pvrtexlib::PVRTextureUtilities& getPvrTexUtil()
{
	static ThreadLocal<pvrtexlib::PVRTextureUtilities> th_Local;
	
	pvrtexlib::PVRTextureUtilities& s_Util = *th_Local;

	return s_Util;
}

static FIBITMAP* LoadPVR(Ref<StreamSource> source)
{
	//	LOG_TIMESCOPE(0, ".. loading '%s'", source->GetURL().c_str());

	// It may seems to be inefficient to convert pvr -> freeimage -> rescale -> pvr again instead of using pvr directly,
	// In production we use mostly use png as source not pvr directly, so unify the process.

	if (source->getContentType() == ContentType::IMAGE_CCZ)
	{
		source = OpenCCZ(source);
	}

	try
	{
		Ref<MemoryBuffer> buf = source->open()->buffer();
		MemoryBuffer::Access mem(buf);

		pvrtexlib::CPVRTexture orig((const uint8*)mem.getMemory());
		pvrtexlib::CPVRTexture decomp;

		pvrtexlib::PVRTextureUtilities& pvrUtil = getPvrTexUtil();
		pvrUtil.DecompressPVR(orig, decomp);

		// decomp always becomes 32 bpp RGBA

		// convert BGRA -> RGBA
		SwapRGBA(decomp.getData().getData(), decomp.getWidth(), decomp.getHeight(), 4 * decomp.getWidth());

		FIBITMAP* bitmap = FreeImage_ConvertFromRawBits(
			decomp.getData().getData(),	// bits
			decomp.getWidth(),			// width
			decomp.getHeight(),			// height
			(32 * decomp.getWidth() + 31) / 32 * 4, // pitch
			32,							// bpp
			FI_RGBA_RED_MASK,			// red_mask
			FI_RGBA_GREEN_MASK,			// green_mask
			FI_RGBA_BLUE_MASK,			// blue_mask
			TRUE);						// topdown

		buf = NULL;
		return bitmap;
	}
	catch (PVRException& ex)
	{
		LOG(0, "*** pvrtexlib error : %s\n", ex.what());
		return NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

ImageZen::ImageZen(StreamSource* source)
{
	_bitmap = NULL;
	_source = source;
	_sourceWidth = 0;
	_sourceHeight = 0;
}

ImageZen* ImageZen::clone()
{
	ImageZen* cloned = new ImageZen(_source);

	cloned->_bitmap = FreeImage_Clone(_bitmap);
	cloned->_sourceFormat = _sourceFormat;

	return cloned;
}

void ImageZen::onDelete()
{
	if (_bitmap)
	{
		FreeImage_Unload(_bitmap);
		_bitmap = NULL;
	}

	RefCounted::onDelete();
}

bool ImageZen::load()
{
	if (_source == NULL)
		return false;

	switch (_source->getContentType())
	{
	case ContentType::IMAGE_CCZ:
	case ContentType::IMAGE_PVR:
		_bitmap = LoadPVR(_source);
		break;

	default:
		_bitmap = FI_IO::Load(_source, &_sourceFormat);
		break;
	}

	if (_bitmap)
	{
		_sourceWidth = getWidth();
		_sourceHeight = getHeight();
		return true;
	}

	return false;
}

void ImageZen::process(Options& opt)
{
	// NOTE: Order is important in this function

	int width = getWidth();
	int height = getHeight();

	// Apply resizing
	if (opt.width != width || opt.height != height)
	{
		LOG(0, ".. rescale (%d, %d) -> (%d, %d): '%s'\n", width, height, opt.width, opt.height, _source->getUrl().c_str());
		width = opt.width;
		height = opt.height;
		rescale(width, height);
	}

	// Determine texture size by specified option
	int nwidth = width;
	int nheight = height;

	if (nwidth < opt.minWidthHeight)
		nwidth = opt.minWidthHeight;

	if (nheight < opt.minWidthHeight)
		nheight = opt.minWidthHeight;

	if (opt.alignWidthHeight != 1)
	{
		// Align multiplication
		if (nwidth % opt.alignWidthHeight)
			nwidth = (nwidth / opt.alignWidthHeight + 1) * opt.alignWidthHeight;

		if (nheight % opt.alignWidthHeight)
			nheight = (nheight / opt.alignWidthHeight + 1) * opt.alignWidthHeight;
	}

	if (opt.makePOT)
	{
		nwidth = PixelFormat::calcNextPot(nwidth);
		nheight = PixelFormat::calcNextPot(nheight);
	}

	if (opt.makeSquare)
	{
		int larger = std::max(nwidth, nheight);
		nwidth = larger;
		nheight = larger;
	}

	// If we need to enlarge the canvas
	if (nwidth != width || nheight != height)
	{
		LOG(0, "?? enlarge (%d, %d) -> (%d, %d): '%s'\n", width, height, nwidth, nheight, _source->getUrl().c_str());
		enlargeCanvas(nwidth, nheight);
	}
}

uint32 ImageZen::write(StreamWriter* w, Options& opt, StreamWriter* dumpFile)
{
	int bpp = FreeImage_GetBPP(_bitmap);

	if (bpp != 32)
	{
		// We'll work in 32 bpp space - convert the bitmap to 32 bpp first!
		FIBITMAP* bpp32 = FreeImage_ConvertTo32Bits(_bitmap);
		FreeImage_Unload(_bitmap);
		_bitmap = bpp32;

		bpp = FreeImage_GetBPP(_bitmap);
	}

	process(opt);

	int width = getWidth();
	int height = getHeight();
	int pitch = FreeImage_GetPitch(_bitmap);

	uint8 mipmapCount = 1;
	uint32 dataSize = 0;
	uint8* data = NULL;

	// Perform conversion according to opt.Format by pvrtexlib

	pvrtexlib::PixelType pt = ToPvrPixelType(opt.format);

	if (pt == pvrtexlib::MGLPT_NOTYPE)
		NIT_THROW(EX_NOT_SUPPORTED);

	pvrtexlib::PVRTextureUtilities& util = getPvrTexUtil();
	pvrtexlib::CPVRTexture orig(width, height);

	// Convert FIBITMAP to CPVRTexture
	uint8* origData = orig.getData().getData();
	memset(origData, 0, orig.getData().getDataSize());
	FreeImage_ConvertToRawBits(
		origData, _bitmap, pitch, bpp, 
		FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);

	// Do sharpen if specified
	if (opt.sharpen)
	{
		LOG(0, ".. sharpen: '%s'\n", _source->getUrl().c_str());
		SharpenImageRGBA((PixelRGBA_8888*)origData, width, height, pitch);
	}

	SwapRGBA(orig.getData().getData(), width, height, pitch);

	pvrtexlib::CPVRTexture comp(orig.getHeader());
	comp.setPixelType(pt);
	comp.setAlpha(opt.format.hasAlpha());

//	comp.setBorder(true); // TODO: We need pixelwise adjustment to Content-Left-Top-Right-Bottom

	if (opt.makeMipmaps)
	{
		// TODO: mipmap Count
		mipmapCount = 1;
		comp.setMipMapCount(mipmapCount - 1); // TODO: Is this sufficient?
	}

	try
	{
		if (opt.format.isAlphaPremultiplied())
		{
			// Apply alpha pre-multiplication
			bool ok = util.ProcessRawPVR(orig, orig.getHeader(), false, 0, 0, 0, true);
			if (!ok)
			{
				LOG(0, "*** '%s': pvrtexlib can't perform premultiply alpha\n", _source->getUrl().c_str());
			}
		}

		util.CompressPVR(orig, comp);
		data = comp.getData().getData();
		dataSize = comp.getData().getDataSize();

		bool saveComp = true;

		if (dumpFile)
		{
			if (saveComp)
			{
				Ref<MemorySource> dumpSrc = new MemorySource("dump.pvr");
				Ref<StreamWriter> dsw = dumpSrc->newWriter();

				pvrtexlib::CPVRTexture& tex = comp;

				// Save to pvr first
				uint8* hdr = (uint8*)NIT_ALLOC(tex.getHeader().getFileHeaderSize());
				tex.getHeader().writeToPointer(hdr);
				dsw->writeRaw(hdr, tex.getHeader().getFileHeaderSize());
				void* texData = tex.getData().getData();
				size_t texSize = tex.getData().getDataSize();
				dsw->writeRaw(texData, texSize);
				NIT_DEALLOC(hdr, 0);

				// Convert to png using temporary free-image bitmap loaded by LoadPVR()
				FIBITMAP* dumpBmp = LoadPVR(dumpSrc.get());
				FI_IO::Save(FIF_PNG, dumpBmp, dumpFile);
				FreeImage_Unload(dumpBmp);
			}
			else
			{
				FI_IO::Save(FIF_PNG, _bitmap, dumpFile);
			}
		}
	}
	catch (PVRException& ex)
	{
		LOG(0, "*** '%s': pvrtexlib error: %s\n", _source->getUrl().c_str(), ex.what());
		return 0;
	}

	if (data == NULL)
		return 0;

	if (opt.contentType == ContentType::IMAGE_NTEX)
	{
		Image::SigHeader sig = { 0 };
		sig.signature		= NIT_NTEX_SIGNATURE;
		sig.version			= NIT_NTEX_VERSION;

		if (opt.flipEndian) sig.flipEndian();

		Image::Header hdr = { 0 };
		hdr.pixelFormat		= opt.format;
		hdr.memorySize		= dataSize;
		hdr.width			= width;
		hdr.height			= height;
		hdr.sourceWidth		= _sourceWidth;
		hdr.sourceHeight	= _sourceHeight;
		hdr.contentLeft		= 0;			// TODO: support later
		hdr.contentTop		= 0;
		hdr.contentRight	= opt.width;
		hdr.contentBottom	= opt.height;
		hdr.mipmapCount		= mipmapCount;

		if (opt.flipEndian) hdr.flipEndian();

		w->writeRaw(&sig, sizeof(sig));
		w->writeRaw(&hdr, sizeof(hdr));

		// TODO: Need a flip to pixel buffer when it's flipendian
		w->writeRaw(data, dataSize);

		return sizeof(sig) + sizeof(hdr) + dataSize;
	}

	NIT_THROW(EX_NOT_SUPPORTED);
}

void ImageZen::rescale(int dstWidth, int dstHeight, FREE_IMAGE_FILTER filter)
{
	FIBITMAP* rescaled = FreeImage_Rescale(_bitmap, dstWidth, dstHeight, filter);

	if (rescaled == NULL)
		NIT_THROW(EX_SYSTEM);

	FreeImage_Unload(_bitmap);
	_bitmap = rescaled;
}

void ImageZen::enlargeCanvas(int left, int top, int right, int bottom, uint32 color)
{
	FIBITMAP* enlarged = FreeImage_EnlargeCanvas(_bitmap, left, top, right, bottom, &color);
	
	if (enlarged == NULL)
		NIT_THROW(EX_SYSTEM);

	FreeImage_Unload(_bitmap);
	_bitmap = enlarged;
}

void ImageZen::enlargeCanvas(int width, int height, uint32 color)
{
	int cw = getWidth();
	int ch = getHeight();

	enlargeCanvas(0, 0, width - cw, height - ch, color);
}

////////////////////////////////////////////////////////////////////////////////

ImageHandler::ImageHandler()
{
	memset(&_options, 0, sizeof(_options));
	_makeDump = false;
}

void ImageHandler::setCodec(const String& codec)
{
//	Handler::SetCodec(codec);
	StringVector tokens = StringUtil::tokenise(codec);
	if (tokens.empty())
		Handler::setCodec(codec);

	_codec = tokens[0];

	memset(&_options, 0, sizeof(_options));
	_makeDump = false;

	_options.minWidthHeight = 1;
	_options.alignWidthHeight = 1;
	_options.flipEndian = _entry->getPacker()->isBigEndian();

	if (_codec == "png" || _codec == "jpeg" || _codec == "gif" || _codec == "pvr")
	{
		_options.contentType = ContentType::fromExtension(_codec);
	}
	else
	{
		_options.contentType = ContentType::IMAGE_NTEX;
		_options.format = PixelFormat(_codec);

		if (_options.format == PixelFormat::UNKNOWN)
			Handler::setCodec(codec);
	}

	for (uint i=0; i<tokens.size(); ++i)
	{
		if (tokens[i] == "square")
			_options.makeSquare = true;
		else if (tokens[i] == "pot")
			_options.makePOT = true;
		else if (tokens[i] == "mipmaps")
			_options.makeMipmaps = true;
		else if (tokens[i] == "min")
		{
			++i;
			if (i == tokens.size())
				NIT_THROW_FMT(EX_INVALID_PARAMS, "min value expected");
			int m; sscanf(tokens[i].c_str(), "%d", &m);
			_options.minWidthHeight = m;
		}
		else if (tokens[i] == "align")
		{
			++i;
			if (i == tokens.size())
				NIT_THROW_FMT(EX_INVALID_PARAMS, "align value expected");
			int a; sscanf(tokens[i].c_str(), "%d", &a);
			_options.alignWidthHeight = a;
		}
		else if (tokens[i] == "pivot")
		{
			++i;
			if (i == tokens.size())
				NIT_THROW_FMT(EX_INVALID_PARAMS, "pivot x expected");
			String pvx = tokens[i];
			++i;
			if (i == tokens.size())
				NIT_THROW_FMT(EX_INVALID_PARAMS, "pivot y expected");
			String pvy = tokens[i];
			int px, py;
			sscanf(pvx.c_str(), "%d", &px);
			sscanf(pvx.c_str(), "%d", &py);
			_options.pivotX = px;
			_options.pivotY = py;
		}
		else if (tokens[i] == "dump")
		{
			_makeDump = true;
		}
	}

	if (_options.makeMipmaps)
	{
		_options.makePOT = true;
		// TODO: make square also?
	}

	// Format constraints:
	switch (_options.format)
	{
	case PixelFormat::DXT1:
	case PixelFormat::DXT3: 
	case PixelFormat::DXT5: 
	case PixelFormat::DXT3_PA:
	case PixelFormat::DXT5_PA:
		// should be multiplied by 4
		if (_options.alignWidthHeight % 4)
			_options.alignWidthHeight = (_options.alignWidthHeight / 4 + 1) * 4;
		break;

	case PixelFormat::PVR2:
	case PixelFormat::PVR2A:
	case PixelFormat::PVR2A_PA:
	case PixelFormat::PVR4:
	case PixelFormat::PVR4A:
	case PixelFormat::PVR4A_PA:
		// Should be POT + SQUARE + at least 8 pixel
		if (!_options.makePOT)
			_options.makePOT = true;

		if (!_options.makeSquare)
			_options.makeSquare = true;

		if (_options.minWidthHeight < 8)
			_options.minWidthHeight = 8;
		break;
	}
}

void ImageHandler::setResize(const String& resize)
{
//	Handler::SetResize(resize);
	_resize = resize;
}

void ImageHandler::setPayload(const String& payload)
{
	Handler::setPayload(payload);
}

StreamWriter* ImageHandler::createDumpFile(String ext, String filename, bool convertExt)
{
	Ref<FileLocator> dumpPath = _entry->getPacker()->getDumpPath();

	if (dumpPath == NULL)
		return NULL;

	if (filename.empty())
		filename = _entry->getPacker()->getName() + "/" + _entry->getFilename();

	if (convertExt)
	{
		String fn, origExt;
		StringUtil::splitBaseFilename(_entry->getFilename(), fn, origExt);
		filename = fn + "." + ext;
	}
	else
	{
		filename = filename + "." + ext;
	}

	return dumpPath->create(filename);
}

void ImageHandler::generate()
{
	Ref<StreamSource> imgSource = _entry->getSource();

	if (_options.contentType == ContentType::UNKNOWN)
		return Handler::generate();

	if (_options.contentType.getValue() != ContentType::IMAGE_NTEX &&
		_options.contentType == imgSource->getContentType())
	{
		// If the specified codec and the content-type of the file is same, just copy as is.
		LOG(0, ".. '%s: %s': content type match - copy as is\n", _entry->getPacker()->getName().c_str(), _entry->getFilename().c_str());
		return Handler::generate();
	}

	if (_options.format == PixelFormat::UNKNOWN)
	{
		LOG(0, "*** '%s: %s': unknown pixel format - copy as is\n", _entry->getPacker()->getName().c_str(), _entry->getFilename().c_str());
		return Handler::generate();
	}

	Ref<ImageZen> zen = new ImageZen(imgSource);

	if (!zen->load())
	{
		LOG(0, "*** '%s: %s': can't load image - copy as is\n", _entry->getPacker()->getName().c_str(), _entry->getFilename().c_str());
		return Handler::generate();
	}

	int origWidth = zen->getWidth();
	int origHeight = zen->getHeight();

	_options.width = origWidth;
	_options.height = origHeight;

	if (!_resize.empty())
	{
		// Parse 'resize' tag
		StringVector tokens = StringUtil::tokenise(_resize);

		for (uint i=0; i<tokens.size(); ++i)
		{
			const String& token = tokens[i];

			bool bad = false;

			if (token == "sharpen")
			{
				_options.sharpen = true; // TODO: implement!
			}
			else if (token == "dump")
			{
				_makeDump = true;
			}
			else if (Wildcard::match("?*/?*", token))
			{
				// handle token like '1/2', '2/3'
				size_t p = token.find('/');
				int numerator = atoi(token.c_str());
				int denominator = atoi(token.c_str() + p + 1);

				if (numerator == 0 || denominator == 0)
					bad = true;
				else
				{
					_options.width = _options.width * numerator / denominator;
					_options.height = _options.height * numerator / denominator;
				}
			}
			else if (Wildcard::match("x?*", token))
			{
				// handle token like 'x2', 'x1.5'
				float multiplier = (float)atof(token.c_str() + 1);

				if (fabs(multiplier) < 0.001f)
					bad = true;
				else
				{
					_options.width = int(_options.width * multiplier);
					_options.height = int(_options.height * multiplier);
				}
			}
			else if (Wildcard::match("?*x?*", token))
			{
				// handle token like '64x64'
				size_t p = token.find('x');
				int w = atoi(token.c_str());
				int h = atoi(token.c_str() + p + 1);

				// like '0x640' : retain original width
				if (w != 0)
					_options.width = w;

				// like '640x0' : retain original height
				if (h != 0)
					_options.height = h;

				if (w == 0 && h == 0)
					bad = true;
			}
			else
			{
				bad = true;
			}

			if (bad)
			{
				LOG(0, "*** '%s: %s': bad resize option: %s\n", _entry->getPacker()->getName().c_str(), _entry->getFilename().c_str(), token.c_str());
			}
		}
	}

	Ref<MemoryBuffer::Writer> w = new MemoryBuffer::Writer();
	Ref<StreamWriter> dumpFile;

	if (_makeDump)
		dumpFile = createDumpFile(ContentType::toExtension(_options.contentType) + ".png");

	uint32 dataSize = zen->write(w, _options, dumpFile);

	if (dataSize == 0)
	{
		LOG(0, "*** '%s: %s': can't write image - copy as is\n", _entry->getPacker()->getName().c_str(), _entry->getFilename().c_str());
		return Handler::generate();
	}

	_entry->getData()->memorySize = dataSize;
	_entry->getData()->contentType = _options.contentType;

	return Handler::generate(new MemorySource(_entry->getSource(), w->getBuffer()));
}

////////////////////////////////////////////////////////////////////////////////

NS_BUNDLER_END;
