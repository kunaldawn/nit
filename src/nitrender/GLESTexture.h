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

#include "nitrender/nitrender.h"

#include "nitrender/GLESRenderDevice.h"

#include "nit/content/Texture.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class RenderContext;
class GLESTexture;
class GLESTextureManager;

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API GLESTexture : public Texture
{
public:
	GLESTexture(StreamSource* source, GLESTextureManager* manager = NULL);
	GLESTexture(Image* sourceImage, GLESTextureManager* manager = NULL);
	GLESTexture(uint16 width, uint16 height, const PixelFormat& pixelFormat, GLESTextureManager* manager = NULL, uint16 contentWidth=0, uint16 contentHeight=0); // for render target

public:
	GLESTextureHandle*					getHandle()								{ return _handle; }

	virtual void						loadHeader();

public:
	bool								isActivated()							{ return _activateResult == AR_OK; }

	GLESTexture*						getActivatedProxy();

	enum ActivateResult
	{
		AR_OK = 0,
		AR_NOT_LOADED,
		AR_LOADING,
		AR_HAS_ERROR,
		AR_DISPOSED,
	};

	ActivateResult						activate(RenderContext* ctx);
	void								deactivate();

public:									// Texture Overrides
	virtual void						setMinFilter(MinFilter minFilter)		{ Texture::setMinFilter(minFilter); _texParamDirty = true; }
	virtual void						setMagFilter(MagFilter magFilter)		{ Texture::setMagFilter(magFilter); _texParamDirty = true; }
	virtual void						setWrapModeS(WrapMode wrapMode)			{ Texture::setWrapModeS(wrapMode); _texParamDirty = true; }
	virtual void						setWrapModeT(WrapMode wrapMode)			{ Texture::setWrapModeT(wrapMode); _texParamDirty = true; }
	virtual void						setAutoGenMipmap(bool flag)				{ Texture::setAutoGenMipmap(flag); _texParamDirty = true; }

public:									// TODO: Do not expose to user code
	bool								applyTexParams(RenderContext* ctx);

protected:								// Content Impl
	virtual ActivateResult				onActivate(RenderContext* ctx);
	virtual void						onDeactivate();
	virtual void						onLoad(bool async);
	virtual void						onUnload();

protected:
	Ref<GLESTextureHandle>				_handle;
	ActivateResult						_activateResult;

	bool								_texParamDirty;

	bool								createGLTex(RenderContext* ctx, int level);
	bool								createGLCompTex(RenderContext* ctx, int level);
};

////////////////////////////////////////////////////////////////////////////////

class NITRENDER_API GLESTextureManager : public TextureManager
{
public:
	GLESTextureManager(Package* package);

protected:								// Module Impl
	virtual void						onRegister();
	virtual void						onUnregister();
	virtual void						onInit();
	virtual void						onFinish();

protected:								// ContentManager impl
	virtual Ref<Content>				createFrom(StreamSource* source);

	typedef set<RefCache<Texture> >::type ContentCache;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
