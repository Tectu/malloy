#pragma once

#include "endpoint.hpp"
#include "../http/connection_t.hpp"
#include "../http/request_generator_t.hpp"
#include "../../core/http/generator.hpp"
#include "../../core/http/request.hpp"
#include "../../core/http/response.hpp"
#include "../../core/http/types.hpp"

#include <functional>
#include <optional>

namespace malloy::server
{

     /**
     * An HTTP endpoint.
     */
    struct endpoint_http :
        endpoint
    {
        template<typename... Bodies>
        using response_t = std::variant<malloy::http::response<Bodies>...>;

        template<typename... Bodies>
        using writer_t = std::function<void(const boost::beast::http::request_header<>&, std::variant<malloy::http::response<Bodies>...>&&, const http::connection_t&)>;

        using handle_retr = std::optional<malloy::http::response<boost::beast::http::string_body>>;
        using req_header_t = boost::beast::http::request_header<>;
        using req_t = http::request_generator_t;
        using url_t = malloy::http::uri;

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
         * @param location The location to check for matching.
         * @return Whether this endpoint matches the request.
         */
        [[nodiscard]]
        virtual
        bool matches(const req_header_t& req, [[maybe_unused]] const url_t& location) const
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
        handle_retr handle(const req_t& req, const http::connection_t& conn) const = 0;
    };

}
