#pragma once

#include <concepts> 
#include <variant>


#include "malloy/server/routing/body_type.hpp"


namespace malloy::server::concepts {
namespace detail {
    struct any_callable {
            template<typename T>
            void operator()(T&&) {}
    };
}

template<typename F, typename Req>
concept route_handler =
	std::invocable<F, const Req&> ||
	std::invocable<F, const Req&,
				   const std::vector<std::string>>;


template<typename Func>
concept websocket_handler = std::invocable <Func, const malloy::http::request_header<>&, const std::shared_ptr<websocket::connection>&>;

} // namespace malloy::server::concepts
