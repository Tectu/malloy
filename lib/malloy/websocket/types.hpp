#pragma once

#include <functional>
#include <string>

namespace malloy::websocket
{

    template<typename Payload = std::string>
    using writer_t  = std::function<void(Payload&&)>;

    template<typename Payload = std::string, typename Resp = std::string>
    using handler_t = std::function<void(Payload&&, writer_t<Resp>)>;
}
