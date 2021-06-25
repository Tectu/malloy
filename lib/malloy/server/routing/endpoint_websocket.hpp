#pragma once

#include "endpoint.hpp"
#include "malloy/websocket/types.hpp"

namespace malloy::server
{
    namespace detail {
        template<typename T, typename H>
        concept has_handler = requires(T t, H h) { t.set_handler(h); };
    }
    class endpoint_websocket
    {
    public:
        virtual void bind_to(http::connection_t& conn) = 0;
        std::string resource;
    };

    template<typename Payload, typename Resp>
    class endpoint_websocket_impl : public endpoint_websocket {
    public:
        void bind_to(http::connection_t& conn) override {
            std::visit([this](auto& c) {
                if constexpr (detail::has_handler<decltype(*c), decltype(handler)>) {
                    c->set_handler(handler);
                }
                }, conn);
        }
        malloy::websocket::handler_t<Payload, Resp> handler;
    };
}
