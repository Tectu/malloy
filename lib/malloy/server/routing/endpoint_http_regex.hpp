#pragma once

#include "endpoint_http.hpp"

#include <functional>
#include <regex>

namespace malloy::server
{
    struct endpoint_http_regex :
        endpoint_http
    {
        using handler_t = std::function<malloy::http::response(const malloy::http::request&)>;

        std::regex resource_base;
        handler_t handler;

        [[nodiscard]]
        bool matches_resource(const malloy::http::request& req) const
        {
            std::smatch match_result;
            std::string str{ req.uri().raw() };
            return std::regex_match(str, match_result, resource_base);
        }

        [[nodiscard]]
        bool matches(const malloy::http::request& req) const override
        {
            // Resource
            if (!matches_resource(req))
                return false;

            // Base class
            return endpoint_http::matches(req);
        }

        [[nodiscard]]
        malloy::http::response handle(const malloy::http::request& req) const override
        {
            if (handler)
                return handler(req);

            return malloy::http::generator::server_error("no valid handler available.");
        }

    };

}
