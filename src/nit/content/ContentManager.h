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

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// TODO: Add a ContentService, like ServiceContext provide a ContentManagerID, use them like a g_Texture?
// Otherwise make each ContentManager a service?
// =>
// A specific service will not serve user code when their ContentManager not provided as a whole.
// Therefore, The service should check registered modules during its onInit() and then setup by its own context.
// In example, RenderService of each rendering implementation (gl, dx etc) should register its own tex manager :
// g_Cocos->GetTexManager()
// g_Nit3D->GetTexManager()

class NIT_API ContentManager : public Module
{
public:
	enum ManagerID
	{
		MGR_CUSTOM						= 0,

		MGR_IMAGE,
		MGR_TEXTURE,
		MGR_MATERIAL,
		MGR_SHADER,
		MGR_MESH,
		MGR_SKELETON,
		MGR_FONT,
		MGR_OVERLAY,
		MGR_PARTICLE,

		MANAGER_ID_COUNT
	};

public:
	ContentManager(const String& name, Package* package);

public:
	virtual ManagerID					getManagerID() = 0;
	virtual void						allContentTypes(vector<ContentType>::type& outResults) = 0;
	virtual float						loadOrder() = 0;
	virtual float						getPriority() = 0;
	virtual bool						canLink(StreamSource* source) = 0;

public:
	virtual Ref<Content>				createFrom(StreamSource* source=NULL) = 0;

protected:
	friend class Content;
	virtual void						onDispose(Content* content);
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
