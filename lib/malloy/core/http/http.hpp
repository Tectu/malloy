#pragma once

#include "types.hpp"

/**
 * @namespace malloy::http
 *
 * A namespace for everything related to HTTP.
 */
namespace malloy::http
{
}


namespace malloy
{

    /**
     * Convert HTTP method/verb to string.
     *
     * @param method The method.
     * @return String representation.
     */
    [[nodiscard]]
    inline
    std::string
    to_string(const malloy::http::method method)
    {
        return boost::beast::http::to_string(method);
    }

}