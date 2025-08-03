#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json_fwd.hpp>

struct UrlRequest
{
    std::string path;
};

struct UrlResponse
{
    std::string mimetype;
    std::vector<uint8_t> data;
};

struct Timer
{
    using ptr = std::unique_ptr<Timer>;

    virtual ~Timer() = default;
    virtual auto start(int milliseconds) -> void = 0;
    virtual auto stop() -> void = 0;
    virtual auto tick() -> void = 0;
};

struct WebViewInterface
{
    struct Preferences
    {
        float minimumFontSize            = 22;
        bool  shouldPrintbackgrounds     = false;
        bool  tabFocusesLinks            = false;
        bool  isTextInteractionEnabled   = false;
        bool  isElementFullscreenEnabled = true;
        bool  scriptsCanOpenWindows      = true;
        bool  fraudWarningsEnabled       = false;
    };

    virtual ~WebViewInterface();

    auto execute(const std::string& script) -> void;
    auto loadUrl(const std::string& url) -> void;
    auto loadHtml(const std::string& html) -> void;
    auto callOnMessageThread(std::function<void()>&& callback) -> void;

    auto makeTimer(int milliseconds, std::function<void()>&& function) -> std::unique_ptr<Timer>;

    virtual auto getWindowTitle() const -> const char* = 0;
    virtual auto getPreferences() const -> Preferences;
    virtual auto onStart() -> void = 0;
    virtual auto onScriptMessage(const nlohmann::json&) -> bool = 0;
    virtual auto onUrlRequest(const UrlRequest& request) -> std::unique_ptr<UrlResponse> = 0;

    struct Impl;
    Impl* impl = nullptr;
};

auto startWebApp(WebViewInterface* iface) -> int;
