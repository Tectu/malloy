#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/beast/core/buffer_traits.hpp>

#include "malloy/error.hpp"

#include <concepts>
#include <variant>

namespace malloy::concepts
{
    namespace detail
    {
        struct is_variant_helper {
            template<typename... Ts>
            void operator()(const std::variant<Ts...>&) const {};
        };
    }    // namespace detail

    template<typename B>
    concept const_buffer_sequence = boost::asio::is_const_buffer_sequence<B>::value;

    template<typename Func>
    concept accept_handler = std::invocable<Func, malloy::error_code>;

    template<typename B>
    concept dynamic_buffer = boost::asio::is_dynamic_buffer<B>::value;

    template<typename Func>
    concept async_read_handler = std::invocable<Func, boost::beast::error_code, std::size_t>;

    template<typename V>
    concept is_variant = requires(V v)
    {
        detail::is_variant_helper{}(v);
    };

}    // namespace malloy::concepts

/** 
 * @page general_concepts Core Concepts
 * @section const_buffer_sequence 
 * @par alias for boost::asio::is_const_buffer_sequence 
 *
 * @section dynamic_buffer 
 * @par alias for boost::asio::is_dynamic_buffer
 * 
 * @section async_read_handler 
 * @par Handler type that has malloy::error_code and std::size_t as its
 * parameters
 *
 * @section accept_handler 
 * @par Handler type that has malloy::error_code as its only parameter 
 *
 * @section is_variant 
 * @par Requires that a type be a std::variant<...>
 *
 *
 */
