#pragma once

#include <functional>
#include <string>

namespace malloy::server::websocket
{
    using payload_type = std::string;
    using writer_type  = std::function<void(payload_type&&)>;
    using handler_type = std::function<void(payload_type, writer_type)>;
}
