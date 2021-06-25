#pragma once

#include <functional>
#include <string>

namespace malloy::websocket
{

    template<typename Payload>
    using writer_t  = std::function<void(Payload&&)>;

    template<typename Payload, typename Resp>
    using handler_t = std::function<void(Payload&&, writer_t<Resp>)>;
}
