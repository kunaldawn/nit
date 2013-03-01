#import <UIKit/UIKit.h>

#include "nit/nit.h"

#import "nit2d/Nit2dAppDelegate.h"

NS_NIT_BEGIN;

NIT_PLUGIN_DECLARE(NitRenderPlugin);
NIT_PLUGIN_DECLARE(Nit2dPlugin);
NIT_PLUGIN_DECLARE(NitNetPlugin);

class NitTestIOS : public AppBase
{
protected:
    virtual void RegisterStaticPlugins()
    {
        Register("nitrender", NIT_PLUGIN_CREATE(NitRenderPlugin));
        Register("nit2d", NIT_PLUGIN_CREATE(Nit2dPlugin));
        Register("nitnet", NIT_PLUGIN_CREATE(NitNetPlugin));
    }
    
    virtual void OnInit()
    {
        AppBase::onInit();
        
//        g_SessionService->Register("startup", AppRuntime::DefaultSessionFactory);
    }
};

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////

@interface AppDelegate : Nit2dAppDelegate
@end

@implementation AppDelegate
@end

////////////////////////////////////////////////////////////////////////////////

using namespace nit;

int main(int argc, char *argv[])
{
    AppRuntime rt;
    
    rt.setArguments(argc, argv);
    
    rt.prepare(new NitTestIOS(), "nit-test.app.cfg");
    
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}

