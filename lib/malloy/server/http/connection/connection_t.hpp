
#pragma once

#include <variant>
#include <memory>



namespace malloy::server::http {
class connection_plain;

#if MALLOY_FEATURE_TLS

class connection_tls;

#endif
}
namespace malloy::server::websocket {
class connection_plain;

#if MALLOY_FEATURE_TLS 

class connection_tls;

#endif
}
namespace malloy::server::http {

using connection_t = std::variant<
    std::shared_ptr<connection_plain>,
    std::shared_ptr<malloy::server::websocket::connection_plain>
#if MALLOY_FEATURE_TLS 
    ,std::shared_ptr<connection_tls>
    ,std::shared_ptr<malloy::server::websocket::connection_tls>
#endif 
    >;
namespace detail {
template<typename... Args>
using req_gen_helper = std::variant<std::shared_ptr<Args::request_generator...>>;
}

using request_generator_t = detail::req_gen_helper<
    connection_plain,
    malloy::server::websocket::connection_plain,
#if MALLOY_FEATURE_TLS 
    ,connection_tls
    ,malloy::server::websocket::connection_tls
#endif 


}
