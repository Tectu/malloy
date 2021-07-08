#pragma once

#include "malloy/websocket/connection.hpp"

namespace malloy::client::websocket
{

    /**
     * A client-side WebSocket connection.
     */
    using connection = malloy::websocket::connection<true>;

}
