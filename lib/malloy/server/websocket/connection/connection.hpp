#pragma once

#include "malloy/websocket/types.hpp"
#include "malloy/websocket/connection.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <spdlog/logger.h>

#include <future>
#include <queue>
#include <thread>

namespace malloy::server::websocket
{

    using connection = malloy::websocket::connection<false>;

}
