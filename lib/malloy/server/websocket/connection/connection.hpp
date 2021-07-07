#pragma once

#include "malloy/websocket/connection.hpp"

namespace malloy::server::websocket
{

    /**
     * A server-side WebSocket connection.
     */
    using connection = malloy::websocket::connection<false>;

}
