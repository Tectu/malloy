
#pragma once

#include <variant>
#include <memory>

#include "malloy/server/websocket/connection/connection.hpp"



namespace malloy::server::http {
class connection_plain;

#if MALLOY_FEATURE_TLS

class connection_tls;

#endif
}
namespace malloy::server::http {

using connection_t = std::variant<
    std::shared_ptr<connection_plain>
#if MALLOY_FEATURE_TLS 
    ,std::shared_ptr<connection_tls>
#endif 
    >;
}
