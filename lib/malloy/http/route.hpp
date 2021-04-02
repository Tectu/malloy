#pragma once

#include <regex>

#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/string_body.hpp>

#include "request.hpp"

namespace malloy::http::server
{

    template<
        typename Request,
        typename Response
    >
    class route
    {
    public:
        using method_type  = boost::beast::http::verb;

        method_type method;
        std::regex rule;
        std::function<Response(const Request&)> handler;

        // Construction
        route() = default;
        route(const route& other) = default;
        route(route&& other) noexcept = default;
        virtual ~route() = default;

        // Operators
        route& operator=(const route& rhs) = default;
        route& operator=(route&& rhs) noexcept = default;

        // General
        [[nodiscard]]
        bool matches_target(const std::string& target) const
        {
            std::smatch match_result;
            return std::regex_match(target, match_result, rule);
        }

        [[nodiscard]]
        bool matches_request(const Request& req) const
        {
            // Check method
            if (req.method() not_eq method)
                return false;

            // Check rule
            const std::string target{ req.uri().resource_string() };
            if (not matches_target(target))
                return false;

            return true;
        }

    };

}
