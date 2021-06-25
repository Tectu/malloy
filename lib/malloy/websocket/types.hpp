#pragma once

#include <functional>
#include <string>

namespace malloy::websocket
{

    template<typename Payload = std::string>
    using writer_t  = std::function<void(Payload&&)>;
}
