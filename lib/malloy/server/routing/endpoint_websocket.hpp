#pragma once

#include "endpoint.hpp"
#include "../websocket/connection.hpp"

namespace malloy::server
{
    class endpoint_websocket
    {
    public:
        std::string resource;
        typename websocket::connection::handler_t handler;
    };
}
