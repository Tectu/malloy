#pragma once

#include <spdlog/logger.h>

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

    /**
     * Log to connection logger.
     *
     * @details This freestanding function allows logging to the connection logger.
     *
     * @tparam Args Arguments parameter pack.
     * @param conn The connection
     * @param level Log level.
     * @param fmt Format string.
     * @param args Arguments.
     */
    // ToDo: Maybe move this function to the top-level `malloy` namespace?
    // ToDo: Move fmt in lambda capture?
    // ToDo: Move args in lambda capture?
    template<typename ...Args>
    void
    log(
        const connection_t& conn,
        spdlog::level::level_enum level,
        fmt::format_string<Args...> fmt,
        Args&&... args
    )
    {
        std::visit(
            [level, &fmt, ...args = std::forward<Args>(args)](const auto& c) mutable {
                if (!c)
                    return;

                if (c->logger())
                    c->logger()->log(level, fmt, std::forward<Args>(args)...);
            },
            conn
        );
    }
}
