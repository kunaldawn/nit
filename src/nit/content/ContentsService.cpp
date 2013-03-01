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

#include "nit/content/ContentsService.h"

#include "nit/content/Content.h"
#include "nit/content/ContentManager.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

ContentsService::ContentsService()
: Service("ContentsService", NULL, SVC_CONTENTS)
{
	memset(_managerID, 0, sizeof(_managerID));
}

ContentsService::~ContentsService()
{

}

struct LessPriority
{
	bool operator() (ContentManager* a, ContentManager* b)
	{
		return a->getPriority() < b->getPriority();
	}
};

void ContentsService::Register(ContentManager* mgr)
{
	ContentManager::ManagerID id = mgr->getManagerID();
	if (id != ContentManager::MGR_CUSTOM)
	{
		if (_managerID[id] != NULL)
		{
			ContentManager* that = _managerID[id];
			NIT_THROW_FMT(EX_DUPLICATED, "ContentManager '%s' already with same id for '%s'",
				that->getName().c_str(), mgr->getName().c_str());
		}

		_managerID[id] = mgr;
	}

	// collect content type list which the manager can handle
	vector<ContentType>::type ctypes;
	mgr->allContentTypes(ctypes);

	vector<CMgrList*>::type touched; // list of entries which touched during this registration

	// add to CMgrLookup
	for (uint i=0; i < ctypes.size(); ++i)
	{
		CMgrList& l = _mgrLookup[ctypes[i]];
		l.push_back(mgr);
		touched.push_back(&l);
	}

	// sort CMgrList
	for (uint i=0; i < touched.size(); ++i)
	{
		CMgrList& l = *touched[i];

		std::sort(l.begin(), l.end(), LessPriority());
		// remove duplicated entries (sorted, so can use unique)
		l.erase(std::unique(l.begin(), l.end()), l.end()); 
	}
}

void ContentsService::Unregister(ContentManager* mgr)
{
	// collect content type list from manager to unregister
	vector<ContentType>::type ctypes;
	mgr->allContentTypes(ctypes);

	// remove from CMgrLookup and CMgrList
	for (uint i=0; i < ctypes.size(); ++i)
	{
		CMgrList& l = _mgrLookup[ctypes[i]];
		l.erase(std::remove(l.begin(), l.end(), mgr));
	}

	ContentManager::ManagerID id = mgr->getManagerID();
	if (id != ContentManager::MGR_CUSTOM)
	{
		if (_managerID[id] != mgr)
			return;

		_managerID[id] = NULL;
	}
}

ContentManager* ContentsService::findManager(const ContentType& ct)
{
	// Return content manager with highest priority
	CMgrList& l = _mgrLookup[ct];
	return l.empty() ? NULL : l[0];
}

ContentManager* ContentsService::findManager(StreamSource* source)
{
	const ContentType& ct = source->getContentType();

	// Check highest priority first
	CMgrList& l = _mgrLookup[ct];
	for (uint i=0; i < l.size(); ++i)
	{
		if (l[i]->canLink(source))
			return l[i];
	}
	return NULL;
}

void ContentsService::onRegister()
{
	require(g_Package);
}

void ContentsService::onUnregister()
{

}

void ContentsService::onInit()
{

}

void ContentsService::onFinish()
{

}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
