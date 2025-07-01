#pragma once

#include "type_traits.hpp"
#include "http/response.hpp"

#include <variant>
#include <type_traits>

/**
 * @namespace malloy::mp
 * @brief Namespace for metaprogramming utils
 */
namespace malloy::mp
{

    namespace detail
    {
        /// Helper to map variant<T...> to variant<response<T>...>
        template<
            template<typename...> typename V,
            typename... Vargs
        >
        struct conv_to_resp_helper
        {
            using type = V<http::response<Vargs>...>;

            explicit
            constexpr
            conv_to_resp_helper(V<Vargs...>)
            {
            }
        };
    }

    /**
     * @brief Pattern:
     *  unwrap_variant<variant<T>> -> T
     *  unwrap_variant<variant<T, ...> -> std::variant<T, ...>
     *
     *  i.e. If it is a 1 element it just returns that type, otherwise the entire variant type.
     */
    template<typename V>
    using unwrap_variant = std::conditional_t<std::variant_size_v<V> == 1, std::variant_alternative_t<0, V>, V>;

    /**
     * @brief Converts from a variant of possible bodies to the actual body type taken by callbacks
     */
    template<typename V>
    requires (malloy::concepts::is_variant_of_bodies<V>)
    using body_type = unwrap_variant<V>;

    /// Resolves the body type used by a filter F
    // ToDo: Concept for F
    template<typename F>
    using bodies_for_t = std::invoke_result_t<decltype(&F::body_for), const F*, const typename F::header_type&>;

    /// Converts a variant<T...> where T is body to a variant<response<T>...>
    template<typename Bodies>
    requires (malloy::concepts::is_variant_of_bodies<Bodies>)
    using to_responses = typename decltype(detail::conv_to_resp_helper{std::declval<Bodies>()})::type;

    /// Resolves to the type that must be taken in callbacks handling responses for Filter
    // ToDo: Concept for Filter
    template<typename Filter>
    using filter_resp_t = unwrap_variant<to_responses<bodies_for_t<Filter>>>;

}
