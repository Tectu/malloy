#pragma once

#include "endpoint.hpp"
#include "malloy/websocket/types.hpp"

namespace malloy::server
{
    struct endpoint_websocket
    {
        std::string resource;
        malloy::websocket::handler_t handler;
    };
}
