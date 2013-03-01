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

#include "nit/content/Texture.h"

#include "nit/content/ContentsService.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

Texture::Texture(TextureManager* manager)
: Content(manager)
{
	_minTexCoord = Vector2(0, 0);
	_maxTexCoord = Vector2(1, 1);

	_minFilter = MIN_LINEAR;
	_magFilter = MAG_LINEAR;
	_wrapModeS = WRAP_CLAMP_TO_EDGE;
	_wrapModeT = WRAP_CLAMP_TO_EDGE;
	_autoGenMipmap = false;
	memset(&_header, 0, sizeof(_header));
}

void Texture::loadHeader()
{
	if (_headerLoaded) return;

	if (_sourceImage == NULL && _source)
	{
		_sourceImage = new Image(_source);
	}

	if (_sourceImage)
	{
		_sourceImage->loadHeader();

		if (_sourceImage->hasError())
		{
			_error = true;
			return;
		}

		_header = _sourceImage->getHeader();
	}

	_headerLoaded = true;
}

void Texture::onLoad(bool async)
{
	if (_sourceImage == NULL && _source)
	{
		_sourceImage = new Image(_source);
	}

	if (_sourceImage)
	{
		_sourceImage->load(); // If there's exception, Texture::Load() fails also.

		if (_sourceImage->hasError())
		{
			_error = true;
			return;
		}

		_header = _sourceImage->getHeader();

		// TODO: NTEX can support HW buffer loading
		// Utilize HW header flag from Image header
	}
}

void Texture::onUnload()
{
	if (_sourceImage)
		_sourceImage->unload();
}

void Texture::onDispose()
{
	_sourceImage = NULL;
}

////////////////////////////////////////////////////////////////////////////////

TextureManager::TextureManager(const String& name, Package* package)
: ContentManager(name, package)
{

}

void TextureManager::onRegister()
{
	if (!require(g_Contents)) NIT_THROW(EX_NOT_FOUND);
}

void TextureManager::onUnregister()
{

}

void TextureManager::onInit()
{
	g_Contents->Register(this);
}

void TextureManager::onFinish()
{
	TextureCache disposing;
	disposing.swap(_cached);

	for (TextureCache::iterator itr = disposing.begin(), end = disposing.end(); itr != end; ++itr)
	{
		(*itr)->dispose();
	}

	disposing.clear();

	g_Contents->Unregister(this);
}

bool TextureManager::canLink(StreamSource* source)
{
	// TODO: implement this
	return true;
}

void TextureManager::allContentTypes(vector<ContentType>::type& outResults)
{
	// TODO: ImageManager::AllContentTypes(outResults);
	outResults.push_back(ContentType::IMAGE_JPEG);
	outResults.push_back(ContentType::IMAGE_PNG);
	outResults.push_back(ContentType::IMAGE_PVR);
	outResults.push_back(ContentType::IMAGE_CCZ);
	outResults.push_back(ContentType::IMAGE_NTEX);
}

void TextureManager::onDispose(Content* content)
{
	Texture* tex = static_cast<Texture*>(content);

	_cached.erase(tex);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
