#pragma once

#include "types.hpp"
#include "cookie.hpp"
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
        /**
         * Default constructor.
         */
        response() = default;

        /**
         * Constructor.
         *
         * @param status_ The HTTP status to use.
         */
        response(const status& status_)
        {
            result(status_);
        }

        /**
         * Copy constructor.
         *
         * @param other The object to copy-construct from.
         */
        response(const response& other) = default;

        /**
         * Move constructor.
         *
         * @param other The object to move-construct from.
         */
        response(response&& other) noexcept = default;

        /**
         * Destructor.
         */
        virtual ~response() = default;

        /**
         * Copy assignment operator.
         *
         * @param rhs The object to copy-assign from.
         * @return A reference to the assignee.
         */
        response& operator=(const response& rhs) = default;

        /**
         * Move assignment operator.
         *
         * @param rhs The object to move-assign from.
         * @return A reference to the assignee.
         */
        response& operator=(response&& rhs) noexcept = default;

        /**
         * Retrieve the HTTP status.
         *
         * @return The HTTP status
         */
        [[nodiscard]]
        status status() const { return result(); }

        /**
         * Adds a cookie.
         *
         * @param c The cookie.
         */
        void add_cookie(const cookie& c)
        {
            insert(boost::beast::http::field::set_cookie, c.to_string());
        }
    };

}
