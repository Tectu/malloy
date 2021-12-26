#pragma once

#include "cookie.hpp"
#include "types.hpp"

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

#include <unordered_map>

namespace malloy::http
{
    template<typename Fields = boost::beast::http::fields>
    using request_header = boost::beast::http::request_header<Fields>;

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

            // Cookies
            {
                const auto &[begin, end] = msg_t::base().equal_range(field::cookie);
                for (auto it = begin; it != end; it++) {
                    const auto &str = it->value();

                    const auto &sep_pos = it->value().find('=');
                    if (sep_pos == std::string::npos)
                        continue;

                    std::string key{str.substr(0, sep_pos)};
                    std::string value{str.substr(sep_pos + 1)};
                    m_cookies.insert_or_assign(std::move(key), std::move(value));
                }
            }
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
        [[nodiscard]] std::string_view
        cookie(const std::string_view& name) const
        {
            const auto& it = std::find_if(
                std::cbegin(m_cookies),
                std::cend(m_cookies),
                [&name](const auto& pair) {
                    return pair.first == name;
                }
            );

            if (it == std::cend(m_cookies))
                return { };

            return it->second;
        }

    private:
        std::uint16_t m_port = 0;
        std::unordered_map<std::string, std::string> m_cookies;
    };
}

