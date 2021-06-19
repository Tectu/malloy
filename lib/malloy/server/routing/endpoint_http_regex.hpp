#pragma once

#include "endpoint_http.hpp"

#include "malloy/http/response.hpp"

#include <functional>
#include <regex>

namespace malloy::server
{
    struct resource_matcher {
        [[nodiscard]]
        virtual bool matches_resource(const malloy::http::request& req) const = 0;
    };

    template<typename Response>
    struct endpoint_http_regex :
        endpoint_http, public resource_matcher
    {
        using handler_t = std::function<Response(const malloy::http::request&)>;

        std::regex resource_base;
        handler_t handler;
        std::function<void(const malloy::http::request&, Response&&, const http::connection_t&)> writer;

        [[nodiscard]]
        bool matches_resource(const malloy::http::request& req) const override
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
        handle_retr handle(const malloy::http::request& req, const http::connection_t& conn) const override
        {
            if (handler) {
                writer(req, handler(req), conn);
                return std::nullopt;
            }

            return malloy::http::generator::server_error("no valid handler available.");
        }

    };

}
