#include "preflight_config.hpp"

using namespace malloy::server::http;

void preflight_config::setup_response(
    malloy::http::response<>& resp,
    const std::span<malloy::http::method> methods
) const
{
    // Create a string representing all supported methods
    std::string methods_string;
    for (const auto& method : methods) {
        methods_string += boost::beast::http::to_string(method);
        if (&method != &methods.back())
            methods_string += ", ";
    }

    //resp.set(malloy::http::field::content_type, "text/html");
    resp.base().set("Access-Control-Allow-Origin", origin);
    resp.base().set("Access-Control-Allow-Methods", methods_string);
    //resp.base().set("Access-Control-Allow-Headers", "Content-Type");
    //resp.base().set("Access-Control-Max-Age", "60");
}