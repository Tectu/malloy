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
        /**
         * The method type to use.
         */
        using method_type  = boost::beast::http::verb;

        /**
         * The method this route matches.
         */
        method_type method;

        /**
         * The rule to be applied to the URI.
         */
        std::regex rule;

        /**
         * The execution handler.
         *
         * This handler will be executed by the @ref router if the route matches.
         */
        std::function<Response(const Request&)> handler;

        /**
         * Default constructor.
         */
        route() = default;

        /**
         * Copy constructor.
         *
         * @param other The object to copy-construct from.
         */
        route(const route& other) = default;

        /**
         * Move constructor.
         *
         * @param other The object to move-construct from.
         */
        route(route&& other) noexcept = default;

        /**
         * Destructor
         */
        virtual ~route() = default;

        /**
         * Copy assignment operator.
         *
         * @param rhs The object to copy-assign from.
         * @return Reference to the assignee.
         */
        route& operator=(const route& rhs) = default;

        /**
         * Move assignment operator.
         *
         * @param rhs The object to move-assign from.
         * @return Reference to the assignee.
         */
        route& operator=(route&& rhs) noexcept = default;

        /**
         * Checks whether this route would match the specified resource.
         *
         * @param resource The resource to check.
         * @return `true` if this route matches, `false` otherwise.
         */
        [[nodiscard]]
        bool matches_resource(const std::string& resource) const
        {
            std::smatch match_result;
            return std::regex_match(resource, match_result, rule);
        }

        /**
         * Checks whether this route would match the specified request.
         *
         * @param req The request to check.
         * @return `true` if this route matches, `false` otherwise.
         */
        [[nodiscard]]
        bool matches_request(const Request& req) const
        {
            // Check method
            if (req.method() not_eq method)
                return false;

            // Check rule
            const std::string resource{ req.uri().raw() };
            if (not matches_resource(resource))
                return false;

            return true;
        }

    };

}
