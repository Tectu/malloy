#pragma once

#include "endpoint.hpp"
#include "../../http/generator.hpp"
#include "../../http/request.hpp"
#include "../../http/response.hpp"
#include "../../http/types.hpp"

#include "malloy/server/http/connection/connection_t.hpp"
#include "malloy/server/http/connection/request_generator_t.hpp"

#include <optional>
#include <functional>

namespace malloy::server
{

    namespace detail {
        struct request_info {
            template<typename T>
            request_info(const boost::beast::http::request<T>& req) : version{ req.version() }, keep_alive{ req.keep_alive() } {} // Implicit 
            request_info(int version_, bool keep_alive_) : version{ version_ }, keep_alive{ keep_alive_ } {}


            int version;
            bool keep_alive;
        };
    }
     /**
     * An HTTP endpoint.
     */
    struct endpoint_http :
        endpoint
    {
        template<typename Req, typename... Bodies>
        using writer_t = std::function<void(const malloy::http::request<Req>&, std::variant<malloy::http::response<Bodies>...>&&, const http::connection_t&)>;

        using handle_retr = std::pair<detail::request_info, std::optional<malloy::http::response<boost::beast::http::string_body>>;
        using req_header_t = boost::beast::http::request_header<>;
        using req_t = http::request_generator_t;


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
        bool matches(const req_header_t& req) const
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
