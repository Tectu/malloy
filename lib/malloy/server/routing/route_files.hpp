#pragma once

#include "route.hpp"

#include <filesystem>

namespace malloy::server
{

    template<
        typename Request,
        typename Response
    >
    class route_files :
        public route<Request, Response>
    {
    public:
        std::string resource_base;
        std::filesystem::path base_path;

        [[nodiscard]]
        bool matches(const Request& req) const override
        {
            return req.uri().resource_starts_with(resource_base);
        }

        [[nodiscard]]
        Response handle(const Request& req) const override
        {
            // Chop request resource path
            Request req_clone{ req };
            req_clone.uri().chop_resource(resource_base);

            // Create response
            auto resp = http::generator::file(req_clone, base_path);

            return resp;
        }

    };

}
