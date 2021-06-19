#pragma once

#include "endpoint_http.hpp"

namespace malloy::server
{

    struct endpoint_http_redirect :
        endpoint_http
    {
    public:
        http::status status;
        std::string resource_old;
        std::string resource_new;


        [[nodiscard]]
        bool matches(const malloy::http::request& req) const override
        {
            return resource_old == req.uri().resource_string();
        }

        [[nodiscard]]
        handle_retr handle(const malloy::http::request& req, const http::connection_t&) const override
        {
            return malloy::http::generator::redirect(status, resource_new);
        }

    };

}
