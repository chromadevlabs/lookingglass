

#include "webviewinterface.h"
#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <optional>
#include <map>

static auto toU8Vec(std::string_view string) -> std::vector<uint8_t>
{
    return { (const char*) string.begin(),
             (const char*) string.end() };
}

static auto file_get_last_write_time(std::string_view filepath)
{
    using namespace std::filesystem;
    std::error_code ec;

    return last_write_time(path(filepath), ec).time_since_epoch()
                                              .count();
}

static auto file_get_size(std::string_view filepath) -> size_t
{
    using namespace std::filesystem;
    std::error_code ec;

    return file_size(path(filepath), ec);
}

static auto file_read_binary(std::string_view filepath) -> std::optional<std::vector<uint8_t>>
{
    if (auto file = std::ifstream(filepath, std::ios::binary); file.is_open())
    {
        std::vector<uint8_t> vec;

        vec.resize(file_get_size(filepath));
        file.read((char*) vec.data(), vec.size());

        return std::make_optional(vec);
    }

    return std::nullopt;
}

static auto file_read_string(std::string_view filepath) -> std::optional<std::string>
{
    if (auto file = std::ifstream(filepath); file.is_open())
    {
        std::string string;

        while (! file.eof())
        {
            static char buffer[1024]{};
            auto n = file.readsome(buffer, sizeof(buffer));
            string += { buffer, buffer + n };
        }

        return std::make_optional(string);
    }

    return std::nullopt;
}

struct WebAppInterface : WebViewInterface
{
    using endpoint_t = std::function<void(const nlohmann::json&)>;
    std::map<std::string, endpoint_t> functions;
    Timer::ptr timer;

    WebAppInterface()
    {
        registerScriptEndpoint("print", [] (const nlohmann::json& json)
            {
                auto string = json[0].get<std::string>();
                printf("print(\"%s\")\n", string.c_str());
            });
    }

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
        try
        {
            auto key = message["name"].get<std::string>();
            auto func = functions.at(key);
            func(message["content"]);
            return true;
        }
        catch(const std::exception & e)
        {
            auto json = message.dump();
            printf("Error: bad script call: %s\n", json.c_str());
        }

        return false;
    }

    auto registerScriptEndpoint(const std::string& name, endpoint_t&& endpoint) -> void
    {
        functions[name] = std::move(endpoint);
    }

    auto onUrlRequest(const UrlRequest& request) -> std::unique_ptr<UrlResponse> override
    {
        constexpr std::string_view prefix = "local://";
        constexpr auto root               = "/Users/chroma/Desktop/lookingglass/app/";

        std::string path = root + request.path.substr(prefix.length());

        printf("Request: %s\n", path.c_str());

        if (auto data = file_read_binary(path))
        {
            auto response = std::make_unique<UrlResponse>();

            response->data     = *data;
            response->mimetype = "text/html";

            return response;
        }

        return nullptr;
    }
};

auto main(int, const char**) -> int
{
    WebAppInterface app;
    return startWebApp(&app);
}
