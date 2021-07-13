#pragma once

#include "error.hpp"

#include <boost/asio/buffer.hpp>
#include <boost/beast/core/buffer_traits.hpp>

#include <concepts>
#include <variant>

namespace malloy::concepts
{
    namespace detail
    {
        /**
         * @brief Helper for the is<...> concept.
         * @details Essentially an expanded lambda []<typename... Ts>(const T<Ts..>&){}
         * @note Needed for clang 12 support
         * @tparam T The type to check against
         */
        template<template<typename...> typename T>
        struct is_helper
        {
            template<typename... Ts>
            void operator()(const T<Ts...>&) const {}
        };



    }    // namespace detail

    template<typename T, template<typename...> typename A>
    concept is = requires(const T& t, const detail::is_helper<A>& h) {
        h(t);
    };

    template<typename B>
    concept const_buffer_sequence = boost::asio::is_const_buffer_sequence<B>::value;

    template<typename Func>
    concept accept_handler = std::invocable<Func, malloy::error_code>;

    template<typename B>
    concept dynamic_buffer = boost::asio::is_dynamic_buffer<B>::value;

    template<typename Func>
    concept async_read_handler = std::invocable<Func, boost::beast::error_code, std::size_t>;

    template<typename V>
    concept is_variant = is<V, std::variant>;

    namespace detail {
        template<typename T, template<typename> typename Pred>
        concept sats_pred = static_cast<bool>(Pred<T>::value);

        template<template<typename...> typename A, template<typename> typename Cond>
        struct is_container_of_helper {
            template<sats_pred<Cond>... Ts>
            void operator()(const A<Ts...>&) const {}
        };
        template<template<typename...> typename A>
        struct is_a {
            template<typename T>
            struct type {
                static constexpr bool value = is<T, A>;
            };
        };
    }

    template<typename T, template<typename...> typename Container, template<typename> typename Cond>
    concept is_container_of_if = requires(const T& v, const detail::is_container_of_helper<Container, Cond>& h) {
        h(v);
    };
    template<typename T, template<typename...> typename Contained, template<typename...> typename Container>
    concept is_container_of = is_container_of_if<T, Container, typename detail::is_a<Contained>::type>;

    template<typename T>
    concept is_variant_of_bodies = is_container_of_if<T, std::variant, boost::beast::http::is_body>;

    static_assert(is_container_of<std::variant<std::tuple<std::string>, std::tuple<int>>, std::tuple, std::variant>, "is_container_of is defective");

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
