
#pragma once

#include <type_traits>
#include <variant>

namespace malloy::tmp {

/**
 * @brief Pattern:
 *  unwrap_variant<variant<T>> -> T
 *  unwrap_variant<variant<T, ...> -> std::variant<T, ...>
 *
 *  i.e. If its a 1 element it just returns that type, otherwise the entire variant type
 */
template<typename V>
using unwrap_variant = std::conditional_t<std::variant_size_v<V> == 1, std::variant_alternative_t<0, V>, V>;
}

/**
 * @namespace malloy::tmp
 * @brief Contains template metaprogramming helpers
 *
 */

