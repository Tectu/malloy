#pragma once

#include "request.hpp"

namespace malloy::http::server
{

    template<
        typename Request,
        typename Response
    >
    struct route
    {
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
         * Checks whether this route would match the specified request.
         *
         * @param req The request to check.
         * @return `true` if this route matches, `false` otherwise.
         */
        [[nodiscard]]
        virtual bool matches(const Request& req) const = 0;

        /**
         * The execution handler.
         *
         * This handler will be executed by the @ref router if the route matches.
         */
        [[nodiscard]]
        virtual Response handle(const Request& req) const = 0;
    };

}
