#pragma once

#include "types.hpp"

#include <boost/beast/http/string_body.hpp>

namespace malloy::http
{

    /**
     * Represents an HTTP request.
     */
    template<typename Body = boost::beast::http::string_body>
    class request :
        public boost::beast::http::request<Body>
    {
        using msg_t = boost::beast::http::request<Body>;

    public:
        request() = default;

        /**
         * Constructor.
         *
         * @param method_ The HTTP method
         * @param host The host to connect to.
         * @param port The port at which the host serves requests.
         * @param target_ The target.
         */
        request(http::method method_, std::string_view host, const std::uint16_t port, std::string_view target_)
            : m_port(port)
        {
            msg_t::version(11);
            msg_t::method(method_);
            msg_t::target(target_);
            msg_t::set(http::field::host, host);
        }

        /**
         * Constructor
         *
         * @param raw The underlying raw HTTP message
         */
        explicit
        request(msg_t&& raw)
        {
            using namespace boost::beast::http;

            // Underlying
            msg_t::operator=(std::move(raw));
        }

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
        virtual
        ~request() = default;

        /**
         * Copy assignment operator.
         *
         * @param rhs The object to copy-assign from.
         * @return A reference to the assignee.
         */
        request&
        operator=(const request& rhs) = default;

        /**
         * Move assignment operator.
         *
         * @param rhs The object to move-assign from.
         * @return A reference to the assignee.
         */
        request&
        operator=(request&& rhs) noexcept = default;

        /**
         * Retrieve the port.
         *
         * @return The port.
         */
        [[nodiscard]]
        std::uint16_t
        port() const noexcept
        {
            return m_port;
        }

    private:
        std::uint16_t m_port = 0;
    };
}

#include <sstream>

namespace malloy
{

    template<typename Body>
    [[nodiscard]]
    std::string
    to_string(const http::request<Body>& r)
    {
        std::ostringstream ss;
        ss << r;

        return ss.str();
    }

}
