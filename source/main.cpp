

#include "webviewinterface.h"
#include <nlohmann/json.hpp>
#include <string_view>

static auto toU8Vec(std::string_view string) -> std::vector<uint8_t>
{
    return { (const char*) string.begin(), (const char*) string.end() };
}

struct WebAppInterface : WebViewInterface
{
    auto getWindowTitle() const -> const char* override
    {
        return "LookingGlass - Test App";
    }

    auto onStart() -> void override
    {
        loadUrl("local://index.html");
    }

    auto onScriptMessage(const nlohmann::json& message) -> bool override
    {
        return false;
    }

    auto onUrlRequest(const UrlRequest& request) -> std::unique_ptr<UrlResponse> override
    {
        auto response = std::make_unique<UrlResponse>();

        const auto html =
        R"(<!DOCTYPE html>
            <html>
                <head>
                <script>
                    const v = {
                        "Message": "Hello"
                    };
                    window.webkit.messageHandlers.local.postMessage(v);
                </script>
                </head>
                <body>
                    <p>Hello</p>
                </body>
            </html>)";

        response->data = toU8Vec(html);
        response->mimetype = "text/html";
        return response;
    }
};

auto main(int, const char**) -> int
{
    WebAppInterface app;
    return startWebApp(&app);
}
