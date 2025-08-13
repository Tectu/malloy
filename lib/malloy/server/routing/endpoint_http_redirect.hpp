#pragma once

#include "endpoint_http.hpp"
#include "../../core/http/utils.hpp"

namespace malloy::server
{

    struct endpoint_http_redirect :
        endpoint_http
    {
    public:
        malloy::http::status status;
        std::string resource_old;
        std::string resource_new;


        [[nodiscard]]
        bool
        matches(const req_header_t& head) const override
        {
            return malloy::http::resource_string(head) == resource_old;
        }

        [[nodiscard]]
        handle_retr
        handle(const req_t&, const http::connection_t&) const override
        {
            return malloy::http::generator::redirect(status, resource_new);
        }

    };

}
