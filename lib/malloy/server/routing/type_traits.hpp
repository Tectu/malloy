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

template<typename V>
concept is_variant = requires(V v) { 
    []<typename... Args>(const std::variant<Args...>& vs){}(v); // https://stackoverflow.com/q/68115853/12448530
};

template<typename F, typename Req>
concept route_handler =
	std::invocable<F, const Req&> ||
	std::invocable<F, const Req&,
				   const std::vector<std::string>>;


template<typename H>
concept advanced_route_handler = requires(const typename H::request_type::header_type& h, typename H::body_type::value_type& v)

// clang-format off
{
    { H::body_for(h) };
    { H::setup_body(h, v) };

// clang-format on
};
} // namespace malloy::server::concepts
