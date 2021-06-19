#pragma once

#include "endpoint.hpp"
#include "../../http/generator.hpp"
#include "../../http/request.hpp"
#include "../../http/response.hpp"
#include "../../http/types.hpp"

#include "malloy/server/http/connection/connection_t.hpp"

#include <optional>
#include <functional>

namespace malloy::server
{

    /**
     * An HTTP endpoint.
     */
    struct endpoint_http :
        endpoint
    {
        using handle_retr = std::optional<malloy::http::response<boost::beast::http::string_body>>;

        malloy::http::method method = malloy::http::method::unknown;

        endpoint_http() = default;
        endpoint_http(const endpoint_http& other) = default;
        endpoint_http(endpoint_http&& other) noexcept = default;
        ~endpoint_http() override = default;

        endpoint_http& operator=(const endpoint_http& rhs) = default;
        endpoint_http& operator=(endpoint_http&& rhs) noexcept = default;

        /**
         * Checks whether this endpoint would match the specified request.
         *
         * The default implementation only checks for the matching method.
         *
         * @param req The request to check.
         * @return Whether this endpoint matches the request.
         */
        [[nodiscard]]
        virtual
        bool matches(const malloy::http::request& req) const
        {
            return method == req.method();
        }

        /**
         * Handle the request and return the corresponding response.
         *
         * @param req The request.
         * @return The response for the specified request.
         */
        [[nodiscard]]
        virtual
        handle_retr handle(const malloy::http::request& req, const http::connection_t& conn) const = 0;
    };

}
