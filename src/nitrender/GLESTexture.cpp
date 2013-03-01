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

#include "NITRENDER_pch.h"

#include "nitrender/GLESTexture.h"

#include "nitrender/RenderService.h"
#include "nitrender/RenderContext.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

GLESTexture::GLESTexture(StreamSource* source, GLESTextureManager* manager)
: Texture(manager)
{
	_handle = new GLESTextureHandle();
	_activateResult = AR_NOT_LOADED;
	_texParamDirty = true;

	_source = source;

	if (source)
		_linked = true;
}

GLESTexture::GLESTexture(Image* sourceImage, GLESTextureManager* manager)
: Texture(manager)
{
	_handle = new GLESTextureHandle();
	_activateResult = AR_NOT_LOADED;

	_sourceImage = sourceImage;

	if (sourceImage)
		_linked = true;
}

GLESTexture::GLESTexture(uint16 width, uint16 height, const PixelFormat& pixelFormat, GLESTextureManager* manager, uint16 contentWidth, uint16 contentHeight)
: Texture(manager)
{
	_handle = new GLESTextureHandle();
	_activateResult = AR_NOT_LOADED;

	_source = NULL;
	_sourceImage = NULL;

	if (contentWidth == 0) contentWidth = width;
	if (contentHeight == 0) contentHeight = height;

	_header.pixelFormat	= pixelFormat;

	_header.width			= width;
	_header.height			= height;

	_header.sourceWidth	= contentWidth;
	_header.sourceHeight	= contentHeight;

	_header.contentLeft	= 0;
	_header.contentTop		= 0;
	_header.contentRight	= contentWidth;
	_header.contentBottom	= contentHeight;
	_header.memorySize		= 0;
	_header.mipmapCount	= 1;

	_linked = true;
	_loaded = true;
}

GLESTexture::ActivateResult GLESTexture::activate(RenderContext* ctx)
{
	if (_activateResult == AR_OK)
		return AR_OK;

	if (_error)
		_activateResult = AR_HAS_ERROR;
	else if (_loading)
		_activateResult = AR_LOADING;
	else if (!_loaded)
		_activateResult = AR_NOT_LOADED;
	else if (_disposed)
		_activateResult = AR_DISPOSED;
	else
		_activateResult = onActivate(ctx);

	if (_activateResult != AR_OK && _proxy)
	{
		GLESTexture* proxy = dynamic_cast<GLESTexture*>(_proxy.get());
		if (proxy) 
			return proxy->activate(ctx);
	}

	return _activateResult;
}

void GLESTexture::deactivate()
{
	if (_activateResult != AR_OK)
		return;

	onDeactivate();

	_activateResult = AR_NOT_LOADED;
}

GLESTexture* GLESTexture::getActivatedProxy()
{
	if (isActivated())
		return this;

	GLESTexture* proxy = dynamic_cast<GLESTexture*>(_proxy.get());
	if (proxy)
		return proxy->getActivatedProxy();

	return NULL;
}

GLESTexture::ActivateResult GLESTexture::onActivate(RenderContext* ctx)
{
	if (_sourceImage)
	{
		_sourceImage->load();
		bool needPOT = !g_Render->isTextureNPOTSupported();
		if (needPOT && _sourceImage->makePot(false))
			_header = _sourceImage->getHeader();
	}

	_handle->generate(ctx);
	_handle->setCacheFootprint(_header.memorySize);

	GLuint handle = _handle->useHandle(ctx);

	// Bind GL texture
	// TODO: lockGL()
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, handle);

	bool ok = false;
	PixelFormat fmt = getPixelFormat();

	for (int level=0; level<_header.mipmapCount; ++level)
	{
		if (fmt.isCompressed())
			ok = createGLCompTex(ctx, level);
		else
			ok = createGLTex(ctx, level);
	}

	if (ok)
	{
		_texParamDirty = true;
		applyTexParams(ctx);
	}
	else
	{
		_handle->invalidate();

		_error = true;
		LOG(0, "*** failed to activate '%s' of '%s'", getSourceUrl().c_str(), fmt.getName().c_str());
		return AR_HAS_ERROR;
	}

	// Unload non-managed _sourceImage
 	if (_sourceImage && _sourceImage->getManager() == NULL)
 		_sourceImage->unload();

	// TODO: Does the manager retain _sourceImage as loaded? 
	// (ex: Without sufficient VRAM, so we preserve the data itself on memory, 
	// and do activate/deactivate when it's needed while rendering)

	return AR_OK;
}

bool GLESTexture::createGLTex(RenderContext* ctx, int level)
{
	// TODO: mipmap only works when it's POT!

	GLsizei width = _header.width >> level;
	GLsizei height = _header.height >> level;
	GLint border = 0;

	// glTexImage2D allows pixels to be NULL.
	// This case the texture may contain garbage image.
	// Most rendering to render target starts with clear so this is not a problem.
	// But on additive rendering (ex: motion-blur) you must clear the first frame.
	GLvoid* pixels = _sourceImage ? _sourceImage->getMipData(level) : NULL;

	switch (_header.pixelFormat)
	{
	case PixelFormat::RGBA_8888:
	case PixelFormat::RGBA_8888_PA:
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, pixels); break;

		// TODO: platform-wise spec
	case PixelFormat::BGRA_8888:
	case PixelFormat::BGRA_8888_PA:
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, border, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels); break;

	case PixelFormat::RGB_888:
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, pixels); break;

	case PixelFormat::RGBA_4444:
	case PixelFormat::RGBA_4444_PA:
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, border, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, pixels); break;

	case PixelFormat::RGBA_5551:
	case PixelFormat::RGBA_5551_PA:
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, border, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, pixels); break;

	case PixelFormat::RGB_565:
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, width, height, border, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixels); break;

	case PixelFormat::AL_88:
		glTexImage2D(GL_TEXTURE_2D, level, GL_LUMINANCE_ALPHA, width, height, border, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pixels); break;

	case PixelFormat::L8:
		glTexImage2D(GL_TEXTURE_2D, level, GL_LUMINANCE, width, height, border, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels); break;

	case PixelFormat::A8:
		glTexImage2D(GL_TEXTURE_2D, level, GL_ALPHA, width, height, border, GL_ALPHA, GL_UNSIGNED_BYTE, pixels); break;

	default:
		return false;
	}

	return true;
}

bool GLESTexture::createGLCompTex(RenderContext* ctx, int level)
{
	PixelFormat fmt = getPixelFormat();

	int bpp = PixelFormat::calcBitsPerPixel(fmt);

	// compress texture always needs a data pointer
	if (_sourceImage == NULL)
		return false;

	// TODO: mipmap only works when its POT!

	GLenum glFormat;
	GLsizei width = _header.width >> (level);
	GLsizei height = _header.height >> (level);
	GLsizei size = width * height * bpp >> 3;
	GLint border = 0;
	GLvoid* data = _sourceImage->getMipData(level);

	switch (fmt.getValue())
	{
	case PixelFormat::PVR2:
		ASSERT_THROW(width == height, EX_INVALID_PARAMS);
		glFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG; break;

	case PixelFormat::PVR2A:
	case PixelFormat::PVR2A_PA:
		ASSERT_THROW(width == height, EX_INVALID_PARAMS);
		glFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG; break;

	case PixelFormat::PVR4:
		ASSERT_THROW(width == height, EX_INVALID_PARAMS);
		glFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG; break;

	case PixelFormat::PVR4A:
	case PixelFormat::PVR4A_PA:
		ASSERT_THROW(width == height, EX_INVALID_PARAMS);
		glFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG; break;

#ifdef GL_ETC1_RGB8_OED
	case PixelFormat::ETC:
		// TODO: Study more about ETC format
		glFormat = GL_ETC1_RGB8_OES; break;
#endif
	default:							
		return false;
	}

	glCompressedTexImage2D(GL_TEXTURE_2D, level, glFormat, width, height, border, size, data);

	return true;
}

void GLESTexture::onDeactivate()
{
	// TODO: lockGL()

	// Release the GL texture
	_handle->invalidate();
}

void GLESTexture::loadHeader()
{
	Texture::loadHeader();

	if (_sourceImage)
	{
		bool needPOT = !g_Render->isTextureNPOTSupported();
		if (needPOT && _sourceImage->makePot(false))
			_header = _sourceImage->getHeader();
	}
}

void GLESTexture::onLoad(bool async)
{
	Texture::onLoad(async);

	if (_sourceImage)
	{
		bool needPOT = !g_Render->isTextureNPOTSupported();
		if (needPOT && _sourceImage->makePot(false))
			_header = _sourceImage->getHeader();
	}
}

void GLESTexture::onUnload()
{
	deactivate();

	Texture::onUnload();
}

bool GLESTexture::applyTexParams(RenderContext* ctx)
{
	if (_handle == 0) 
		return false;

	if (!_texParamDirty)
		return true;

	_texParamDirty = false;

	glBindTexture(GL_TEXTURE_2D, _handle->useHandle(ctx));

	switch (_minFilter)
	{
	case MIN_NEAREST:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); break;

	case MIN_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); break;

	case MIN_NEAREST_MIPMAP_NEAREST:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); break;

	case MIN_LINEAR_MIPMAP_NEAREST:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); break;

	case MIN_NEAREST_MIPMAP_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); break;

	case MIN_LINEAR_MIPMAP_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); break;

	default:
		return false;
	}

	switch (_magFilter)
	{
	case MAG_NEAREST:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); break;

	case MAG_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); break;

	default:
		return false;
	}

	switch (_wrapModeS)
	{
	case WRAP_CLAMP_TO_EDGE:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); break;

	case WRAP_REPEAT:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); break;

	default:
		return false;
	}

	switch (_wrapModeT)
	{
	case WRAP_CLAMP_TO_EDGE:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); break;

	case WRAP_REPEAT:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); break;

	default: 
		return false;
	}

	if (_autoGenMipmap)
	{
		glGenerateMipmapOES(GL_TEXTURE_2D);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////

GLESTextureManager::GLESTextureManager(Package* package)
: TextureManager("GLESTextureManager", package)
{

}

void GLESTextureManager::onRegister()
{
	TextureManager::onRegister();

	require(g_Render);
}

void GLESTextureManager::onUnregister()
{
	TextureManager::onUnregister();
}

void GLESTextureManager::onInit()
{
	TextureManager::onInit();
}

void GLESTextureManager::onFinish()
{
	TextureManager::onFinish();
}

Ref<Content> GLESTextureManager::createFrom(StreamSource* source)
{
	Ref<Texture> tex;
	Ref<Image> sourceImage;

	tex = new GLESTexture(source, this);

	// TODO: Do not insert here, consider texture's constructor or Content.setManager()
	_cached.insert(tex.get());

	return tex.get();
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
