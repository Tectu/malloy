#pragma once

#include "error.hpp"

#include <boost/asio/buffer.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/core/buffer_traits.hpp>

#include <concepts>
#include <variant>

/**
 * @namespace malloy::concepts
 *
 * Namespace for concepts used throughout malloy.
 */
namespace malloy::concepts
{

    namespace detail
    {

        template<template<typename...> typename T>
        struct is_helper {
            template<typename... Ts>
            void operator()(const T<Ts...>&) const
            {}
        };


    }   // namespace detail

    template<typename B>
    concept const_buffer_sequence = boost::asio::is_const_buffer_sequence<B>::value;

    template<typename Func>
    concept accept_handler = std::invocable<Func, malloy::error_code>;

    template<typename Func>
    concept err_completion_token = boost::asio::completion_token_for<Func, void(malloy::error_code)>;


    template<typename B>
    concept dynamic_buffer = boost::asio::is_dynamic_buffer<B>::value;

    template<typename Func>
    concept async_read_handler = std::invocable<Func, boost::beast::error_code, std::size_t>;

    template<typename Func>
    concept read_completion_token = boost::asio::completion_token_for<Func, void(boost::beast::error_code, std::size_t)>;


    /**
     * Concept to check whether a type has a function with the following signature:
     *   - `std::string operator()()`
     *
     * @tparam T
     */
    template<typename T>
    concept callable_string = requires(T t) {
        { t() } -> std::same_as<std::string>;
        // { t() } -> std::same_as<std::string_view>;     // ToDo: Want to extend this to also accept std::string_view as return type.
    };

    namespace detail
    {
        /**
         * @brief Helper concept to transform a predicate into a concept
         * @tparam T Type to pass to the predicate
         * @tparam Pred Predicate to use. Must have a compile-time accessible field `value` which can be cast to `bool`
         */
        template<typename T, template<typename> typename Pred>
        concept sats_pred = static_cast<bool>(Pred<T>::value);

        /**
         * @brief Helper for the is_container_of_if<...> concept.
         * @details Essentially an expanded lambda of []<sats_pred<Cond>... Ts>(const A<Ts...>&) {}
         * @note Needed for clang 12 support
         * @tparam Cond the condition to use. Must be usable as a predicate for sats_pred
         */
        template<template<typename...> typename A, template<typename> typename Cond>
        struct is_container_of_helper {
            template<sats_pred<Cond>... Ts>
            void operator()(const A<Ts...>&) const
            {}
        };
        /**
         * @brief Predicate which is always true no matter the type passed to it
         * @tparam T
         */
        template<typename T>
        struct always_true {
            static constexpr bool value = true;
        };

    }
    template<typename T, template<typename...> typename Container, template<typename> typename Cond>
    concept is_container_of_if = requires(const T& v, const detail::is_container_of_helper<Container, Cond>& h)
    {
        h(v);
    };

    template<typename T, template<typename...> typename A>
    concept is = is_container_of_if<T, A, detail::always_true>;

    template<typename V>
    concept is_variant = is<V, std::variant>;

    namespace detail
    {
        template<template<typename...> typename A>
        struct is_a {
            template<typename T>
            struct type {
                static constexpr bool value = is<T, A>;
            };
        };
    }

    template<typename T, template<typename...> typename Contained, template<typename...> typename Container>
    concept is_container_of = is_container_of_if<T, Container, detail::is_a<Contained>::template type>;

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
 * @section is_container_of_if<T, Container<Ts...>, Cond<P>
 * @par Satisfied if T is a of type Container with elements that all satisfied Cond where Cond is a predicate with
 * compile time accessible `value` field that is static_cast'able to bool and `true` if satisfied.
 *
 * @section is<T, Container<...>>
 * @par Satisfied if T is of type Container with any inner types
 *
 * @section is_container_of<T, Contained<...>, Container<...>>
 * @par Satisfied if is<T, Container> and every element in T (Tn) satisfies is<Tn, Contained>
 *
 */
