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

#include "nitrender/GLESRenderSpec.h"

#include "nitrender/RenderService.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

GLESRenderSpec::GLESRenderSpec(GLESRenderDevice* device)
{
	_vendor = (const char*) glGetString(GL_VENDOR);
	_renderer = (const char*) glGetString(GL_RENDERER);
	_version = (const char*) glGetString(GL_VERSION);
	StringVector extensions = StringUtil::tokenise((const char*) glGetString(GL_EXTENSIONS));
	_extensions.insert(extensions.begin(), extensions.end());

	// GLES 1.1
	// http://www.khronos.org/registry/gles/

	setCapable(RENDER_BLENDING);
	setCapable(RENDER_OCCLUSION);
	setCapable(RENDER_POINT_SPRITES);

	int value;
	value = 0; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value); _maxTextureSize = value;

	// GLES 1.1 extension 
	if (hasExtension("GL_OES_texture_npot"))
	{
		setCapable(TEXTURE_NPOT);
		setCapable(RT_TEXTURE_NPOT); // TODO: check again
	}

	if (hasExtension("GL_APPLE_texture_2D_limited_npot"))
	{
		setCapable(TEXTURE_NPOT);
		setCapable(RT_TEXTURE_NPOT); // TODO: check again
		setCapable(TEXTURE_NPOT_LIMITED);
	}

	if (hasExtension("GL_EXT_texture_compression_dxt1") && hasExtension("GL_EXT_texture_compression_dxt3") && hasExtension("GL_EXT_texture_compression_dxt5"))
	{
		setCapable(FORMAT_TEXTURE_DXT);
	}

	setCapable(FORMAT_TEXTURE_PVRTC, hasExtension("GL_IMG_texture_compression_pvrtc"));
	setCapable(FORMAT_TEXTURE_ETC1, hasExtension("GL_OES_compressed_ETC1_RGB8_texture"));

	// platform specific
#if defined(NIT_IOS) || defined(NIT_ANDROID)
	setCapable(DEVICE_MAIN_MEM_SHARES_FRAME);
	setCapable(DEVICE_MAIN_MEM_SHARES_TEX);
	setCapable(DEVICE_MAIN_MEM_SHARES_GEOMETRY);
#endif

#if defined(NIT_ANDROID)
	setCapable(DEVICE_POSSIBLE_LOST);
#endif

	// TODO: temporal
	if (!isCapable(TEXTURE_NPOT))
		g_Render->textureNeedPOT();

	if (!isCapable(RT_TEXTURE_NPOT))
		g_Render->renderTargetNeedPOT();
}

bool GLESRenderSpec::hasExtension(const String& ext)
{
	return _extensions.find(ext) != _extensions.end();
}

bool GLESRenderSpec::hasExtensions(const String& pattern)
{
	for (StringSet::iterator itr = _extensions.begin(), end = _extensions.end(); itr != end; ++itr)
	{
		const String& ext = *itr;
		if (Wildcard::match(pattern, ext))
			return true;
	}
	return false;
}

void GLESRenderSpec::findExtensions(const String& pattern, StringVector& outResults)
{
	for (StringSet::iterator itr = _extensions.begin(), end = _extensions.end(); itr != end; ++itr)
	{
		const String& ext = *itr;
		if (Wildcard::match(pattern, ext))
			outResults.push_back(ext);
	}
}

void GLESRenderSpec::report(LogChannel* ch)
{
	LOG_SCOPE(ch, "++ * GLESRenderSpec Report\n");

	LOG(ch, "++ - GL_VENDOR: %s\n", _vendor.c_str());
	LOG(ch, "++ - GL_RENDERER: %s\n", _renderer.c_str());
	LOG(ch, "++ - GL_VERSION: %s\n", _version.c_str());
	LOG(ch, "++ + GL_EXTENSIONS:\n");

	for (StringSet::iterator itr = _extensions.begin(), end = _extensions.end(); itr != end; ++itr)
	{
		LOG(ch, "++   - %s\n", itr->c_str());
	}

	RenderSpec::report(ch);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
