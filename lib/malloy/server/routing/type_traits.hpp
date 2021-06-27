#pragma once

#include <concepts> 
#include <variant>



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

template<typename H>
concept request_filter = std::move_constructible<H> && requires(const H& f, const typename H::request_type::header_type& h, typename H::request_type::body_type::value_type& v)

// clang-format off
{
    { f.setup_body(h, v) };
// clang-format on
};
} // namespace malloy::server::concepts
