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

        request(
            http::method method_,
            const char* host,
            const char* target_
        )
        {
            version(11);
            method(method_);
            target(target_);
            set(http::field::host, host);
        }

        /**
         * Constructor
         *
         * @param raw The underlying raw HTTP message
         */
        request(boost::beast::http::request<boost::beast::http::string_body>&& raw)
        {
            using namespace boost::beast::http;

            using base_type = boost::beast::http::request<boost::beast::http::string_body>;

            // Underlying
            base_type::operator=(std::move(raw));

            // URI
            class uri u{ std::string{target().data(), target().size()} };
            m_uri = std::move(u);

            // Cookies
            {
                const auto& [begin, end] = base().equal_range(field::cookie);
                for (auto it = begin; it != end; it++) {
                    const auto& str = it->value();

                    const auto& sep_pos = it->value().find('=');
                    if (sep_pos == std::string::npos)
                        continue;

                    std::string key{ str.substr(0, sep_pos) };
                    std::string value{ str.substr(sep_pos+1) };
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
        std::unordered_map<std::string, std::string> cookies() const noexcept
        {
            return m_cookies;
        }

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
        std::string_view cookie(const std::string_view& name) const
        {
            const auto& it = std::find_if(
                std::cbegin(m_cookies),
                std::cend(m_cookies),
                [&name]( const auto& pair ) {
                    return pair.first == name;
                }
            );

            if (it == std::cend(m_cookies))
                return { };

            return it->second;
        }

    private:
        class uri m_uri;
        std::unordered_map<std::string, std::string> m_cookies;
    };

}
