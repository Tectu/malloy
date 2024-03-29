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
    template<typename Body = boost::beast::http::string_body, typename Fields = boost::beast::http::fields>
    class response :
        public boost::beast::http::response<Body, Fields>
    {
        using msg_t = boost::beast::http::response<Body, Fields>;

    public:
        /**
         * Default constructor.
         */
        response() = default;

        /**
         * Constructor.
         *
         * @param msg The HTTP message.
         */
        explicit
        response(msg_t&& msg)
        {
            msg_t::operator=(std::move(msg));
        }

        /**
         * Constructor.
         *
         * @param header The header.
         */
        explicit
        response(const boost::beast::http::response_header<Fields>& header) :
            msg_t{ header }
        {
        }

        /**
         * Constructor.
         *
         * @param header The header.
         */
        explicit
        response(boost::beast::http::response_header<Fields>&& header) :
            msg_t{ std::move(header) }
        {
        }

        /**
         * Constructor.
         *
         * @param status_ The HTTP status to use.
         */
        explicit
        response(const status& status_)
        {
            msg_t::result(status_);
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
        virtual
        ~response() = default;

        /**
         * Copy assignment operator.
         *
         * @param rhs The object to copy-assign from.
         * @return A reference to the assignee.
         */
        response&
        operator=(const response& rhs) = default;

        /**
         * Move assignment operator.
         *
         * @param rhs The object to move-assign from.
         * @return A reference to the assignee.
         */
        response&
        operator=(response&& rhs) noexcept = default;

        /**
         * Set the HTTP status code.
         *
         * @param status The HTTP status code.
         */
        void
        set_status(http::status status) { msg_t::result(status); }

        /**
         * Retrieve the HTTP status.
         *
         * @return The HTTP status
         */
        [[nodiscard]]
        http::status
        status() const { return msg_t::result(); }

        /**
         * Adds a cookie.
         *
         * @param c The cookie.
         */
        void
        add_cookie(const cookie& c)
        {
            msg_t::insert(malloy::http::field::set_cookie, c.to_string());
        }
    };

}

#include <sstream>

namespace malloy
{

    template<typename Body, typename Fields>
    [[nodiscard]]
    std::string
    to_string(const http::response<Body, Fields>& r)
    {
        std::ostringstream ss;
        ss << r;

        return ss.str();
    }

}
