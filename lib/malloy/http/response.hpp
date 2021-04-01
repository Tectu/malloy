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
    };

}
