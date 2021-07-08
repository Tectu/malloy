#pragma once

#include "../../core/websocket/connection.hpp"

namespace malloy::server::websocket
{

    /**
     * A server-side WebSocket connection.
     */
    using connection = malloy::websocket::connection<false>;

}
