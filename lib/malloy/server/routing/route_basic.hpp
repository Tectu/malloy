#pragma once

#include "route.hpp"
#include "malloy/http/generator.hpp"

#include <boost/beast/http/verb.hpp>

#include <regex>

namespace malloy::server
{

    template<
        typename Request,
        typename Response
    >
    class route_basic :
        public route<Request, Response>
    {
    public:
        using method_type  = boost::beast::http::verb;

        method_type method = method_type::unknown;
        std::regex resource_base;
        std::function<Response(const Request&)> handler;

        [[nodiscard]]
        bool matches_resource(const Request& req) const
        {
            std::smatch match_result;
            std::string str{ req.uri().raw() };
            return std::regex_match(str, match_result, resource_base);
        }

        [[nodiscard]]
        bool matches(const Request& req) const override
        {
            // Resource
            if (not matches_resource(req))
                return false;

            // Method
            return (method == req.method());
        }

        [[nodiscard]]
        Response handle(const Request& req) const override
        {
            if (handler)
                return handler(req);

            return http::generator::server_error("no valid handler available.");
        }

    };

}
