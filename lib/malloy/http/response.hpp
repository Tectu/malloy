#pragma once

#include "http.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace malloy::http
{

    /**
     * The response type.
     */
    class response :
        public boost::beast::http::response<boost::beast::http::string_body>
    {
    public:
        response() = default;
        response(const status& _status)
        {
            result(_status);
        }

        response(const response& other) = default;
        response(response&& other) noexcept = default;
        virtual ~response() = default;

        response& operator=(const response& rhs) = default;
        response& operator=(response&& rhs) noexcept = default;

        [[nodiscard]]
        status status() const { return result(); }

        [[nodiscard]]
        static
        response bad_request(std::string_view reason)
        {
            response res(status::bad_request);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.body() = reason;
            return res;
        }
    };

    // Returns a bad request response
    auto const bad_request =
        [](const auto& req, boost::beast::string_view why)
        {
            boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::bad_request, req.version()};
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = std::string(why);
            res.prepare_payload();
            return res;
        };

    // Returns a not found response
    auto const not_found =
        [](const auto& req, boost::beast::string_view target)
        {
            boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::not_found, req.version()};
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "The resource '" + std::string(target) + "' was not found.";
            res.prepare_payload();
            return res;
        };

    // Returns a server error response
    auto const server_error =
        [](const auto& req, boost::beast::string_view what)
        {
            boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::internal_server_error, req.version()};
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "An error occurred: '" + std::string(what) + "'";
            res.prepare_payload();
            return res;
        };

}
