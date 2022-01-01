#pragma once

#include "malloy/core/tmp.hpp"
#include "malloy/core/http/response.hpp"
#include "malloy/core/type_traits.hpp"

namespace malloy::client::tmp {

namespace detail {
template<template<typename...> typename V, typename... Vargs>
struct conv_to_resp_helper {
    constexpr conv_to_resp_helper(V<Vargs...>) {}
    using type = V<http::response<Vargs>...>;
};
}

template<typename V>
requires (malloy::concepts::is_variant_of_bodies<V>)
using body_type = malloy::tmp::unwrap_variant<V>;

template<typename F>
using bodies_for_t = std::invoke_result_t<decltype(&F::body_for), const F*, const typename F::header_type&>;

template<typename Bodies>
requires (malloy::concepts::is_variant_of_bodies<Bodies>)
using to_responses = typename decltype(detail::conv_to_resp_helper{std::declval<Bodies>()})::type;

template<typename Filter>
using filter_resp_t = malloy::tmp::unwrap_variant<to_responses<bodies_for_t<Filter>>>;

};
