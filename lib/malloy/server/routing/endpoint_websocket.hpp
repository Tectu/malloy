#pragma once

#include "endpoint.hpp"
#include "malloy/websocket/types.hpp"
#include "malloy/server/websocket/connection/connection.hpp"

namespace malloy::server
{
    class endpoint_websocket
    {
    public:
        std::string resource;
        typename websocket::connection::handler_t handler;
    };
}
