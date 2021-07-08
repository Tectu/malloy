#pragma once

#include <variant> 

#include "connection.hpp"
#include "connection_plain.hpp"
#if MALLOY_FEATURE_TLS
    #include "connection_tls.hpp"
#endif

namespace malloy::server::http
{

    namespace detail
    {
        template<typename... Args>
        using req_gen_helper = std::variant<std::shared_ptr<typename Args::request_generator>...>;
    }

    using request_generator_t = detail::req_gen_helper<
        connection_plain
#if MALLOY_FEATURE_TLS
        ,connection_tls
#endif 
#ifdef MALLOY_INTERNAL_TESTING
        ,malloy::mock::http::connection
#endif
    >;

}
