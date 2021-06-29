#pragma once

#include "../../websocket/types.hpp"
#include "malloy/websocket/connection.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <spdlog/logger.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <queue>

namespace malloy::client::websocket
{

    /**
     * A client-side WebSocket connection.
     *
     */
    using connection = malloy::websocket::connection<true>;

}
