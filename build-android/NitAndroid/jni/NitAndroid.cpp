#include "nit/nit.h"
#include "nit/runtime/NitRuntime.h"

#include <android/window.h>

NS_NIT_BEGIN;

NIT_PLUGIN_DECLARE(NitRenderPlugin);
NIT_PLUGIN_DECLARE(Nit2dPlugin);
NIT_PLUGIN_DECLARE(NitNetPlugin);

class NitAndroidApp : public AppBase
{
protected:
	virtual void registerStaticPlugins()
	{
		Register("nitrender", NIT_PLUGIN_CREATE(NitRenderPlugin));
		Register("nit2d", NIT_PLUGIN_CREATE(Nit2dPlugin));
		Register("nitnet", NIT_PLUGIN_CREATE(NitNetPlugin));
	}
	
	virtual void onInit()
	{
		AppBase::onInit();
		
		g_SessionService->Register("startup", AppRuntime::defaultSessionFactory);
	}
};

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////

extern "C" void android_main(android_app* app)
{
	using namespace nit;

	g_AndroidJNI->init(app->activity);

	AppRuntime* rt = new AppRuntime();

	LOG(0, "++ AppRuntime %08x created\n", rt);

	app->userData = NULL; // NitRuntime does not use userdata, so my app can use freely

	// Let the screen turn on when debug start (must call this function prior to window init)
	ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_TURN_SCREEN_ON, 0);

	// Consider status bar position on layout
	ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_LAYOUT_IN_SCREEN | AWINDOW_FLAG_LAYOUT_INSET_DECOR, 0); // TODO: not working

	rt->linkAndroidApp(app);

	rt->prepare(new NitAndroidApp(), "nit-test.app.cfg");

	LOG(0, ".. Entering android_main thread loop\n");

	rt->run();

	LOG(0, ".. Leaving android_main thread loop\n");

	LOG(0, ".. Detaching jni env\n");
	g_AndroidJNI->getJavaVM()->DetachCurrentThread();

	delete rt;
	LOG(0, ".. AppRuntime %08x deleted\n", rt);

	LogManager::getSingleton().shutdown();
	
	// HACK: set callbacks to be NULL
	// so that avoid deadlock problem at callback time following ANativeActivity_finish()
	memset(app->activity->callbacks, 0, sizeof(*app->activity->callbacks));

	ANativeActivity_finish(app->activity);
}

////////////////////////////////////////////////////////////////////////////////
