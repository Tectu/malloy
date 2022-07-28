#pragma once

#include <variant>
#include <memory>

namespace malloy::server::http
{
    class connection_plain;

#if MALLOY_FEATURE_TLS
    class connection_tls;
#endif

    /**
     * Type to hold either a plain connection or a TLS connection.
     */
    using connection_t = std::variant<
        std::shared_ptr<connection_plain>
#if MALLOY_FEATURE_TLS 
        ,std::shared_ptr<connection_tls>
#endif 
    >;
}
