#pragma once

#include <boost/beast/http/message.hpp>
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
     * The HTTP field.
     */
    using field = boost::beast::http::field;

    /**
     * The HTTP fields
     */
    using fields = boost::beast::http::fields;

    /**
     * HTTP request header.
     */
    template<typename Fields = fields>
    using request_header = boost::beast::http::request_header<Fields>;

    /**
     * HTTP response header.
     */
    template<typename Fields = fields>
    using response_header = boost::beast::http::response_header<Fields>;
}
