#pragma once

#include "http.hpp"
#include "../utils.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <filesystem>

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
            res.prepare_payload();

            return res;
        }

        [[nodiscard]]
        static
        response not_found(std::string_view resource)
        {
            response res(status::not_found);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.body() = "The resource '" + std::string(resource) + "' was not found.";
            res.prepare_payload();

            return res;
        }

        [[nodiscard]]
        static
        response server_error(std::string_view what)
        {
            response res(status::internal_server_error);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.body() = "An error occurred: '" + std::string(what) + "'";
            res.prepare_payload();

            return res;
        }

        [[nodiscard]]
        static
        response file(const std::filesystem::path& path)
        {
            // Get file content
            const std::string& file_content = malloy::file_contents(path);

            // Get mime type
            const std::string_view& mime_type = malloy::mime_type(path);

            response res{status::ok};
            res.set(boost::beast::http::field::content_type, boost::string_view{ mime_type.data(), mime_type.size() });
            res.body() = file_content;
            res.prepare_payload();

            return res;
        }
    };

}
