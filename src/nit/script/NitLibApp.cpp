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

#include "nit/script/NitLibApp.h"
#include "nit/script/NitLibCore.h"

#include "nit/script/NitBindMacro.h"

#include "nit/app/AppBase.h"
#include "nit/app/Package.h"
#include "nit/app/Session.h"
#include "nit/app/Module.h"

#include "nit/app/PackArchive.h"

#include "nit/content/Content.h"
#include "nit/content/ContentManager.h"
#include "nit/content/ContentsService.h"

#include "nit/input/InputService.h"
#include "nit/input/InputUser.h"
#include "nit/input/InputDevice.h"
#include "nit/input/InputSource.h"
#include "nit/input/InputCommand.h"

#include "nit/content/Image.h"
#include "nit/content/Texture.h"

#include "nit/logic/Object.h"
#include "nit/logic/Component.h"
#include "nit/logic/Transform.h"
#include "nit/logic/Feature.h"
#include "nit/logic/AutomataComponent.h"

#include "nit/data/Settings.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

ScriptObject::ScriptObject(ScriptPeer* peer) 
{
	_peer = peer;
}

void ScriptObject::onAttach()
{
	if (_peer == NULL) return;

	_peer->callMethod("onAttach", 0);

	Object::onAttach();
}

void ScriptObject::onDispose()
{
	if (_peer == NULL) return;

	_peer->callMethod("onDispose", 0);
	_peer = NULL;

	Object::onDispose();
}

ScriptObject::~ScriptObject()
{

}

DataSchema* ScriptObject::getDataSchema()
{
	if (_dataSchema) return _dataSchema;

	HSQUIRRELVM v = _peer->getWorker();

	_peer->pushObject(v);

	// Inspect class._schema which registered at ScriptDataSchema.Register(), NULL if none
	sq_getclass(v, -1);

	sq_pushstring(v, "_schema", -1);
	if (SQ_SUCCEEDED(sq_get(v, -2)))
	{
		_dataSchema = NitBind::get<DataSchema>(v, -1);
		sq_poptop(v);
	}
	sq_poptop(v);

	return _dataSchema;
}

void ScriptObject::onLoadBegin(DataObjectContext* context)
{
	HSQUIRRELVM v = _peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = _peer->callMethod("onLoadBegin", 1, false);

	// TODO: check sr and throw if error
}

void ScriptObject::onLoadEnd(DataObjectContext* context)
{
	HSQUIRRELVM v = _peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = _peer->callMethod("onLoadEnd", 1, false);

	// TODO: check sr and throw if error
}

void ScriptObject::onSaveBegin(DataObjectContext* context)
{
	// TODO: check if purged instance!

	HSQUIRRELVM v = _peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = _peer->callMethod("onSaveBegin", 1, false);

	// TODO: check sr and throw if error
}

void ScriptObject::onSaveEnd(DataObjectContext* context)
{
	HSQUIRRELVM v = _peer->getWorker();

	NitBind::push(v, context);
	ScriptResult sr = _peer->callMethod("onSaveEnd", 1, false);

	// TODO: check sr and throw if error
}

Ref<DataObject> ScriptObject::adaptFunc(ScriptDataSchema* schema, HSQUIRRELVM v, int instIdx)
{
	return NitBind::get<DataObject>(v, instIdx);
}

SQInteger ScriptObject::pushFunc(ScriptDataSchema* schema, HSQUIRRELVM v, DataObject* object)
{
	return NitBind::push(v, object);
}

bool ScriptObject::getFunc(DataObject* object, DataProperty* prop, DataValue& outValue)
{
	ScriptObject* sobj = (ScriptObject*)object;
	ScriptDataProperty* sprop = (ScriptDataProperty*)prop;
	return ScriptDataProperty::peerGet(sobj->_peer, sprop, outValue);
}

bool ScriptObject::setFunc(DataObject* object, DataProperty* prop, DataValue& value)
{
	ScriptObject* sobj = (ScriptObject*)object;
	ScriptDataProperty* sprop = (ScriptDataProperty*)prop;
	return ScriptDataProperty::peerSet(sobj->_peer, sprop, value);
}

////////////////////////////////////////////////////////////////////////////////

ScriptComponent::ScriptComponent(ScriptPeer* peer)
: _peer(peer)
{
	HSQUIRRELVM v = _peer->getRuntime()->getWorker();

	const char* name = NULL;
	SQInteger top = sq_gettop(v);

	_peer->pushObject(v);
	sq_pushstring(v, "_classname", -1);
	sq_get(v, -2);
	sq_getstring(v, -1, &name);

	if (name) _name = name;
	sq_settop(v, top);

}

bool ScriptComponent::onValidate()
{
	if (_peer == NULL) return false;

	HSQUIRRELVM v = _peer->getRuntime()->getWorker();

	ScriptResult r = _peer->callMethod("onValidate", 0, true);

	if (r == SCRIPT_CALL_NO_METHOD) return true;

	bool ok = r == SCRIPT_CALL_OK;

	if (ok)
	{
		ok = NitBind::isNull(v, -1) || NitBind::getBool(v, -1);
		sq_poptop(v);
	}

	return ok;
}

void ScriptComponent::onEventStart()
{
	if (_peer == NULL) return;

	_peer->callMethod("onEventStart", 0);
}

void ScriptComponent::onActivate()
{
	if (_peer == NULL) return;

	_peer->callMethod("onActivate", 0);
}

void ScriptComponent::onDeactivate()
{
	if (_peer) _peer->callMethod("onDeactivate", 0);
}

void ScriptComponent::onDispose()
{
	if (_peer == NULL) return;

	_peer->callMethod("onDispose", 0);
	_peer = NULL;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TClass>
bool ScriptFeature::callMethod(const char* name, TClass* target)
{
	HSQUIRRELVM v = _peer->getRuntime()->getWorker();
	if (v == NULL) return false;
	NitBind::push(v, target);

	ScriptResult r = _peer->callMethod(name, 1, true);

	if (r == SCRIPT_CALL_NO_METHOD) return true;

	bool ok = r == SCRIPT_CALL_OK;

	if (ok)
	{
		ok = NitBind::isNull(v, -1) || NitBind::getBool(v, -1);
		sq_poptop(v);
	}

	return ok;
}

ScriptFeature::ScriptFeature(const String& name, Package* package, HSQUIRRELVM v, SQInteger idx)
: Feature(name, package)
{
	_peer = new ScriptPeer(v, idx);

	// If package got unloaded during a session alive, isolated module problem may occur.
	// To prevent this, keep them not to be compacted during current session.

	// TODO: How about to use lookup()?
	if (package)
		package->setStayForCurrent(true);

	if (g_Session)
		g_Session->channel()->bind(Events::OnSessionStop, this, &ScriptFeature::onSessionStop);
}

void ScriptFeature::onRegister()
{
	_peer->callMethod("onRegister", 0);
}

void ScriptFeature::onUnregister()
{
	_peer->callMethod("onUnregister", 0);
}

void ScriptFeature::onInit()
{
	_peer->callMethod("onInit", 0);
}

void ScriptFeature::onFinish()
{
	_peer->callMethod("onFinish", 0);
}

bool ScriptFeature::onInstall(Object* obj)
{
	return callMethod("onInstall", obj);
}

bool ScriptFeature::onUninstall(Object* obj)
{
	return callMethod("onUninstall", obj);
}

void ScriptFeature::onSessionStop(const Event* evt)
{
	if (isInitialized())
		g_App->Unregister(this);
}

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Module, RefCounted, incRefCount, decRefCount);

class NB_Module : TNitClass<Module>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(package),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(isRequired,	"(mod: Module): bool"),
			FUNC_ENTRY	(_dump),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IEventSink>(v);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(package)				{ return push(v, self(v)->getPackage()); }

	NB_FUNC(isRequired)					{ return push(v, self(v)->isRequired(get<Module>(v, 2))); }

	// init(), finish() is used on only C++ and will be handled at app.Register() automatically

	// for require(), we expose it on Feature class not here to avoid misuse.

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		return pushFmt(v, "(%s -> %08X)", o->getName().c_str(), o);
	}

	NB_FUNC(_dump)
	{
		LOG(0, "  requires:\n");
		self(v)->printDependancy(2);
		return push(v, true);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Feature, Module, incRefCount, decRefCount);

class NB_Feature : TNitClass<Feature>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(require,		"(mod: Module)"),
			FUNC_ENTRY_H(mark,			"(comp: Component)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(require)					{ self(v)->require(get<Module>(v, 2)); return 0; }
	NB_FUNC(mark)						{ return push(v, self(v)->mark(get<Component>(v, 2))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ScriptFeature, Feature, IScriptRef::scriptIncRef, IScriptRef::scriptDecRef);

class NB_ScriptFeature : TNitClass<ScriptFeature>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(name = _classname, package = runtime.Locator)"),
			FUNC_ENTRY	(_inherited),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IScriptEventSink>(v);
		addInterface<IScriptRef>(v);
	}

	NB_CONS()
	{
		const char* name = optString(v, 2, NULL);
		if (name == NULL)
		{
			push(v, "_classname");
			sq_get(v, 1);
			sq_getstring(v, -1, &name);
		}

		ScriptRuntime* runtime = ScriptRuntime::getRuntime(v);

		Package* package = opt<Package>(v, 3, NULL);

		if (package == NULL)
			package = dynamic_cast<Package*>(runtime->getLocator());

		if (package == NULL)
			sq_throwerror(v, "can't assign package for module");

		ScriptFeature* mod = new ScriptFeature(name, package, v, 1);

		runtime->retain(mod);
		sq_setinstanceup(v, 1, mod);

		return 0;
	}

	NB_FUNC(_inherited)					{ return SQ_OK; } // allow inheritance

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		SQInteger top = sq_gettop(v);
		const char* clsname = "???";
		push(v, "_classname");
		sq_get(v, 1);
		sq_getstring(v, -1, &clsname);
		sq_settop(v, top);
		return pushFmt(v, "(%s: %s -> %08X)", o->getName().c_str(), clsname, o);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Service, Module, incRefCount, decRefCount);

class NB_Service : TNitClass<Service>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(serviceID),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(serviceID)				{ return push(v, (int)self(v)->getServiceID()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_RAW_PTR(NIT_API, nit::ModuleRegistry, NULL);

class NB_ModuleRegistry : TNitClass<ModuleRegistry>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(get,			"(name or UID): Module"),
			FUNC_ENTRY_H(find,			"(pattern): Module[]"),
			FUNC_ENTRY	(_get),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(get)						{ return push(v, self(v)->get(getString(v, 2))); }

	NB_FUNC(find)
	{
		std::vector<Module*> sl; self(v)->find(getString(v, 2), sl);
		sq_newarray(v, 0);
		for (uint i=0; i<sl.size(); ++i)
			arrayAppend(v, -1, sl[i]);
		return 1;
	}

	NB_FUNC(_get)
	{
		Module* m = NULL;

		type* self = self_noThrow(v);
		if (self) m = self->get(getString(v, 2));
		if (m) return push(v, m);

		// not found - clean failure
		sq_reseterror(v); 
		return SQ_ERROR;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_RAW_PTR(NIT_API, nit::AppBase, NULL);

class NB_AppBase : TNitClass<AppBase>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(title),
			PROP_ENTRY_R(edition),
			PROP_ENTRY_R(version),

			PROP_ENTRY_R(platform),
			PROP_ENTRY_R(build),
			PROP_ENTRY_R(deviceModel),
			PROP_ENTRY_R(deviceForm),
			PROP_ENTRY_R(osVersion),

			PROP_ENTRY_R(pluginPath),
			PROP_ENTRY_R(patchPath),
			PROP_ENTRY_R(bundlePath),
			PROP_ENTRY_R(appSavePath),
			PROP_ENTRY_R(userSavePath),

			PROP_ENTRY_R(settings),
			PROP_ENTRY_R(fullSettings),

			PROP_ENTRY_R(runtime),
			PROP_ENTRY_R(log),

			PROP_ENTRY_R(booting),
			PROP_ENTRY_R(active),
			PROP_ENTRY_R(suspended),
			PROP_ENTRY_R(stopping),
			PROP_ENTRY_R(restarting),

			PROP_ENTRY_R(modules),
			PROP_ENTRY_R(schemaLookup),
			PROP_ENTRY_R(clock),
			PROP_ENTRY_R(timer),
			PROP_ENTRY_R(scheduler),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(getConfig,		"(keypath: string, default: string): string"),
			FUNC_ENTRY_H(stop,			"(exitCode=0) // terminates App"),
			FUNC_ENTRY_H(restart,		"()"),
			FUNC_ENTRY_H(getModule,		"(name: string): Module"),
			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getConfigs()->getName()); }
	NB_PROP_GET(title)					{ return push(v, self(v)->getConfigs()->getTitle()); }
	NB_PROP_GET(edition)				{ return push(v, self(v)->getConfigs()->getEdition()); }
	NB_PROP_GET(version)				{ return push(v, self(v)->getConfigs()->getVersion()); }

	NB_PROP_GET(platform)				{ return push(v, self(v)->getConfigs()->getPlatform()); }
	NB_PROP_GET(build)					{ return push(v, self(v)->getConfigs()->getBuild()); }
	NB_PROP_GET(deviceModel)			{ return push(v, self(v)->getConfigs()->getDeviceModel()); }
	NB_PROP_GET(deviceForm)				{ return push(v, self(v)->getConfigs()->getDeviceForm()); }
	NB_PROP_GET(osVersion)				{ return push(v, self(v)->getConfigs()->getOsVersion()); }

	NB_PROP_GET(pluginPath)				{ return push(v, self(v)->getConfigs()->getPluginPath()); }
	NB_PROP_GET(patchPath)				{ return push(v, self(v)->getConfigs()->getPatchPath()); }
	NB_PROP_GET(bundlePath)				{ return push(v, self(v)->getConfigs()->getBundlePath()); }
	NB_PROP_GET(appSavePath)			{ return push(v, self(v)->getConfigs()->getAppSavePath()); }
	NB_PROP_GET(userSavePath)			{ return push(v, self(v)->getConfigs()->getUserSavePath()); }

	NB_PROP_GET(settings)				{ return push(v, self(v)->getConfigs()->getSettings()); }
	NB_PROP_GET(fullSettings)			{ return push(v, self(v)->getConfigs()->getFullSettings()); }

	NB_PROP_GET(runtime)				{ return push(v, NitRuntime::getSingleton()); }
	NB_PROP_GET(log)					{ return push(v, &LogManager::getSingleton()); }

	NB_PROP_GET(booting)				{ return push(v, self(v)->isBooting()); }
	NB_PROP_GET(active)					{ return push(v, self(v)->isActive()); }
	NB_PROP_GET(suspended)				{ return push(v, self(v)->isSuspended()); }
	NB_PROP_GET(stopping)				{ return push(v, self(v)->isStopping()); }
	NB_PROP_GET(restarting)				{ return push(v, self(v)->isRestarting()); }

	NB_PROP_GET(modules)				{ return push(v, self(v)->getModules()); }
	NB_PROP_GET(schemaLookup)			{ return push(v, self(v)->getSchemaLookup()); }
	NB_PROP_GET(clock)					{ return push(v, self(v)->getClock()); }
	NB_PROP_GET(timer)					{ return push(v, self(v)->getTimer()); }
	NB_PROP_GET(scheduler)				{ return push(v, self(v)->getScheduler()); }

	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }
	NB_FUNC(stop)						{ self(v)->stop(optInt(v, 2, 0)); return 0; }
	NB_FUNC(restart)					{ self(v)->restart(); return 0; }
	NB_FUNC(getModule)					{ return push(v, self(v)->getModule(getString(v, 2))); }
	NB_FUNC(getConfig)					{ return push(v, self(v)->getConfig(getString(v, 2), getString(v, 3))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::PackageService, Service, incRefCount, decRefCount);

class NB_PackageService : TNitClass<PackageService>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(processing),
			PROP_ENTRY_R(bundle),
			PROP_ENTRY_R(asyncLoading),
			PROP_ENTRY_R(lookupDB),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(link,			"(name): Package"),
			FUNC_ENTRY_H(linkCustom,	"(name: string, archive: Archive, settings: Settings): Package"),
			FUNC_ENTRY_H(load,			"(name): Package"),
			FUNC_ENTRY_H(loadAsync,		"(name, hurry=true): Package"), // TODO: The convention is not consistent yet!
			FUNC_ENTRY_H(allLinked,		"() : { <name> = <Package>, ... }"),
			FUNC_ENTRY_H(all,			"(): Package[]"),
			FUNC_ENTRY_H(compact,		"()"),
			FUNC_ENTRY_H(channel,		"(): EventChannel"),
			FUNC_ENTRY_H(lookup,		"(type, key: string) : { subtype, pack, entry: string } // null when not found"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(processing)				{ return push(v, self(v)->getProcessing()); }
	NB_PROP_GET(bundle)					{ return push(v, self(v)->getBundle()); }
	NB_PROP_GET(asyncLoading)			{ return push(v, self(v)->isAsyncLoading()); }
	NB_PROP_GET(lookupDB)				{ return push(v, self(v)->getLookupDB()); }

	NB_FUNC(link)						{ return push(v, self(v)->link(getString(v, 2))); }
	NB_FUNC(linkCustom)					{ return push(v, self(v)->linkCustom(getString(v, 2), get<Archive>(v, 3), get<Settings>(v, 4))); }
	NB_FUNC(load)						{ return push(v, self(v)->load(getString(v, 2))); }
	NB_FUNC(loadAsync)					{ return push(v, self(v)->loadAsync(getString(v, 2), optBool(v, 3, false))); }

	NB_FUNC(lookup)
	{
		type::LookupResult r;

		bool found = self(v)->lookup(getString(v, 2), getString(v, 3), r);
		if (!found) return 0;

		sq_newtable(v);
		newSlot(v, -1, "subtype", r.subType);
		newSlot(v, -1, "pack", r.pack);
		newSlot(v, -1, "entry", r.entry);

		return 1;
	}

	NB_FUNC(allLinked)
	{
		const PackageService::PackageNameMap& map = self(v)->allLinked();

		sq_newtable(v);
		for (PackageService::PackageNameMap::const_iterator itr = map.begin(), end = map.end(); itr != end; ++itr)
		{
			newSlot(v, -1, itr->first, itr->second.get());
		}
		return 1;
	}

	NB_FUNC(all)
	{
		const PackageService::PackageNameMap& map = self(v)->allLinked();

		sq_newarray(v, 0);
		for (PackageService::PackageNameMap::const_iterator itr = map.begin(), end = map.end(); itr != end; ++itr)
		{
			arrayAppend(v, -1, itr->second.get());
		}
		return 1;
	}

	NB_FUNC(compact)					{ self(v)->compact(); return 0; }

	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::SessionService, Service, incRefCount, decRefCount);

class NB_SessionService : TNitClass<SessionService>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(current),
			PROP_ENTRY_R(next),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(current)				{ return push(v, self(v)->getCurrent()); }
	NB_PROP_GET(next)					{ return push(v, self(v)->getNext()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ContentsService, Service, incRefCount, decRefCount);

class NB_ContentsService : TNitClass<ContentsService>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(texture),
			NULL
		};

		FuncEntry funcs[] = 
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(texture)				{ return push(v, self(v)->getManager<Texture>()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::InputService, Service);

class NB_InputService : TNitClass<InputService>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(systemUser),
			PROP_ENTRY	(defaultUser),
			PROP_ENTRY_R(timer),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(commandChannel,"(): EventChannel"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(systemUser)				{ return push(v, self(v)->getSystemUser()); }
	NB_PROP_GET(defaultUser)			{ return push(v, self(v)->getDefaultUser()); }
	NB_PROP_GET(timer)					{ return push(v, self(v)->getTimer()); }

	NB_PROP_SET(defaultUser)			{ self(v)->setDefaultUser(opt<InputUser>(v, 2, NULL)); return 0; }

	NB_FUNC(commandChannel)				{ return push(v, self(v)->commandChannel()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputEvent, Event, incRefCount, decRefCount);

class NB_InputEvent : TNitClass<InputEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(user),
			PROP_ENTRY_R(device),
			PROP_ENTRY_R(source),
			PROP_ENTRY_R(command),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(user)					{ return push(v, self(v)->getUser()); }
	NB_PROP_GET(device)					{ return push(v, self(v)->getDevice()); }
	NB_PROP_GET(source)					{ return push(v, self(v)->getSource()); }
	NB_PROP_GET(command)				{ return push(v, self(v)->getCommand()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::InputUser, NULL);

class NB_InputUser : TNitClass<InputUser>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(index),
			PROP_ENTRY	(identity),
			PROP_ENTRY_R(locked),
			PROP_ENTRY_R(hasChannel),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(find,			"(pattern: string): InputDevice[]"),
			FUNC_ENTRY_H(acquire,		"(device: InputDevice)"),
			FUNC_ENTRY_H(release,		"(device: InputDevice)"),
			FUNC_ENTRY_H(lock,			"(reset=true, pattern=\"*\")"),
			FUNC_ENTRY_H(unlock,		"(pattern=\"*\")"),
			FUNC_ENTRY_H(channel,		"(): EventChannel"),

			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(index)					{ return push(v, self(v)->getIndex()); }
	NB_PROP_GET(identity)				{ return push(v, self(v)->getIdentity()); }
	NB_PROP_GET(locked)					{ return push(v, self(v)->isLocked()); }
	NB_PROP_GET(hasChannel)				{ return push(v, self(v)->hasChannel()); }

	NB_PROP_SET(identity)				{ self(v)->setIdentity(opt<DataObject>(v, 2, NULL)); return 0; }

	NB_FUNC(find)
	{
		vector<InputDevice*>::type results;
		self(v)->find(getString(v, 2), results);

		sq_newarray(v, 0);
		for (uint i=0; i<results.size(); ++i)
			arrayAppend(v, -1, results[i]);
		return 1;
	}

	NB_FUNC(acquire)					{ self(v)->acquire(get<InputDevice>(v, 2)); return 0; }
	NB_FUNC(release)					{ self(v)->release(get<InputDevice>(v, 2)); return 0; }
	NB_FUNC(lock)						{ self(v)->lock(optBool(v, 2, true), optString(v, 3, "*")); return 0; }
	NB_FUNC(unlock)						{ self(v)->unlock(optString(v, 2, "*")); return 0; }
	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		SQInteger top = sq_gettop(v);
		const char* clsname = "???";
		push(v, "_classname");
		sq_get(v, 1);
		sq_getstring(v, -1, &clsname);
		sq_settop(v, top);
		const String& name = o->getName();
		if (!name.empty())
			return pushFmt(v, "(%s: %s -> %08X)", name.c_str(), clsname, o);
		else
			return pushFmt(v, "(%s -> %08X)", clsname, o);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::InputDevice, NULL);

class NB_InputDevice : TNitClass<InputDevice>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(user),
			PROP_ENTRY_R(locked),
			PROP_ENTRY_R(hasChannel),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(name: string)"),
			FUNC_ENTRY_H(attach,		"(source: InputSource)"),
			FUNC_ENTRY_H(detach,		"(source: InputSource)"),
			FUNC_ENTRY_H(find,			"(pattern: string): InputSource[]"),
			FUNC_ENTRY_H(lock,			"(reset=true)"),
			FUNC_ENTRY_H(unlock,		"()"),
			FUNC_ENTRY_H(channel,		"(): EventChannel"),

			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(user)					{ return push(v, self(v)->getUser()); }
	NB_PROP_GET(locked)					{ return push(v, self(v)->isLocked()); }
	NB_PROP_GET(hasChannel)				{ return push(v, self(v)->hasChannel()); }
	
	NB_CONS()							{ setSelf(v, new InputDevice(getString(v, 2))); return SQ_OK; }

	NB_FUNC(attach)						{ self(v)->attach(get<InputSource>(v, 2)); return 0; }
	NB_FUNC(detach)						{ self(v)->detach(get<InputSource>(v, 2)); return 0; }

	NB_FUNC(find)
	{
		vector<InputSource*>::type results;
		self(v)->find(getString(v, 2), results);

		sq_newarray(v, 0);
		for (uint i=0; i<results.size(); ++i)
			arrayAppend(v, -1, results[i]);
		return 1;
	}

	NB_FUNC(lock)						{ self(v)->lock(optBool(v, 2, true)); return 0; }
	NB_FUNC(unlock)						{ self(v)->unlock(); return 0; }
	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		SQInteger top = sq_gettop(v);
		const char* clsname = "???";
		push(v, "_classname");
		sq_get(v, 1);
		sq_getstring(v, -1, &clsname);
		sq_settop(v, top);
		const String& name = o->getName();
		if (!name.empty())
			return pushFmt(v, "(%s: %s -> %08X)", name.c_str(), clsname, o);
		else
			return pushFmt(v, "(%s -> %08X)", clsname, o);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputSource, RefCounted, incRefCount, decRefCount);

class NB_InputSource : TNitClass<InputSource>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(device),
			PROP_ENTRY	(enabled),
			PROP_ENTRY	(lockable),
			PROP_ENTRY_R(locked),
			PROP_ENTRY_R(hasChannel),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(lock,			"(reset=true)"),
			FUNC_ENTRY_H(unlock,		"()"),
			FUNC_ENTRY_H(reset,			"()"),
			FUNC_ENTRY_H(update,		"(frame: int) // use frame from device or other source"),
			FUNC_ENTRY_H(channel,		"(): EventChannel"),

			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(device)					{ return push(v, self(v)->getDevice()); }
	NB_PROP_GET(enabled)				{ return push(v, self(v)->isEnabled()); }
	NB_PROP_GET(lockable)				{ return push(v, self(v)->isLockable()); }
	NB_PROP_GET(locked)					{ return push(v, self(v)->isLocked()); }
	NB_PROP_GET(hasChannel)				{ return push(v, self(v)->hasChannel()); }

	NB_PROP_SET(enabled)				{ self(v)->setEnabled(getBool(v, 2)); return 0; }
	NB_PROP_SET(lockable)				{ self(v)->setLockable(getBool(v, 2)); return 0; }

	NB_FUNC(lock)						{ self(v)->lock(optBool(v, 2, true)); return 0; }
	NB_FUNC(unlock)						{ self(v)->unlock(); return 0; }
	NB_FUNC(reset)						{ self(v)->reset(); return 0; }
	NB_FUNC(update)						{ self(v)->update(getInt(v, 2)); return 0; }
	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }

	NB_FUNC(_tostring)
	{
		// TODO: We can unify _tostring function implementation!
		type* o = self(v);
		SQInteger top = sq_gettop(v);
		const char* clsname = "???";
		push(v, "_classname");
		sq_get(v, 1);
		sq_getstring(v, -1, &clsname);
		sq_settop(v, top);
		const String& name = o->getName();
		if (!name.empty())
			return pushFmt(v, "(%s: %s -> %08X)", name.c_str(), clsname, o);
		else
			return pushFmt(v, "(%s -> %08X)", clsname, o);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputSourceEvent, InputEvent, incRefCount, decRefCount);

class NB_InputSourceEvent : TNitClass<InputSourceEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		bind(v, NULL, NULL);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputTrigger, InputSource, incRefCount, decRefCount);

class NB_InputTrigger : TNitClass<InputTrigger>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(pushed),
			PROP_ENTRY_R(repeatable),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(pushed)					{ return push(v, self(v)->isPushed()); }
	NB_PROP_GET(repeatable)				{ return push(v, self(v)->isRepeatable()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputKeypad, InputSource, incRefCount, decRefCount);

class NB_InputKeypad : TNitClass<InputKeypad>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputSlider, InputSource, incRefCount, decRefCount);

class NB_InputSlider : TNitClass<InputSlider>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(value),
			PROP_ENTRY_R(awake),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(value)					{ return push(v, self(v)->getValue()); }
	NB_PROP_GET(awake)					{ return push(v, self(v)->isAwake()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputStick, InputSource, incRefCount, decRefCount);

class NB_InputStick : TNitClass<InputStick>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(value),
			PROP_ENTRY_R(awake),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(value)					{ return push(v, self(v)->getValue()); }
	NB_PROP_GET(awake)					{ return push(v, self(v)->isAwake()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputPointer, InputSource, incRefCount, decRefCount);

class NB_InputPointer : TNitClass<InputPointer>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(value),
			PROP_ENTRY_R(on),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(value)					{ return push(v, self(v)->getValue()); }
	NB_PROP_GET(on)						{ return push(v, self(v)->isOn()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputPointerEvent, InputSourceEvent, incRefCount, decRefCount);

class NB_InputPointerEvent : TNitClass<InputPointerEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(prevPos),
			PROP_ENTRY_R(pos),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(prevPos)				{ return push(v, self(v)->getPrevPos()); }
	NB_PROP_GET(pos)					{ return push(v, self(v)->getPos()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputAccel, InputSource, incRefCount, decRefCount);

class NB_InputAccel : TNitClass<InputAccel>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputCommand, RefCounted, incRefCount, decRefCount);

class NB_InputCommand : TNitClass<InputCommand>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(command),
			PROP_ENTRY_R(param),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(cmd: string)\n"
										"(cmd: string, param: DataValue)"),

			FUNC_ENTRY	(_tostring),

			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(command)				{ return push(v, self(v)->getCommand()); }

	NB_PROP_GET(param)					{ return ScriptDataValue::push(v, self(v)->getParam()); }

	NB_CONS()
	{
		if (isNone(v, 3))
			setSelf(v, new InputCommand(getString(v, 2)));
		else
		{
			DataValue param;
			SQRESULT sr = ScriptDataValue::toValue(v, 3, param);
			if (SQ_FAILED(sr)) return sr;
			setSelf(v, new InputCommand(getString(v, 2), param));
		}
		return 0;
	}

	NB_FUNC(_tostring)
	{
		// TODO: We can unify _tostring function implementation!
		type* o = self(v);
		SQInteger top = sq_gettop(v);
		const char* clsname = "???";
		push(v, "_classname");
		sq_get(v, 1);
		sq_getstring(v, -1, &clsname);
		sq_settop(v, top);
		const String& cmd = o->getCommand();
		DataValue param = o->getParam();
		return pushFmt(v, "(%s(%s): %s -> %08X)", cmd.c_str(), param.c_str(), clsname, o);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputCommandEvent, InputEvent, incRefCount, decRefCount);

class NB_InputCommandEvent : TNitClass<InputCommandEvent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(user: InputUser, cmd: InputCommand)\n"
										"(device: InputDevice, cmd: InputCommand)\n"
										"(source: InputSource, cmd: InputCommand)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		if (is<InputUser>(v, 2))
			setSelf(v, new InputCommandEvent(get<InputUser>(v, 2), get<InputCommand>(v, 3)));
		else if (is<InputDevice>(v, 2))
			setSelf(v, new InputCommandEvent(get<InputDevice>(v, 2), get<InputCommand>(v, 3)));
		else
			setSelf(v, new InputCommandEvent(get<InputSource>(v, 2), get<InputCommand>(v, 3)));

		return SQ_OK;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::InputAutomata, InputSource, incRefCount, decRefCount);

class NB_InputAutomata : TNitClass<InputAutomata>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(initial),
			PROP_ENTRY_R(terminal),
			PROP_ENTRY_R(current),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(name: string)"),
			FUNC_ENTRY_H(newState,		"(enterAction=null, exitAction=null : InputCommand): InputAutomata.State"),
			FUNC_ENTRY_H(commandHandler,"(): EventHandler"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(initial)				{ return push(v, self(v)->getInitial()); }
	NB_PROP_GET(terminal)				{ return push(v, self(v)->getTerminal()); }
	NB_PROP_GET(current)				{ return push(v, self(v)->getCurrent()); }

	NB_CONS()							{ setSelf(v, new InputAutomata(getString(v, 2))); return SQ_OK; }

	NB_FUNC(newState)					{ return push(v, self(v)->newState(opt<InputCommand>(v, 2, NULL), opt<InputCommand>(v, 3, NULL))); }
	NB_FUNC(commandHandler)				{ return push(v, self(v)->commandHandler()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_WEAK(NIT_API, nit::InputAutomata::State, NULL);

class NB_InputAutomataState : TNitClass<InputAutomata::State>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY  (enter),
			PROP_ENTRY	(exit),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(dispatch,		"(cmd: InputCommand, next: InputAutomata.State) : InputAutomata.State"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(enter)					{ return push(v, self(v)->getEnter()); }
	NB_PROP_GET(exit)					{ return push(v, self(v)->getExit()); }

	NB_PROP_SET(enter)					{ self(v)->setEnter(get<InputCommand>(v, 2)); return 0; }
	NB_PROP_SET(exit)					{ self(v)->setExit(get<InputCommand>(v, 2)); return 0; }
	
	NB_FUNC(dispatch)					{ return push(v, self(v)->dispatch(get<InputCommand>(v, 2), get<InputAutomata::State>(v, 3))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_AUTODELETE(NIT_API, nit::ScopedInputLock, NULL, delete);

class NB_ScopedInputLock : TNitClass<ScopedInputLock>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"()"),
			FUNC_ENTRY_H(lock,			"(user: InputUser, reset=true, pattern=\"*\")\n"
										"(device: InputDevice, reset=true)\n"
										"(source: InputSource, reset=true)"),
			FUNC_ENTRY_H(unlock,		"(user: InputUser, pattern=\"*\")\n"
										"(device: InputDevice)\n"
										"(source: InputSource)"),
			FUNC_ENTRY_H(leave,			"(unlock=true)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()
	{
		setSelf(v, new ScopedInputLock());
		return SQ_OK;
	}

	NB_FUNC(lock)
	{
		if (is<InputUser>(v, 2))
			self(v)->lock(get<InputUser>(v, 2), optBool(v, 3, true), optString(v, 4, "*"));
		else if (is<InputDevice>(v, 2))
			self(v)->lock(get<InputDevice>(v, 2), optBool(v, 3, true));
		else if (is<InputSource>(v, 2))
			self(v)->lock(get<InputSource>(v, 2), optBool(v, 3, true));
		return 0;
	}

	NB_FUNC(unlock)
	{
		if (is<InputUser>(v, 2))
			self(v)->unlock(get<InputUser>(v, 2), optString(v, 3, "*"));
		else if (is<InputDevice>(v, 2))
			self(v)->unlock(get<InputDevice>(v, 2));
		else if (is<InputSource>(v, 2))
			self(v)->unlock(get<InputSource>(v, 2));
		return 0;
	}

	NB_FUNC(leave)
	{
		self(v)->leave(optBool(v, 2, true));
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Object, DataObject, incRefCount, decRefCount);

class NB_Object : TNitClass<Object>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(runtimeId),
			PROP_ENTRY	(active),
 			PROP_ENTRY_R(world),
			PROP_ENTRY	(name),
			PROP_ENTRY_R(attached),
			PROP_ENTRY_R(activated),
			PROP_ENTRY_R(hasError),
			PROP_ENTRY_R(statusFlag),
			PROP_ENTRY_R(disposed),
			
 			PROP_ENTRY	(transform),

			PROP_ENTRY	(position),
			PROP_ENTRY	(rotation),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY	(),

 			FUNC_ENTRY	(_get),
			FUNC_ENTRY	(_dump),

			FUNC_ENTRY_H(dispose,		"()"),

			FUNC_ENTRY_H(attach,		"(component): Component // returns param itself for simple coding"),
			FUNC_ENTRY_H(get,			"(name): Component"),
			FUNC_ENTRY_H(find,			"(pattern): Component[]"),

			FUNC_ENTRY_H(getFeature,	"(name): Feature"),
			FUNC_ENTRY_H(hasFeature,	"(feature: Feature): bool"),
			FUNC_ENTRY_H(findFeature,	"(pattern): Feature[]"),

			FUNC_ENTRY_H(install,		"(mod: Feature): bool"),
			FUNC_ENTRY_H(uninstall,		"(mod: Feature): bool"),

			FUNC_ENTRY_H(beginEdit,		"(): bool // true if new edit begin"),
			FUNC_ENTRY_H(endEdit,		"(): bool // true if all edits end"),

			FUNC_ENTRY_H(move,			"(dx, dy, dz)\n(delta: Vector3)"),
			FUNC_ENTRY_H(rotate,		"(delta: Quat)"),

			FUNC_ENTRY_H(info,			"(msg: string, prefix=\"\")"),
			FUNC_ENTRY_H(error,			"(msg: string, prefix=\"\")"),

			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IEventSink>(v);
	}

	NB_CONS()							{ sq_setinstanceup(v, 1, new Object()); return 0; }

	NB_PROP_GET(runtimeId)				{ return push(v, self(v)->getRuntimeId()); }
	NB_PROP_GET(active)					{ return push(v, self(v)->isActive()); }
	NB_PROP_GET(refCount)				{ return push(v, self(v)->getRefCount()); }
 	NB_PROP_GET(world)					{ return push(v, self(v)->getWorld()); }
	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(attached)				{ return push(v, self(v)->isAttached()); }
	NB_PROP_GET(activated)				{ return push(v, self(v)->isActivated()); }
	NB_PROP_GET(hasError)				{ return push(v, self(v)->hasError()); }
	NB_PROP_GET(statusFlag)				{ return push(v, (uint)self(v)->getStatusFlag()); }
	NB_PROP_GET(disposed)				{ return push(v, self(v)->isDisposed()); }

	NB_PROP_SET(active)					{ self(v)->setActive(getBool(v, 2)); return 0; }
	NB_PROP_SET(name)					{ self(v)->setName(getString(v, 2)); return 0; }

 	NB_PROP_GET(transform)				{return push(v, self(v)->getTransform()); }
 	NB_PROP_SET(transform)				{ self(v)->setTransform(opt<Transform>(v, 2, NULL)); return 0; }

	NB_PROP_GET(position)				{ return push(v, self(v)->getPosition()); }
	NB_PROP_SET(position)				{ self(v)->setPosition(*get<Vector3>(v, 2)); return 0; }

	NB_PROP_GET(rotation)				{ return push(v, self(v)->getRotation()); }
	NB_PROP_SET(rotation)				{ self(v)->setRotation(*get<Quat>(v, 2)); return 0; }

	NB_FUNC(dispose)					{ return push(v, self(v)->dispose()); }

	NB_FUNC(get)						{ return push(v, self(v)->get(getString(v, 2))); }
	NB_FUNC(getFeature)					{ return push(v, self(v)->getFeature(getString(v, 2))); }
	NB_FUNC(hasFeature)					{ return push(v, self(v)->hasFeature(get<Feature>(v, 2))); }

	NB_FUNC(_get)
	{
		Component* c = NULL;

		type* self = self_noThrow(v);
		if (self) c = self->get(getString(v, 2));
		if (c) return push(v, c);

		// not found - clean failure
		sq_reseterror(v); 
		return SQ_ERROR;
	}

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		const char* clsname = typeid(*o).name();
		if (clsname) clsname = &clsname[6]; // skip "class "
		return pushFmt(v, "(%s: %s -> %08X)", o->getName().c_str(), clsname, o);
	}

	NB_FUNC(_dump)
	{
		LOG(0, "  components:\n");

		vector<Component*>::type cl; self(v)->find("*", cl);

		for (uint i=0; i<cl.size(); ++i)
		{
			Component* c = cl[i];
			push(v, c);
			sq_tostring(v, -1);
			const char* compstr;
			sq_getstring(v, -1, &compstr);
			LOG(0, "  - %-18s = %s\n", c->getName().c_str(), compstr);
			sq_poptop(v);
		}

		return push(v, true);
	}

	NB_FUNC(attach)
	{
		if (isString(v, 2))
			return push(v, self(v)->attach(getString(v, 2), get<Component>(v, 3)));
		return push(v, self(v)->attach(get<Component>(v, 2)));
	}

	NB_FUNC(find) 
	{
		vector<Component*>::type cl; 
		self(v)->find(getString(v, 2), cl);

		sq_newarray(v, 0);
		for (uint i=0; i<cl.size(); ++i)
		{
			arrayAppend(v, -1, cl[i]);
		}
		return 1;
	}

	NB_FUNC(findFeature)
	{
		vector<Feature*>::type ml;
		self(v)->findFeature(getString(v, 2), ml);

		sq_newarray(v, 0);
		for (uint i=0; i<ml.size(); ++i)
		{
			arrayAppend(v, -1, ml[i]);
		}
		return 1;
	}

	NB_FUNC(install)					{ return push(v, self(v)->install(get<Feature>(v, 2))); }
	NB_FUNC(uninstall)					{ return push(v, self(v)->uninstall(get<Feature>(v, 2))); }

	NB_FUNC(beginEdit)					{ return push(v, self(v)->beginEdit()); }
	NB_FUNC(endEdit)					{ return push(v, self(v)->endEdit()); }

	NB_FUNC(move)
	{
		if (is<Vector3>(v, 2))
			self(v)->move(*get<Vector3>(v, 2));
		else
			self(v)->move(Vector3(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4)));
		return 0;
	}

	NB_FUNC(rotate)						{ self(v)->rotate(*get<Quat>(v, 2)); return 0; }

	NB_FUNC(info)						{ self(v)->info(getString(v, 2), optString(v, 3, "")); return 0; }
	NB_FUNC(error)						{ self(v)->error(getString(v, 2), optString(v, 3, "")); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Component, RefCounted, incRefCount, decRefCount);

class NitBindComponent : TNitClass<Component>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(active),
 			PROP_ENTRY_R(world),
			PROP_ENTRY_R(object),
			PROP_ENTRY	(name),
 			PROP_ENTRY_R(feature),
			PROP_ENTRY_R(attached),
			PROP_ENTRY_R(disposed),
			PROP_ENTRY_R(activated),
			PROP_ENTRY_R(hasError),
			PROP_ENTRY_R(statusFlag),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(checkValid,	"(): bool"),
			FUNC_ENTRY_H(dispose,		"(): bool // false if failed to dispose"),
			FUNC_ENTRY_H(info,			"(msg: string, prefix=\"\")"),
			FUNC_ENTRY_H(error,			"(msg: string, prefix=\"\")"),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IEventSink>(v);
	}

	NB_PROP_GET(active)					{ return push(v, self(v)->isActive()); }
	NB_PROP_GET(refCount)				{ return push(v, self(v)->getRefCount()); }
 	NB_PROP_GET(world)					{ return push(v, self(v)->getWorld()); }
	NB_PROP_GET(object)					{ return push(v, self(v)->getObject()); }
	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
 	NB_PROP_GET(feature)				{ return push(v, self(v)->getFeature()); }
	NB_PROP_GET(attached)				{ return push(v, self(v)->isAttached()); }
	NB_PROP_GET(disposed)				{ return push(v, self(v)->isDisposed()); }
	NB_PROP_GET(activated)				{ return push(v, self(v)->isActivated()); }
	NB_PROP_GET(hasError)				{ return push(v, self(v)->hasError()); }
	NB_PROP_GET(statusFlag)				{ return push(v, (uint)self(v)->getStatusFlag()); }

	NB_PROP_SET(active)					{ self(v)->setActive(getBool(v, 2)); return 0; }
	NB_PROP_SET(name)					{ self(v)->setName(getString(v, 2)); return 0; }

	NB_FUNC(checkValid)					{ return push(v, self(v)->checkValid()); }
	NB_FUNC(dispose)					{ return push(v, self(v)->dispose()); }
	NB_FUNC(info)						{ self(v)->info(getString(v, 2), optString(v, 3, "")); return 0; }
	NB_FUNC(error)						{ self(v)->error(getString(v, 2), optString(v, 3, "")); return 0; }

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		const char* clsname = typeid(*o).name();
		if (clsname) clsname = &clsname[6]; // skip "class "
		const char* objname = o->getObject() ? o->getObject()->getName().c_str() : "(null)";
		return pushFmt(v, "(%s.%s: %s -> %08X)", objname, o->getName().c_str(), clsname, o);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::World, RefCounted, incRefCount, decRefCount);

class NB_World : TNitClass<World>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY	(name),
			PROP_ENTRY_R(disposed),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(dispose,		"()"),

			FUNC_ENTRY_H(attach,		"(GameObject): GameObject // returns param itself for simple coding"),
			FUNC_ENTRY_H(all,			"(): GameObject[]"),
			FUNC_ENTRY_H(find,			"(wildcard, activeOnly=false, ignoreCase=true): GameObject[]"),
			FUNC_ENTRY_H(findComponents,"(wildcard, activeOnly=true, ignoreCase=true): GameObject[]"),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IEventSink>(v);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(disposed)				{ return push(v, self(v)->isDisposed()); }

	NB_PROP_SET(name)					{ self(v)->setName(getString(v, 2)); return 0; }

	NB_FUNC(dispose)					{ return push(v, self(v)->dispose()); }

	NB_FUNC(attach)
	{
		if (isString(v, 2))
			return push(v, self(v)->attach(getString(v, 2), get<Object>(v, 3)));
		return push(v, self(v)->attach(get<Object>(v, 2)));
	}

	NB_FUNC(all)
	{
		World* world = self(v);
		World::ObjectResultSet r;
		world->all(r);

		sq_newarray(v, 0);
		for (uint i = 0; i < r.size(); i++)
		{
			arrayAppend(v, -1, r[i]);
		}

		return 1;
	}

	NB_FUNC(find)
	{
		World* world = self(v);
		World::ObjectResultSet r;
		world->find(getString(v, 2), r, optBool(v, 3, false), optBool(v, 4, true));

		sq_newarray(v, 0);
		for (uint i = 0; i < r.size(); i++)
		{
			arrayAppend(v, -1, r[i]);
		}

		return 1;
	}

	NB_FUNC(findComponents)
	{
		World* world = self(v);
		World::ComponentResultSet r;
		world->findComponents(getString(v, 2), r, optBool(v, 3, true), optBool(v, 4, true));

		sq_newarray(v, 0);
		for (uint i = 0; i < r.size(); i++)
		{
			arrayAppend(v, -1, r[i]);
		}

		return 1;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ScriptObject, Object, IScriptRef::scriptIncRef, IScriptRef::scriptDecRef);

class NB_ScriptObject : TNitClass<ScriptObject>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY	(),
			FUNC_ENTRY	(_inherited),
			FUNC_ENTRY	(_tostring),
			FUNC_ENTRY	(_createDataSchema),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IScriptEventSink>(v);
		addInterface<IScriptRef>(v);
	}

	NB_FUNC(_inherited)					{ return SQ_OK; } // allow inheritance

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		SQInteger top = sq_gettop(v);
		const char* clsname = "???";
		push(v, "_classname");
		sq_get(v, 1);
		sq_getstring(v, -1, &clsname);
		sq_settop(v, top);
		return pushFmt(v, "(%s: %s -> %08X)", o->getName().c_str(), clsname, o);
	}

	NB_FUNC(_createDataSchema)
	{
		return push(v, (DataSchema*)new ScriptDataSchema(
			ScriptObject::adaptFunc, 
			ScriptObject::pushFunc,
			ScriptObject::getFunc,
			ScriptObject::setFunc));
	}

	NB_CONS()
	{
		ScriptObject* obj = new ScriptObject(new ScriptPeer(v, 1));

		ScriptRuntime::getRuntime(v)->retain(obj);

		sq_setinstanceup(v, 1, obj);

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ScriptComponent, Component, IScriptRef::scriptIncRef, IScriptRef::scriptDecRef);

class NB_ScriptComponent: TNitClass<ScriptComponent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY	(),
			FUNC_ENTRY	(_inherited),
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);

		addInterface<IScriptEventSink>(v);
		addInterface<IScriptRef>(v);
	}

	NB_FUNC(_inherited)					{ return SQ_OK; } // allow inheritance

	NB_CONS()
	{
		ScriptComponent* comp = new ScriptComponent(new ScriptPeer(v, 1));

		ScriptRuntime::getRuntime(v)->retain(comp);

		sq_setinstanceup(v, 1, comp);

		return 0;
	}

	NB_FUNC(_tostring)
	{
		type* o = self(v);
		SQInteger top = sq_gettop(v);
		const char* clsname = "???";
		push(v, "_classname");
		sq_get(v, 1);
		sq_getstring(v, -1, &clsname);
		sq_settop(v, top);
		const char* objname = o->getObject() ? o->getObject()->getName().c_str() : "(null)";
		return pushFmt(v, "(%s.%s: %s -> %08X)", objname, o->getName().c_str(), clsname, o);
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::PackBundle, StreamLocator, incRefCount, decRefCount);

class NB_PackBundle : TNitClass<PackBundle>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(base),
			PROP_ENTRY_R(uid),
			PROP_ENTRY_R(baseUid),
			PROP_ENTRY_R(compats),
			PROP_ENTRY_R(revision),
			PROP_ENTRY_R(active),
			PROP_ENTRY_R(valid),
			PROP_ENTRY_R(realFile),
			PROP_ENTRY_R(pack),
			PROP_ENTRY_R(locator),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(bundleRoot: FileLocator, uid: string)\n"
										"(bundleFile: StreamSource)"),
			FUNC_ENTRY_H(isCompatible,	"(version: string)"),
			FUNC_ENTRY_H(isRecentThan,	"(bundle: PackBundle): bool"),
			FUNC_ENTRY_H(link,			"(packName: string)"),
			FUNC_ENTRY_H(unpackZBundle,	"[class] (r: StreamReader, w: StreamWriter): PackBundle.ZBundleInfo"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(base)					{ return push(v, self(v)->getBase()); }
	NB_PROP_GET(uid)					{ return push(v, self(v)->getUid()); }
	NB_PROP_GET(baseUid)				{ return push(v, self(v)->getBaseUid()); }
	NB_PROP_GET(revision)				{ return push(v, self(v)->getRevision()); }
	NB_PROP_GET(active)					{ return push(v, self(v)->isActive()); }
	NB_PROP_GET(valid)					{ return push(v, self(v)->isValid()); }
	NB_PROP_GET(realFile)				{ return push(v, self(v)->getRealFile()); }
	NB_PROP_GET(pack)					{ return push(v, self(v)->getPack()); }
	NB_PROP_GET(locator)				{ return push(v, self(v)->getLocator()); }

	NB_PROP_GET(compats)
	{
		const StringVector& compats = self(v)->getCompats();
		sq_newarray(v, 0);
		for (uint i=0; i < compats.size(); ++i)
		{
			arrayAppend(v, -1, compats[i]);
		}
		return 1;
	}

	NB_CONS()
	{
		if (is<FileLocator>(v, 2))
			setSelf(v, new PackBundle(get<FileLocator>(v, 2), getString(v, 3)));
		else
			setSelf(v, new PackBundle(get<StreamSource>(v, 2)));
		return 0;
	}

	NB_FUNC(isCompatible)				{ return push(v, self(v)->isCompatible(getString(v, 2))); }
	NB_FUNC(isRecentThan)				{ return push(v, self(v)->isRecentThan(get<PackBundle>(v, 2))); }
	NB_FUNC(link)						{ return push(v, self(v)->link(getString(v, 2))); }
	NB_FUNC(unpackZBundle)				{ return push(v, PackBundle::unpackZBundle(get<StreamReader>(v, 2), get<StreamWriter>(v, 3)).get()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::PackBundle::ZBundleInfo, RefCounted, incRefCount, decRefCount);

class NB_PackBundleZBundleInfo : TNitClass<PackBundle::ZBundleInfo>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(flags),
			PROP_ENTRY_R(uid),
			PROP_ENTRY_R(baseUid),
			PROP_ENTRY_R(bundleCfg),
			PROP_ENTRY_R(extData),
			PROP_ENTRY_R(sourceSize),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(flags)					{ return push(v, self(v)->getHeader().flags); }
	NB_PROP_GET(uid)					{ return push(v, self(v)->getUid()); }
	NB_PROP_GET(baseUid)				{ return push(v, self(v)->getBaseUid()); }
	NB_PROP_GET(bundleCfg)				{ return push(v, self(v)->getBundleCfg()); }
	NB_PROP_GET(extData)				{ return push(v, self(v)->getExtData()); }
	NB_PROP_GET(sourceSize)				{ return push(v, (uint32)self(v)->getHeader().sourceSize); } // TODO: apply uint64
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::PackArchive, Archive, incRefCount, decRefCount);

class NB_PackArchive : TNitClass<PackArchive>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(realFile),
			PROP_ENTRY_R(realFileOffset),
			PROP_ENTRY_R(endianFlip),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(name: string, packFile: StreamSource)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(realFile)				{ return push(v, self(v)->getRealFile()); }
	NB_PROP_GET(realFileOffset)			{ return push(v, size_t(self(v)->getRealFileOffset())); } // TODO: apply uint64
	NB_PROP_GET(endianFlip)				{ return push(v, self(v)->isEndianFlip()); }

	NB_CONS()							{ setSelf(v, new PackArchive(getString(v, 2), get<StreamSource>(v, 3))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::PackArchive::File, StreamSource, incRefCount, decRefCount);

class NB_PackArchiveFile : TNitClass<PackArchive::File>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(offset),
			PROP_ENTRY_R(payloadType),
			PROP_ENTRY_R(payloadCrc32),
			PROP_ENTRY_R(payloadParam0),
			PROP_ENTRY_R(payloadParam1),
			PROP_ENTRY_R(sourceSize),
			PROP_ENTRY_R(sourceCrc32),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(offset)					{ return push(v, size_t(self(v)->getOffset())); } // TODO: apply uint64
	NB_PROP_GET(payloadType)			{ return push(v, self(v)->getPayloadType()); }
	NB_PROP_GET(payloadCrc32)			{ return push(v, self(v)->getPayloadCRC32()); }
	NB_PROP_GET(payloadParam0)			{ return push(v, self(v)->getPayloadParam0()); }
	NB_PROP_GET(payloadParam1)			{ return push(v, self(v)->getPayloadParam1()); }
	NB_PROP_GET(sourceSize)				{ return push(v, self(v)->getSourceSize()); }
	NB_PROP_GET(sourceCrc32)			{ return push(v, self(v)->getSourceCRC32()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Package, Archive, incRefCount, decRefCount);

class NB_Package : TNitClass<Package>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(linked),
			PROP_ENTRY_R(loading),
			PROP_ENTRY_R(loaded),
			PROP_ENTRY_R(archive),
			PROP_ENTRY_R(bundle),
			PROP_ENTRY_R(settingsSource),

			PROP_ENTRY	(stayResident),
			PROP_ENTRY	(stayForCurrent),
			PROP_ENTRY	(stayForNext),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(loadAsync,		"(hurry=false)"),

			FUNC_ENTRY_H(channel,		"(): EventChannel"),

			FUNC_ENTRY_H(link,			"(streamName: string, throwEx=true): Content\n"
										"(streamName: string, locatorPattern: string, throwEx=true): Content"),
			FUNC_ENTRY_H(linkCustom,	"(streamName: string, content: Content, otherSource: StreamSource=NULL, throwEx=true): ContentSource"),

			FUNC_ENTRY_H(openDb,		"(streamName: string): Database"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(linked)					{ return push(v, self(v)->isLinked()); }
	NB_PROP_GET(loading)				{ return push(v, self(v)->isLoading()); }
	NB_PROP_GET(loaded)					{ return push(v, self(v)->isLoaded()); }
	NB_PROP_GET(archive)				{ return push(v, self(v)->getArchive()); }
	NB_PROP_GET(bundle)					{ return push(v, self(v)->getBundle()); }
	NB_PROP_GET(settingsSource)			{ return push(v, self(v)->getSettingsSource()); }
	NB_PROP_GET(stayResident)			{ return push(v, self(v)->isStayResident()); }
	NB_PROP_GET(stayForCurrent)			{ return push(v, self(v)->isStayForCurrent()); }
	NB_PROP_GET(stayForNext)			{ return push(v, self(v)->isStayForNext()); }

	NB_PROP_SET(stayResident)			{ self(v)->setStayResident(getBool(v, 2)); return 0; }
	NB_PROP_SET(stayForCurrent)			{ self(v)->setStayForCurrent(getBool(v, 2)); return 0; }
	NB_PROP_SET(stayForNext)			{ self(v)->setStayForNext(getBool(v, 2)); return 0; }

	NB_FUNC(loadAsync)					{ self(v)->loadAsync(optBool(v, 2, false)); return 0; }
	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }

	NB_FUNC(link)						
	{ 
		if (!isNone(v, 3) && isString(v, 3))
			return push(v, self(v)->link(getString(v, 2), getString(v, 3), optBool(v, 4, true)).get());
		else
			return push(v, self(v)->link(getString(v, 2), optBool(v, 3, true)).get()); 
	}

	NB_FUNC(openDb)						{ return push(v, self(v)->openDb(getString(v, 2))); }

	NB_FUNC(linkCustom)					{ return push(v, self(v)->linkCustom(getString(v, 2), get<Content>(v, 3), opt<StreamSource>(v, 4, NULL), optBool(v, 5, true))); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Session, RefCounted, incRefCount, decRefCount);

class NB_Session : TNitClass<Session>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] = 
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(argument),
			PROP_ENTRY_R(ready),

			PROP_ENTRY_R(timer),
			PROP_ENTRY_R(scheduler),

			PROP_ENTRY_R(tickTime),
			PROP_ENTRY_R(baseTime),

			PROP_ENTRY_R(runtime),
			PROP_ENTRY_R(package),
			PROP_ENTRY_R(world),
			PROP_ENTRY_R(localFeatures),
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(channel,		"(): EventChannel"),

			FUNC_ENTRY_H(restart,		"[class] (quick=true)"),
			FUNC_ENTRY_H(start,			"[class] (name, argument=\"\")"),
			FUNC_ENTRY_H(enter,			"[class] (name, argument=\"\")"),
			FUNC_ENTRY_H(leave,			"[class] ()"),

			FUNC_ENTRY_H(register,		"(mod: Feature)"),
			FUNC_ENTRY_H(unregister,	"(mod: Feature)"),
			FUNC_ENTRY_H(getFeature,	"(name: string): Feature"),

			FUNC_ENTRY_H(require,		"(packageName: string): Package"),
			NULL,
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(argument)				{ return push(v, self(v)->getArgument()); }
	NB_PROP_GET(ready)					{ return push(v, self(v)->isReady()); }

	NB_PROP_GET(timer)					{ return push(v, self(v)->getTimer()); }
	NB_PROP_GET(scheduler)				{ return push(v, self(v)->getScheduler()); }

	NB_PROP_GET(tickTime)				{ return push(v, self(v)->getTickTime()); }
	NB_PROP_GET(baseTime)				{ return push(v, self(v)->getBaseTime()); }

	NB_PROP_GET(runtime)				{ return push(v, self(v)->getRuntime()); }
	NB_PROP_GET(package)				{ return push(v, self(v)->getPackage()); }
	NB_PROP_GET(world)					{ return push(v, self(v)->getWorld()); }

	NB_PROP_GET(localFeatures)			{ return push(v, self(v)->getLocalFeatures()); }

	NB_FUNC(channel)					{ return push(v, self(v)->channel()); }

	NB_FUNC(restart)					{ g_SessionService->restart(optBool(v, 2, true)); return 0; }
	NB_FUNC(start)						{ g_SessionService->start(createSession(v, getString(v, 2), optString(v, 3, ""))); return 0; }
	NB_FUNC(enter)						{ g_SessionService->enter(createSession(v, getString(v, 2), optString(v, 3, ""))); return 0; }
	NB_FUNC(leave)						{ g_SessionService->leave(); return 0; }

	NB_FUNC(register)					{ self(v)->Register(get<Feature>(v, 2)); return 0; }
	NB_FUNC(unregister)					{ self(v)->Unregister(get<Feature>(v, 2)); return 0; }
	NB_FUNC(getFeature)					{ return push(v, self(v)->getFeature(getString(v, 2))); }

	NB_FUNC(require)					{ return push(v, self(v)->require(getString(v, 2))); return 0; }

	static Session* createSession(HSQUIRRELVM v, const char* name, const char* arg)
	{
		Session* session = g_SessionService->create(name, arg);

		if (session == NULL)
		{
			sqx_throwfmt(v, "unknown session '%s'", name);
		}

		return session;
	}
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ContentSource, StreamSource, incRefCount, decRefCount);

class NB_ContentSource : TNitClass<ContentSource>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(package),
			PROP_ENTRY_R(streamSource),
			PROP_ENTRY_R(content),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(package)				{ return push(v, self(v)->getPackage()); }
	NB_PROP_GET(streamSource)			{ return push(v, self(v)->getStreamSource()); }
	NB_PROP_GET(content)				{ return push(v, self(v)->getContent()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Content, RefCounted, incRefCount, decRefCount);

class NB_Content : TNitClass<Content>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(manager),
			PROP_ENTRY_R(source),
			PROP_ENTRY_R(sourceUrl),
			PROP_ENTRY_R(timestamp),
			PROP_ENTRY_R(memorySize),

			PROP_ENTRY_R(hasProxy),
			PROP_ENTRY	(proxy),

			PROP_ENTRY_R(hasCacheEntry),
			PROP_ENTRY_R(cached),
			
			PROP_ENTRY_R(linked),
			PROP_ENTRY_R(loading),
			PROP_ENTRY_R(loaded),
			PROP_ENTRY_R(disposed),
			PROP_ENTRY_R(hasError),
			NULL
		};

		FuncEntry funcs[] = 
		{
			FUNC_ENTRY_H(load,			"(activate=true)"),
			FUNC_ENTRY_H(unload,		"()"),
			FUNC_ENTRY_H(dispose,		"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(manager)				{ return push(v, self(v)->getManager()); }
	NB_PROP_GET(source)					{ return push(v, self(v)->getSource()); }
	NB_PROP_GET(sourceUrl)				{ return push(v, self(v)->getSourceUrl()); }
	NB_PROP_GET(timestamp)				{ return push(v, self(v)->getTimestamp()); }
	NB_PROP_GET(memorySize)				{ return push(v, self(v)->getMemorySize()); }

	NB_PROP_GET(hasProxy)				{ return push(v, self(v)->hasProxy()); }
	NB_PROP_GET(proxy)					{ return push(v, self(v)->getProxy()); }

	NB_PROP_GET(hasCacheEntry)			{ return push(v, self(v)->hasCacheEntry()); }
	NB_PROP_GET(cached)					{ return push(v, self(v)->IsCached()); }

	NB_PROP_GET(linked)					{ return push(v, self(v)->isLinked()); }
	NB_PROP_GET(loading)				{ return push(v, self(v)->isLoading()); }
	NB_PROP_GET(loaded)					{ return push(v, self(v)->isLoaded()); }
	NB_PROP_GET(disposed)				{ return push(v, self(v)->isDisposed()); }
	NB_PROP_GET(hasError)				{ return push(v, self(v)->hasError()); }

	NB_PROP_SET(proxy)					{ self(v)->setProxy(opt<Content>(v, 2, NULL)); return 0; }

	NB_FUNC(load)						{ self(v)->load(); return 0; }
	NB_FUNC(unload)						{ self(v)->unload(); return 0; }
	NB_FUNC(dispose)					{ self(v)->dispose(); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::ContentManager, Module, incRefCount, decRefCount);

class NB_ContentManager : TNitClass<ContentManager>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			FUNC_ENTRY_H(createFrom,	"(source: StreamSource=NULL): Content"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_FUNC(createFrom)					{ return push(v, self(v)->createFrom(opt<StreamSource>(v, 2, NULL)).get()); }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_VALUE(NIT_API, nit::PixelFormat, NULL);

class NB_PixelFormat : TNitClass<PixelFormat>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(name),
			PROP_ENTRY_R(value),
			PROP_ENTRY_R(id),
			PROP_ENTRY_R(hasAlpha),
			PROP_ENTRY_R(alphaPremultiplied),
			PROP_ENTRY_R(compressed),
			PROP_ENTRY_R(grayscale),
			PROP_ENTRY_R(orderedAsRgba),
			PROP_ENTRY_R(tiled),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H(				"(name: string)"),
			FUNC_ENTRY	(_eq), // TODO: This meta method not yet exists
			FUNC_ENTRY	(_tostring),
			NULL
		};

		bind(v, props, funcs);

		vector<std::pair<String, PixelFormat> >::type fmts;
		PixelFormat::allFormats(fmts);

		for (uint i=0; i<fmts.size(); ++i)
		{
			addStatic(v, fmts[i].first.c_str(), fmts[i].second);
		}
	};

	NB_PROP_GET(name)					{ return push(v, self(v)->getName()); }
	NB_PROP_GET(value)					{ return push(v, self(v)->getValue()); }
	NB_PROP_GET(id)						{ return push(v, self(v)->getId()); }
	NB_PROP_GET(hasAlpha)				{ return push(v, self(v)->hasAlpha()); }
	NB_PROP_GET(alphaPremultiplied)		{ return push(v, self(v)->isAlphaPremultiplied()); }
	NB_PROP_GET(compressed)				{ return push(v, self(v)->isCompressed()); }
	NB_PROP_GET(grayscale)				{ return push(v, self(v)->isGrayscale()); }
	NB_PROP_GET(orderedAsRgba)			{ return push(v, self(v)->isOrderedAsRgba()); }
	NB_PROP_GET(tiled)					{ return push(v, self(v)->isTiled()); }

	NB_CONS()							{ new (self(v)) PixelFormat(getString(v, 2)); return 0; }

	NB_FUNC(_eq)						{ return push(v, self(v)->getValue() == get<PixelFormat>(v, 2)->getValue()); }
	NB_FUNC(_tostring)					{ sqx_pushstringf(v, "(PixelFormat.%s: 0x%04X)", self(v)->getName().c_str(), (int)self(v)->getId()); return 1; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Image, Content, incRefCount, decRefCount);

class NB_Image : TNitClass<Image>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(contentType),
			PROP_ENTRY_R(pixelFormat),
			PROP_ENTRY_R(bitsPerPixel),
			PROP_ENTRY_R(width),
			PROP_ENTRY_R(height),
			PROP_ENTRY_R(pitch),
			PROP_ENTRY_R(byteCount),

			PROP_ENTRY_R(flags),
			PROP_ENTRY_R(sourceWidth),
			PROP_ENTRY_R(sourceHeight),
			PROP_ENTRY_R(contentLeft),
			PROP_ENTRY_R(contentTop),
			PROP_ENTRY_R(contentRight),
			PROP_ENTRY_R(contentBottom),

			PROP_ENTRY_R(mipCount),
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"(source: StreamSource, treatAs=ContentType.UNKNOWN)"),
			FUNC_ENTRY_H(getMipWidth,	"(mipLevel: int): int"),
			FUNC_ENTRY_H(getMipHeight,	"(mipLevel: int): int"),
			FUNC_ENTRY_H(getMipPitch,	"(mipLevel: int): int"),
			FUNC_ENTRY_H(getMipByteCount, "(mipLevel: int): int"),

			FUNC_ENTRY_H(discardMipmaps,	"()"),
			FUNC_ENTRY_H(makePot,		"(square: bool, int min=4)"),
			FUNC_ENTRY_H(reorderAsRgba,	"()"),
			FUNC_ENTRY_H(makeRgba_4444,	"()"),

			FUNC_ENTRY_H(saveNtex,		"(writer: StreamWriter, flipEndian=false)"),
			FUNC_ENTRY_H(savePng,		"(writer: StreamWriter)"),
			FUNC_ENTRY_H(saveJpeg,		"(writer: StreamWriter)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(contentType)			{ return push(v, self(v)->getContentType()); }
	NB_PROP_GET(pixelFormat)			{ return push(v, self(v)->getPixelFormat()); }
	NB_PROP_GET(bitsPerPixel)			{ return push(v, self(v)->getBitsPerPixel()); }
	NB_PROP_GET(width)					{ return push(v, self(v)->getWidth()); }
	NB_PROP_GET(height)					{ return push(v, self(v)->getHeight()); }
	NB_PROP_GET(pitch)					{ return push(v, self(v)->getPitch()); }
	NB_PROP_GET(byteCount)				{ return push(v, self(v)->getByteCount()); }

	NB_PROP_GET(flags)					{ return push(v, self(v)->getFlags()); }
	NB_PROP_GET(sourceWidth)			{ return push(v, self(v)->getSourceWidth()); }
	NB_PROP_GET(sourceHeight)			{ return push(v, self(v)->getSourceHeight()); }

	NB_PROP_GET(contentLeft)			{ return push(v, self(v)->getContentLeft()); }
	NB_PROP_GET(contentTop)				{ return push(v, self(v)->getContentTop()); }
	NB_PROP_GET(contentRight)			{ return push(v, self(v)->getContentRight()); }
	NB_PROP_GET(contentBottom)			{ return push(v, self(v)->getContentBottom()); }

	NB_PROP_GET(mipCount)				{ return push(v, self(v)->getMipCount()); }

	NB_CONS()							{ setSelf(v, new Image(get<StreamSource>(v, 2), *opt<ContentType>(v, 3, ContentType::UNKNOWN))); return 0; }

	NB_FUNC(getMipWidth)				{ return push(v, self(v)->getMipWidth(getInt(v, 2))); }
	NB_FUNC(getMipHeight)				{ return push(v, self(v)->getMipHeight(getInt(v, 2))); }
	NB_FUNC(getMipPitch)				{ return push(v, self(v)->getMipPitch(getInt(v, 2))); }
	NB_FUNC(getMipByteCount)			{ return push(v, self(v)->getMipByteCount(getInt(v, 2))); }

	NB_FUNC(discardMipmaps)				{ self(v)->discardMipmaps(); return 0; }
	NB_FUNC(makeAlphaPremultiplied)		{ self(v)->makeAlphaPremultiplied(); return 0; }
	NB_FUNC(makePot)					{ self(v)->makePot(getBool(v, 2), optInt(v, 3, 4)); return 0; }
	NB_FUNC(reorderAsRgba)				{ self(v)->reorderAsRgba(); return 0; }
	NB_FUNC(makeRgba_4444)				{ self(v)->makeRgba_4444(); return 0; }

	NB_FUNC(saveNtex)					{ self(v)->SaveNtex(get<StreamWriter>(v, 2), optBool(v, 3, false)); return 0; }
	NB_FUNC(savePng)					{ self(v)->SavePng(get<StreamWriter>(v, 2)); return 0; }
	NB_FUNC(saveJpeg)					{ self(v)->SaveJpeg(get<StreamWriter>(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Texture, Content, incRefCount, decRefCount);

class NB_Texture : TNitClass<Texture>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(sourceImage),
			PROP_ENTRY_R(pixelFormat),
			PROP_ENTRY	(minTexCoord),
			PROP_ENTRY	(maxTexCoord),
			PROP_ENTRY	(minFilter),
			PROP_ENTRY	(magFilter),
			PROP_ENTRY	(wrapModeS),
			PROP_ENTRY	(wrapModeT),
			PROP_ENTRY	(autoGenMipmap),
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);

		addStaticTable(v, "MIN_FILTER");
		newSlot(v, -1, "NEAREST",					(int)Texture::MIN_NEAREST);
		newSlot(v, -1, "LINEAR",					(int)Texture::MIN_LINEAR);
		newSlot(v, -1, "NEAREST_MIPMAP_NEAREST",	(int)Texture::MIN_NEAREST_MIPMAP_NEAREST);
		newSlot(v, -1, "LINEAR_MIPMAP_NEAREST",		(int)Texture::MIN_LINEAR_MIPMAP_NEAREST);
		newSlot(v, -1, "NEAREST_MIPMAP_LINEAR",		(int)Texture::MIN_NEAREST_MIPMAP_LINEAR);
		newSlot(v, -1, "LINEAR_MIPMAP_LINEAR",		(int)Texture::MIN_LINEAR_MIPMAP_LINEAR);
		sq_poptop(v);

		addStaticTable(v, "MAG_FILTER");
		newSlot(v, -1, "NEAREST",					(int)Texture::MAG_NEAREST);
		newSlot(v, -1, "LINEAR",					(int)Texture::MAG_LINEAR);
		sq_poptop(v);

		addStaticTable(v, "WRAP_MODE");
		newSlot(v, -1, "CLAMP_TO_EDGE",				(int)Texture::WRAP_CLAMP_TO_EDGE);
		newSlot(v, -1, "REPEAT",					(int)Texture::WRAP_REPEAT);
		sq_poptop(v);
	};

	NB_PROP_GET(sourceImage)			{ return push(v, self(v)->getSourceImage()); }
	NB_PROP_GET(pixelFormat)			{ return push(v, self(v)->getPixelFormat()); }
	NB_PROP_GET(minTexCoord)			{ return push(v, self(v)->getMinTexCoord()); }
	NB_PROP_GET(maxTexCoord)			{ return push(v, self(v)->getMaxTexCoord()); }
	NB_PROP_GET(minFilter)				{ return push(v, (int)self(v)->getMinFilter()); }
	NB_PROP_GET(magFilter)				{ return push(v, (int)self(v)->getMagFilter()); }
	NB_PROP_GET(wrapModeS)				{ return push(v, (int)self(v)->getWrapModeS()); }
	NB_PROP_GET(wrapModeT)				{ return push(v, (int)self(v)->getWrapModeT()); }
	NB_PROP_GET(autoGenMipmap)			{ return push(v, self(v)->getAutoGenMipmap()); }

	NB_PROP_SET(minTexCoord)			{ self(v)->setMinTexCoord(*get<Vector2>(v, 2)); return 0; }
	NB_PROP_SET(maxTexCoord)			{ self(v)->setMaxTexCoord(*get<Vector2>(v, 2)); return 0; }
	NB_PROP_SET(minFilter)				{ self(v)->setMinFilter(Texture::MinFilter(getInt(v, 2))); return 0; }
	NB_PROP_SET(magFilter)				{ self(v)->setMagFilter(Texture::MagFilter(getInt(v, 2))); return 0; }
	NB_PROP_SET(wrapModeS)				{ self(v)->setWrapModeS(Texture::WrapMode(getInt(v, 2))); return 0; }
	NB_PROP_SET(wrapModeT)				{ self(v)->setWrapModeT(Texture::WrapMode(getInt(v, 2))); return 0; }
	NB_PROP_SET(autoGenMipmap)			{ self(v)->setAutoGenMipmap(getBool(v, 2)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::TextureManager, ContentManager, incRefCount, decRefCount);

class NB_TextureManager : TNitClass<TextureManager>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] =
		{
			NULL
		};

		bind(v, props, funcs);
	};
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::Transform, Component, incRefCount, decRefCount);

class NB_Transform : TNitClass<Transform>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY_R(updating),
			PROP_ENTRY	(position),
			PROP_ENTRY	(rotation),
			PROP_ENTRY_R(front),
			PROP_ENTRY_R(right),
			PROP_ENTRY_R(up),
			PROP_ENTRY	(posOffset),
			PROP_ENTRY	(rotOffset),
			PROP_ENTRY_R(invRotOffset),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H("()"),

			FUNC_ENTRY_H(beginTransform,	"()"),
			FUNC_ENTRY_H(endTransform,		"()"),

			FUNC_ENTRY_H(move,				"(dx, dy, dz)\n(delta: Vector3)"),
			FUNC_ENTRY_H(rotate,			"(delta: Quat)"),

			FUNC_ENTRY_H(alignUpFront,		"(up, front: Vector3)"),
			FUNC_ENTRY_H(alignUpRight,		"(up, right: Vector3)"),
			FUNC_ENTRY_H(alignFrontUp,		"(front, up: Vector3)"),
			FUNC_ENTRY_H(alignFrontRight,	"(front, right: Vector3)"),
			FUNC_ENTRY_H(alignRightUp,		"(right, up: Vector3)"),
			FUNC_ENTRY_H(alignRightFront,	"(right, front: Vector3)"),

			FUNC_ENTRY_H(toWorldPos,		"(localPos: Vector3): Vector3"),
			FUNC_ENTRY_H(toLocalPos,		"(worldPos: Vector3): Vector3"),
			FUNC_ENTRY_H(toWorldDir,		"(localDir: Vector3): Vector3"),
			FUNC_ENTRY_H(toLocalDir,		"(worldDir: Vector3): Vector3"),

			FUNC_ENTRY_H(clearOffset,		"()"),

			FUNC_ENTRY_H(joinSyncGroup,		"(groupMember: WorldTransform=null) // if member == null, leaves sync group"),
			FUNC_ENTRY_H(getSyncGroup,		"(): WorldTransform[]"),

			FUNC_ENTRY_H(applyOffset,		"(var pos: Vector3, var rot: Quat)"),
			FUNC_ENTRY_H(applyInvOffset,	"(var pos: Vector3, var rot: Quat)"),
			NULL
		};

		bind(v, props, funcs);		
	}

	NB_PROP_GET(updating)				{ return push(v, self(v)->isUpdating()); }
	NB_PROP_GET(position)				{ return push(v, self(v)->getPosition()); }
	NB_PROP_GET(rotation)				{ return push(v, self(v)->getRotation()); }
	NB_PROP_GET(front)					{ return push(v, self(v)->getFront()); }
	NB_PROP_GET(right)					{ return push(v, self(v)->getRight()); }
	NB_PROP_GET(up)						{ return push(v, self(v)->getUp()); }
	NB_PROP_GET(posOffset)				{ return push(v, self(v)->getPosOffset()); }
	NB_PROP_GET(rotOffset)				{ return push(v, self(v)->getRotOffset()); }
	NB_PROP_GET(invRotOffset)			{ return push(v, self(v)->getInvRotOffset()); }

	NB_PROP_SET(position)				{ self(v)->setPosition(*get<Vector3>(v, 2)); return 0; }
	NB_PROP_SET(rotation)				{ self(v)->setRotation(*get<Quat>(v, 2)); return 0; }
	NB_PROP_SET(posOffset)				{ self(v)->setPosOffset(*get<Vector3>(v, 2)); return 0; }
	NB_PROP_SET(rotOffset)				{ self(v)->setRotOffset(*get<Quat>(v, 2)); return 0; }

	NB_CONS()							{ sq_setinstanceup(v, 1, new Transform()); return 0; }

	NB_FUNC(beginTransform)				{ self(v)->beginTransform(); return 0; }
	NB_FUNC(endTransform)				{ self(v)->endTransform(); return 0; }

	NB_FUNC(move)
	{
		if (is<Vector3>(v, 2))
			self(v)->move(*get<Vector3>(v, 2));
		else
			self(v)->move(Vector3(getFloat(v, 2), getFloat(v, 3), getFloat(v, 4)));
		return 0;
	}

	NB_FUNC(rotate)						{ self(v)->rotate(*get<Quat>(v, 2)); return 0; }

	NB_FUNC(alignUpFront)				{ self(v)->alignUpFront(*get<Vector3>(v, 2), *get<Vector3>(v, 3)); return 0; }
	NB_FUNC(alignUpRight)				{ self(v)->alignUpRight(*get<Vector3>(v, 2), *get<Vector3>(v, 3)); return 0; }
	NB_FUNC(alignFrontUp)				{ self(v)->alignFrontUp(*get<Vector3>(v, 2), *get<Vector3>(v, 3)); return 0; }
	NB_FUNC(alignFrontRight)			{ self(v)->alignFrontRight(*get<Vector3>(v, 2), *get<Vector3>(v, 3)); return 0; }
	NB_FUNC(alignRightUp)				{ self(v)->alignRightUp(*get<Vector3>(v, 2), *get<Vector3>(v, 3)); return 0; }
	NB_FUNC(alignRightFront)			{ self(v)->alignRightFront(*get<Vector3>(v, 2), *get<Vector3>(v, 3)); return 0; }

	NB_FUNC(toWorldPos)					{ return push(v, self(v)->toWorldPos(*get<Vector3>(v, 2))); }
	NB_FUNC(toLocalPos)					{ return push(v, self(v)->toLocalPos(*get<Vector3>(v, 2))); }
	NB_FUNC(toWorldDir)					{ return push(v, self(v)->toWorldDir(*get<Vector3>(v, 2))); }
	NB_FUNC(toLocalDir)					{ return push(v, self(v)->toLocalDir(*get<Vector3>(v, 2))); }

	NB_FUNC(clearOffset)				{ self(v)->clearOffset(); return 0; }

	NB_FUNC(joinSyncGroup)				{ self(v)->joinSyncGroup(opt<Transform>(v, 2, NULL)); return 0; }
	NB_FUNC(getSyncGroup)
	{
		type::SyncGroup* group = self(v)->getSyncGroup();
		if (group == NULL) return 0;

		sq_newarray(v, 0);
		for (type::SyncGroup::SyncSet::iterator itr = group->_syncSet.begin(), end = group->_syncSet.end(); itr != end; ++itr)
		{
			arrayAppend(v, -1, *itr);
		}
		return 1;
	}

	NB_FUNC(applyOffset)				{ self(v)->applyOffset(*get<Vector3>(v, 2), *get<Quat>(v, 3)); return 0; }
	NB_FUNC(applyInvOffset)				{ self(v)->applyInvOffset(*get<Vector3>(v, 2), *get<Quat>(v, 3)); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::TransformAnchor, Transform, incRefCount, decRefCount);

class NB_TransformAnchor : TNitClass<TransformAnchor>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			PROP_ENTRY	(source),
			PROP_ENTRY	(target),
			PROP_ENTRY_R(delta),
			NULL
		};

		FuncEntry funcs[] =
		{
			CONS_ENTRY_H("(source=null, target: WorldTransform=null) // think as source = node, target = obj.Transform and then this.SyncWith(other.node)"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(source)					{ return push(v, self(v)->getSource()); }
	NB_PROP_GET(target)					{ return push(v, self(v)->getTarget()); }

	NB_PROP_GET(delta)	
	{
		const Vector3* delta = self(v)->getDelta();
		if (delta == NULL) return 0;
		return push(v, *delta);
	}

	NB_PROP_SET(source)					{ self(v)->setSource(opt<Transform>(v, 2, NULL)); return 0; }
	NB_PROP_SET(target)					{ self(v)->setTarget(opt<Transform>(v, 2, NULL)); return 0; }

	NB_CONS()							{ sq_setinstanceup(v, 1, new TransformAnchor(opt<Transform>(v, 2, NULL), opt<Transform>(v, 3, NULL))); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_REF(NIT_API, nit::AutomataComponent, Component, incRefCount, decRefCount);

class NB_AutomataComponent : TNitClass<AutomataComponent>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
 			PROP_ENTRY_R(automata),
 			PROP_ENTRY	(state),
			NULL
		};
		
		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_PROP_GET(automata)				{ return push(v, self(v)->getAutomata()); }
	NB_PROP_GET(state)					{ return push(v, self(v)->getState()); }

	NB_PROP_SET(state)					{ self(v)->setState(get<EventState>(v, 2)); return 0; }

	NB_CONS()							{ setSelf(v, new AutomataComponent()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NIT_API SQRESULT NitLibApp_Register(HSQUIRRELVM v)
{
	NB_Module::Register(v);
	NB_ModuleRegistry::Register(v);

	NB_Service::Register(v);

	NB_AppBase::Register(v);
	
	NB_Object::Register(v);
	NitBindComponent::Register(v);
	NB_Feature::Register(v);
	NB_World::Register(v);

	NB_ScriptFeature::Register(v);
	NB_ScriptObject::Register(v);
	NB_ScriptComponent::Register(v);

	NB_Package::Register(v);
	NB_PackBundle::Register(v);
	NB_PackBundleZBundleInfo::Register(v);
	NB_PackArchive::Register(v);
	NB_PackArchiveFile::Register(v);
	NB_PackageService::Register(v);

	NB_Session::Register(v);
	NB_SessionService::Register(v);

	NB_ContentSource::Register(v);
	NB_Content::Register(v);
	NB_ContentManager::Register(v);
	NB_ContentsService::Register(v);

	// TODO: Refactor to NitLibInput.cpp
	NB_InputService::Register(v);
	NB_InputEvent::Register(v);
	NB_InputUser::Register(v);
	NB_InputDevice::Register(v);
	NB_InputSource::Register(v);
	NB_InputSourceEvent::Register(v);
	NB_InputCommand::Register(v);
	NB_InputCommandEvent::Register(v);
	NB_InputAutomata::Register(v);
	NB_InputAutomataState::Register(v);
	NB_ScopedInputLock::Register(v);

	NB_InputTrigger::Register(v);
	NB_InputKeypad::Register(v);
	NB_InputSlider::Register(v);
	NB_InputStick::Register(v);
	NB_InputPointer::Register(v);
	NB_InputPointerEvent::Register(v);
	NB_InputAccel::Register(v);

	NB_PixelFormat::Register(v);
	NB_Image::Register(v);
	NB_Texture::Register(v);
	NB_TextureManager::Register(v);

	NB_Transform::Register(v);
	NB_TransformAnchor::Register(v);

	NB_AutomataComponent::Register(v);

	////////////////////////////////////

	sq_pushroottable(v);
	
	if (g_App)
	{
		NitBind::newSlot(v, -1, "app", g_App);
		NitBind::newSlot(v, -1, "modules", g_App->getModules());
	}

	if (g_Service)
	{
		NitBind::newSlot(v, -1, "session", g_Session);
		NitBind::newSlot(v, -1, "package", g_Package);
		NitBind::newSlot(v, -1, "contents", g_Contents);
		NitBind::newSlot(v, -1, "input", svc_Input);
	}
	sq_poptop(v);

	return SQ_OK;
}

static NitBindLibRegistry lib(NitLibApp_Register, "NitLibApp", "NitLibCore NitLibEvent NitLibMath NitLibTimer NitLibDebug NitLibData");

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;