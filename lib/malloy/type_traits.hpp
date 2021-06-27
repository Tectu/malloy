#pragma once

#include <boost/beast/core/buffer_traits.hpp>
#include <boost/asio/buffer.hpp>

#include "malloy/error.hpp"
#include "malloy/type.hpp"

#include <concepts>


namespace malloy::concepts {

template<typename B>
concept const_buffer_sequence = boost::asio::is_const_buffer_sequence<B>::value;

template<typename Func>
concept accept_handler = std::invocable<Func, malloy::error_code>;

template<typename B>
concept dynamic_buffer = boost::asio::is_dynamic_buffer<B>::value;

template<typename Func>
concept async_read_handler = std::invocable<Func, boost::beast::error_code, std::size_t>;

template<typename V>
concept is_variant = requires(V v) { 
    []<typename... Args>(const std::variant<Args...>& vs){}(v); // https://stackoverflow.com/q/68115853/12448530
};


template<typename V, template<typename> typename T>
concept is_variant_of = requires(const V& v) {
    [] <typename... Args>(const std::variant<T<Args>...>&) {}(v);
};

template<typename T>
concept type_container = requires() { { typename T::type } };

template<typename F>
concept resp_filter = std::move_constructible<F> && requires(const F & f, const typename F::response_type::header_type & h, typename F::request_type::body_type::value_type & v) {
    f.setup_body(h, v);
}

template<typename H>
concept route_filter = std::move_constructible<H> && requires(const H& f, const typename H::request_type::header_type& h, typename H::request_type::body_type::value_type& v)

// clang-format off
{
    { f.setup_body(h, v) };
// clang-format on
};

}