#pragma once

#include "uri.hpp"

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

namespace malloy::http
{

    class request :
        public boost::beast::http::request<boost::beast::http::string_body>
    {
    public:
        request() = default;
        request(boost::beast::http::request<boost::beast::http::string_body>&& raw)
        {
            boost::beast::http::request<boost::beast::http::string_body>::operator=(std::move(raw));

            class uri u{ std::move(std::string{target().data(), target().size()}) };
            m_uri = std::move(u);
        }
        request(const request& other) = default;
        request(request&& other) noexcept = default;
        virtual ~request() = default;

        request& operator=(const request& rhs) = default;
        request& operator=(request&& rhs) noexcept = default;

        [[nodiscard]]
        class uri uri() const noexcept { return m_uri; }

        [[nodiscard]]
        class uri& uri() noexcept { return m_uri; }

    private:
        class uri m_uri;
    };

}
