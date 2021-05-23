#pragma once

#include "endpoint_http.hpp"

#include <filesystem>

namespace malloy::server
{

    struct endpoint_http_files :
        endpoint_http
    {
    public:
        std::string resource_base;
        std::filesystem::path base_path;

        [[nodiscard]]
        bool matches(const malloy::http::request& req) const override
        {
            return req.uri().resource_starts_with(resource_base);
        }

        [[nodiscard]]
        malloy::http::response handle(const malloy::http::request& req) const override
        {
            // Chop request resource path
            malloy::http::request req_clone{ req };
            req_clone.uri().chop_resource(resource_base);

            // Create response
            auto resp = malloy::http::generator::file(req_clone, base_path);

            return resp;
        }

    };

}
