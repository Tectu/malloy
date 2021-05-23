#pragma once

#include "malloy/websocket/types.hpp"

namespace malloy::server
{
    class route_websocket
    {
    public:
        std::string resource;
        malloy::websocket::handler_t handler;
    };
}
