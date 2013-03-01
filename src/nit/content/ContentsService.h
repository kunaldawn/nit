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

#include "nit/app/Service.h"
#include "nit/content/ContentManager.h"

#include "nit/io/ContentTypes.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#define g_Contents						static_cast<nit::ContentsService*>(g_Service->getService(::nit::Service::SVC_CONTENTS))

////////////////////////////////////////////////////////////////////////////////

class TextureManager;

////////////////////////////////////////////////////////////////////////////////

class NIT_API ContentsService : public Service
{
public:
	ContentsService();
	virtual ~ContentsService();

public:
	inline ContentManager*				getManager(ContentManager::ManagerID id)	{ return _managerID[id]; }

	template <typename TContent> 
	typename TContent::ManagerClass*	getManager();

	template <typename TContent>
	Ref<TContent>						createFrom(StreamSource* source);

public:
	void								Register(ContentManager* mgr);
	void								Unregister(ContentManager* mgr);
	ContentManager*						findManager(const ContentType& ct);
	ContentManager*						findManager(StreamSource* source);

protected:								// Module implementation
	virtual void						onRegister();
	virtual void						onUnregister();
	virtual void						onInit();
	virtual void						onFinish();

private:
	typedef vector<ContentManager*>::type CMgrList;
	typedef map<ContentType::ValueType, CMgrList>::type CMgrLookup;

	CMgrLookup							_mgrLookup;

	ContentManager*						_managerID[ContentManager::MANAGER_ID_COUNT];
};

////////////////////////////////////////////////////////////////////////////////

template <typename TContent>
typename TContent::ManagerClass* ContentsService::getManager()
{
	typedef typename TContent::ManagerClass TManager;

	return static_cast<TManager*>(_managerID[TManager::MANAGER_ID]);
}

template <typename TContent>
Ref<TContent> ContentsService::createFrom(StreamSource* source)
{
	typedef typename TContent::ManagerClass TManager;

	TManager* mgr = getManager<TContent>();
	if (mgr == NULL) return NULL;

	Ref<Content> content = mgr->createFrom(source);

	return dynamic_cast<TContent*>(content.get());
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
