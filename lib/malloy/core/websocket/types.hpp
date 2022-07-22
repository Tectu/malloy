#pragma once

#include <boost/beast/websocket/error.hpp>

namespace malloy::websocket
{
    /**
     * Websocket error codes.
     */
    using error = boost::beast::websocket::error;
}
