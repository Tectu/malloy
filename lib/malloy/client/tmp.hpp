#pragma once

#include "malloy/core/tmp.hpp"
#include "malloy/core/http/response.hpp"
#include "malloy/core/type_traits.hpp"

namespace malloy::client::tmp {

namespace detail {
/// Helper to map variant<T...> to variant<response<T>...>
template<template<typename...> typename V, typename... Vargs>
struct conv_to_resp_helper {
    constexpr conv_to_resp_helper(V<Vargs...>) {}
    using type = V<http::response<Vargs>...>;
};
}

/**
 * @brief Converts from a variant of possible bodies to the actual body type taken by callbacks
 */
template<typename V>
requires (malloy::concepts::is_variant_of_bodies<V>)
using body_type = malloy::tmp::unwrap_variant<V>;

/// Resolves the body type used by a filter F
template<typename F>
using bodies_for_t = std::invoke_result_t<decltype(&F::body_for), const F*, const typename F::header_type&>;

/// Converts a variant<T...> where T is body to a variant<response<T>...>
template<typename Bodies>
requires (malloy::concepts::is_variant_of_bodies<Bodies>)
using to_responses = typename decltype(detail::conv_to_resp_helper{std::declval<Bodies>()})::type;

/// Resolves to the type that must be taken in callbacks handling responses for Filter
template<typename Filter>
using filter_resp_t = malloy::tmp::unwrap_variant<to_responses<bodies_for_t<Filter>>>;

};
