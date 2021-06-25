#pragma once

#include <boost/beast/core/error.hpp>
#include <boost/beast/core/buffer_traits.hpp>
#include <boost/asio/buffer.hpp>

#include <concepts>


namespace malloy::concepts {

template<typename B>
concept const_buffer_sequence = boost::asio::is_const_buffer_sequence<B>::value;

template<typename Func>
concept accept_handler = std::invocable<boost::beast::error_code>;

template<typename B>
concept dynamic_buffer = boost::asio::is_dynamic_buffer<B>::value;

template<typename Func>
concept async_read_handler = std::invocable<Func, boost::beast::error_code, std::size_t>;

}