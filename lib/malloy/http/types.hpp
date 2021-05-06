#pragma once

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>

/**
 * @namespace malloy::http
 *
 * A namespace for everything related to HTTP.
 */
namespace malloy::http
{
    /**
     * The HTTP method.
     */
    using method = boost::beast::http::verb;

    /**
     * The HTTP status.
     */
    using status = boost::beast::http::status;

    /**
     * The HTTP fields.
     */
    using field = boost::beast::http::field;
}
