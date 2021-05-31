#pragma once

#include "cookie.hpp"
#include "uri.hpp"
#include "types.hpp"

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

namespace malloy::http
{

    /**
     * Represents an HTTP request.
     */
    class request :
        public boost::beast::http::request<boost::beast::http::string_body>
    {
    public:
        request() = default;

        /**
         * Cosntructor.
         *
         * @param method_ The HTTP method
         * @param host The host to connect to.
         * @param port The port at which the host serves requests.
         * @param target_ The target.
         */
        request(
            http::method method_,
            std::string_view host,
            const std::uint16_t port,
            std::string_view target_
        );

        /**
         * Constructor
         *
         * @param raw The underlying raw HTTP message
         */
        request(boost::beast::http::request<boost::beast::http::string_body>&& raw);

        /**
         * Copy constructor.
         *
         * @param other The object to copy construct from.
         */
        request(const request& other) = default;

        /**
         * Move constructor.
         *
         * @param other The object to move-construct from.
         */
        request(request&& other) noexcept = default;

        /**
         * Destructor.
         */
        virtual ~request() = default;

        /**
         * Copy assignment operator.
         *
         * @param rhs The object to copy-assign from.
         * @return A reference to the assignee.
         */
        request& operator=(const request& rhs) = default;

        /**
         * Move assignment operator.
         *
         * @param rhs The object to move-assign from.
         * @return A reference to the assignee.
         */
        request& operator=(request&& rhs) noexcept = default;

        /**
         * Retrieve the port.
         *
         * @return The port.
         */
        [[nodiscard]]
        std::uint16_t port() const noexcept { return m_port; }

        /**
         * Returns the URI portion of the request.
         *
         * @return A copy of the URI.
         */
        [[nodiscard]]
        class uri uri() const noexcept { return m_uri; }

        /**
         * Returns the URI portion of the request.
         *
         * @return A reference to the URI.
         */
        [[nodiscard]]
        class uri& uri() noexcept { return m_uri; }

        /**
         * Returns the request's cookies.
         *
         * @return The cookies.
         */
        [[nodiscard]]
        std::unordered_map<std::string, std::string> cookies() const noexcept { return m_cookies; }

        /**
         * Checks whether a particular cookie is present.
         *
         * @return Whether the specified cookie is present.
         */
        [[nodiscard]]
        bool has_cookie(const std::string& name) const
        {
            return m_cookies.contains(name);
        }

        /**
         * Gets the value of a cookie.
         */
        [[nodiscard]]
        std::string_view cookie(const std::string_view& name) const;

    private:
        std::uint16_t m_port = 0;
        class uri m_uri;
        std::unordered_map<std::string, std::string> m_cookies;
    };

}
