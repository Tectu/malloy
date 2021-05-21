#pragma once

#include "route.hpp"
#include "malloy/http/generator.hpp"

namespace malloy::server
{

    template<
        typename Request,
        typename Response
    >
    class route_redirect :
        public route<Request, Response>
    {
    public:
        http::status status;
        std::string resource_old;
        std::string resource_new;

        [[nodiscard]]
        bool matches(const Request& req) const override
        {
            return resource_old == req.uri().resource_string();
        }

        [[nodiscard]]
        Response handle(const Request& req) const override
        {
            return http::generator::redirect(status, resource_new);
        }

    };

}
