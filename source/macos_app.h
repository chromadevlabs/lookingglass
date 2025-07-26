#pragma once

#import <Cocoa/Cocoa.h>
#include <Foundation/Foundation.h>
#include <Foundation/NSString.h>
#import <WebKit/WebKit.h>

#include "webviewinterface.h"

#include <cassert>
#include <nlohmann/json.hpp>

static auto nsStringToStdString(const NSString* string) -> std::string
{
    const char* utf8String = [string UTF8String];
    return utf8String ? utf8String : "";
}

static auto stdStringToNsString(const std::string& string) -> NSString*
{
    return [NSString stringWithUTF8String:string.c_str()];
}

static auto vecToNsData(const std::vector<uint8_t>& data) -> NSData*
{
    NSData* nsData = [NSData dataWithBytes:data.data() length:data.size()];
    return nsData;
}

static auto createNSURLResponse(int code,
                                const std::string& path,
                                const std::string& mimetype) -> NSHTTPURLResponse*
{
    NSURL* url = [NSURL URLWithString:stdStringToNsString(path)];
    NSDictionary* headerFields = @{ @"Content-Type": stdStringToNsString(mimetype) };
    NSHTTPURLResponse* response = [[NSHTTPURLResponse alloc] initWithURL:url
                                                              statusCode:code
                                                             HTTPVersion:@"HTTP/1.1"
                                                            headerFields:headerFields];
    return response;
}

struct WebViewInterface::Impl
{
    Impl(WKWebView* view) : webView(view)
    {
    }

    ~Impl() = default;

    WKWebView* webView;
};

WebViewInterface::~WebViewInterface()
{
    delete std::exchange(impl, nullptr);
}

auto WebViewInterface::getPreferences() const -> Preferences
{
    return {};
}

auto WebViewInterface::execute(const std::string& script) -> void
{
    assert(impl != nullptr);
}

auto WebViewInterface::loadUrl(const std::string& urlString) -> void
{
    assert(impl != nullptr);

    NSURL* url = [NSURL URLWithString:stdStringToNsString(urlString)];
    NSURLRequest* request = [NSURLRequest requestWithURL:url];
    [impl->webView loadRequest:request];
}

auto WebViewInterface::loadHtml(const std::string& html) -> void
{
    assert(impl != nullptr);

    NSString* htmlString = stdStringToNsString(html);
    [impl->webView loadHTMLString:htmlString
                          baseURL:nil];
}

static auto idToJson(id data)           -> nlohmann::json;
static auto idNumberToJson(id data)     -> nlohmann::json;
static auto idDateToJson(id data)       -> nlohmann::json;
static auto idStringToJson(id data)     -> nlohmann::json;
static auto idArrayToJson(id data)      -> nlohmann::json;
static auto idDictionaryToJson(id data) -> nlohmann::json;

static auto idNumberToJson(id data) -> nlohmann::json
{
    auto number = (NSNumber*) data;
    return nsStringToStdString([number stringValue]);
}

static auto idDateToJson(id data) -> nlohmann::json
{
    auto date = (NSDate*) data;
    return "DATE";
}

static auto idStringToJson(id data) -> nlohmann::json
{
    return nsStringToStdString((NSString*) data);
}

static auto idArrayToJson(id data) -> nlohmann::json
{
    nlohmann::json result;
    auto array = (NSArray*) data;

    for (id v in array)
        result += idToJson(v);

    return result;
}

static auto idDictionaryToJson(id data) -> nlohmann::json
{
    nlohmann::json result;
    auto dict = (NSDictionary*) data;

    for (NSString* key in dict)
        result[nsStringToStdString(key)] = idToJson(dict[key]);

    return result;
}

static auto idToJson(id data) -> nlohmann::json
{
    if ([data isKindOfClass:[NSNumber class]])
        return idNumberToJson(data);

    if ([data isKindOfClass:[NSDate class]])
        return idDateToJson(data);

    if ([data isKindOfClass:[NSString class]])
        return idStringToJson(data);

    if ([data isKindOfClass:[NSArray class]])
        return idArrayToJson(data);

    if ([data isKindOfClass:[NSDictionary class]])
        return idDictionaryToJson(data);

    assert("Unknown class type????");
    return {};
}

@interface MyCustomScriptMessageHandler : NSObject <WKScriptMessageHandler>
    @property WebViewInterface* webViewInterface;
@end

@implementation MyCustomScriptMessageHandler
    - (void) userContentController:(WKUserContentController *) userContentController
        didReceiveScriptMessage:(WKScriptMessage *) message
    {
        _webViewInterface->onScriptMessage(idToJson(message.body));
    }
@end

@interface MyCustomUrlSchemeHandler : NSObject <WKURLSchemeHandler>
    @property WebViewInterface* webViewInterface;
@end

@implementation MyCustomUrlSchemeHandler
    - (void) webView:(WKWebView*) webView startURLSchemeTask:(id<WKURLSchemeTask>) task
    {
        const auto* nsRequest = [task request];
        UrlRequest request
        {
            .path = nsStringToStdString(nsRequest.URL.path)
        };

        if (auto response = _webViewInterface->onUrlRequest(request))
        {
            auto nsResponse = createNSURLResponse(200, request.path, response->mimetype);

            [task didReceiveResponse:nsResponse];
            [task didReceiveData:vecToNsData(response->data)];
        }
        else
        {
            auto nsResponse = createNSURLResponse(404, request.path, response->mimetype);

            [task didReceiveResponse:nsResponse];
        }

        [task didFinish];
    }

    - (void) webView:(WKWebView*) webView stopURLSchemeTask:(id<WKURLSchemeTask>) task
    {
    }
@end

@interface AppDelegate : NSObject <NSApplicationDelegate>
    @property(nonatomic, strong) NSWindow* window;
    @property(nonatomic, strong) WKWebView* webView;
    @property(nonatomic, strong) MyCustomScriptMessageHandler* scriptMessageHandler;
    @property(nonatomic, strong) MyCustomUrlSchemeHandler* urlSchemeHandler;
    @property                    WebViewInterface* webViewInterface;
@end

@implementation AppDelegate
    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification
    {
        // 1. Create the main window
        NSRect contentRect = NSMakeRect(100, 100, 1000, 700);
        NSWindowStyleMask styleMask = NSWindowStyleMaskTitled
                                    | NSWindowStyleMaskClosable
                                    | NSWindowStyleMaskMiniaturizable
                                    | NSWindowStyleMaskResizable;

        _window = [[NSWindow alloc] initWithContentRect:contentRect
                                              styleMask:styleMask
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];

        [_window setTitle:stdStringToNsString(_webViewInterface->getWindowTitle())];
        [_window setMinSize:NSMakeSize(400, 300)];
        [_window center];

        WKWebViewConfiguration* configuration = [[WKWebViewConfiguration alloc] init];

        //[configuration.preferences setValue:@(YES) forKey:@"developerExtrasEnabled"];
        //[configuration.preferences setValue:@(NO) forKey:@"inspectorUsesCustomAppearance"];

        const auto prefs = _webViewInterface->getPreferences();
        configuration.preferences.minimumFontSize = prefs.minimumFontSize;
        configuration.preferences.shouldPrintBackgrounds = prefs.shouldPrintbackgrounds;
        configuration.preferences.tabFocusesLinks = prefs.tabFocusesLinks;
        configuration.preferences.textInteractionEnabled = prefs.isTextInteractionEnabled;
        configuration.preferences.elementFullscreenEnabled = prefs.isElementFullscreenEnabled;
        configuration.preferences.javaScriptCanOpenWindowsAutomatically = prefs.scriptsCanOpenWindows;
        configuration.preferences.fraudulentWebsiteWarningEnabled = prefs.fraudWarningsEnabled;

        _scriptMessageHandler = [[MyCustomScriptMessageHandler alloc] init];
        _scriptMessageHandler.webViewInterface = _webViewInterface;
        [configuration.userContentController addScriptMessageHandler:_scriptMessageHandler
                                                                name:@"local"];

        _urlSchemeHandler = [[MyCustomUrlSchemeHandler alloc] init];
        _urlSchemeHandler.webViewInterface = _webViewInterface;
        [configuration setURLSchemeHandler:_urlSchemeHandler forURLScheme:@"local"];

        _webView = [[WKWebView alloc] initWithFrame:contentRect
                                      configuration:configuration];
        _webViewInterface->impl = new WebViewInterface::Impl{_webView};

        _webView.autoresizingMask = NSViewWidthSizable
                                  | NSViewHeightSizable;
        _webView.inspectable = YES;

        _window.contentView = _webView;
        [_window makeKeyAndOrderFront:nil];

        // Kick off our callback
        _webViewInterface->onStart();
    }

    - (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication*) sender
    {
        return YES;
    }

    - (void) applicationWillTerminate:(NSNotification*) aNotification
    {
        [self.webView.configuration.userContentController removeScriptMessageHandlerForName:@"myHandler"];
    }
@end
