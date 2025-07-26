
#include "macos_app.h"

static AppDelegate* sAppDelegate;

auto startWebApp(WebViewInterface* iface) -> int
{
    @autoreleasepool
    {
        [NSApplication sharedApplication];

        sAppDelegate                  = [[AppDelegate alloc] init];
        sAppDelegate.webViewInterface = iface;

        [NSApp setDelegate:sAppDelegate];
        [NSApp run];
    }

    return 0;
}
