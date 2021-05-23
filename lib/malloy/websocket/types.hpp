#pragma once

#include <functional>
#include <string>

namespace malloy::websocket
{
    using payload_t = std::string;
    using writer_t  = std::function<void(payload_t&&)>;
    using handler_t = std::function<void(payload_t, writer_t)>;
}
