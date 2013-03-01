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
#include "nit/content/Image.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class Texture;
class TextureManager;

////////////////////////////////////////////////////////////////////////////////

class NIT_API Texture : public Content
{
public:
	typedef TextureManager				ManagerClass;

	typedef Image::Header				Header;

	Texture(TextureManager* manager);

	enum MinFilter
	{
		MIN_NEAREST,
		MIN_LINEAR,
		MIN_NEAREST_MIPMAP_NEAREST,
		MIN_LINEAR_MIPMAP_NEAREST,
		MIN_NEAREST_MIPMAP_LINEAR,
		MIN_LINEAR_MIPMAP_LINEAR,
	};

	enum MagFilter
	{
		MAG_NEAREST,
		MAG_LINEAR,
	};

	enum WrapMode
	{
		WRAP_CLAMP_TO_EDGE,
		WRAP_REPEAT,
	};

public:
	virtual void						loadHeader();

public:
	Image*								getSourceImage()						{ return _sourceImage; }
	const Header&						getHeader()								{ return _header; }
	PixelFormat							getPixelFormat()						{ return _header.pixelFormat; }

	const Vector2&						getMinTexCoord()						{ return _minTexCoord; }
	const Vector2&						getMaxTexCoord()						{ return _maxTexCoord; }
	MinFilter							getMinFilter()							{ return _minFilter; }
	MagFilter							getMagFilter()							{ return _magFilter; }
	WrapMode							getWrapModeS()							{ return _wrapModeS; }
	WrapMode							getWrapModeT()							{ return _wrapModeT; }
	bool								getAutoGenMipmap()						{ return _autoGenMipmap; }

	virtual void						setMinTexCoord(const Vector2& min)		{ _minTexCoord = min; }
	virtual void						setMaxTexCoord(const Vector2& max)		{ _maxTexCoord = max; }
	virtual void						setMinFilter(MinFilter minFilter)		{ _minFilter = minFilter; }
	virtual void						setMagFilter(MagFilter magFilter)		{ _magFilter = magFilter; }
	virtual void						setWrapModeS(WrapMode wrapMode)			{ _wrapModeS = wrapMode; }
	virtual void						setWrapModeT(WrapMode wrapMode)			{ _wrapModeT = wrapMode; }
	virtual void						setAutoGenMipmap(bool flag)				{ _autoGenMipmap = flag; }

protected:								// Content Impl
	virtual size_t						getMemorySize()							{ return _header.memorySize; }
	virtual void						onLoad(bool async);
	virtual void						onUnload();
	virtual void						onDispose();

protected:
	Header								_header;

	Ref<Image>							_sourceImage;

	Vector2								_minTexCoord;
	Vector2								_maxTexCoord;

	MinFilter							_minFilter;
	MagFilter							_magFilter;
	WrapMode							_wrapModeS;
	WrapMode							_wrapModeT;
	bool								_autoGenMipmap;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API TextureManager : public ContentManager
{
public:
	const static ManagerID				MANAGER_ID = MGR_TEXTURE;

public:
	TextureManager(const String& name, Package* package);

protected:								// Module Impl
	virtual void						onRegister();
	virtual void						onUnregister();

protected:								// ContentManager impl
	virtual void						onInit();
	virtual void						onFinish();

	virtual ManagerID					getManagerID()							{ return MANAGER_ID; }
	virtual void						allContentTypes(vector<ContentType>::type& outResults);
	virtual float						loadOrder()								{ return 0.0f; } // TODO: expose to cfg settings
	virtual float						getPriority()							{ return 0.0f; } // TODO: expose to cfg settings
	virtual bool						canLink(StreamSource* source);
	virtual void						onDispose(Content* content);

protected:
	typedef set<RefCache<Texture> >::type TextureCache;

	TextureCache						_cached;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
