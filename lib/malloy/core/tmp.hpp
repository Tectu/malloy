
#pragma once

#include <type_traits>
#include <variant>

namespace malloy::tmp {
template<typename V>
using unwrap_variant = std::conditional_t<std::variant_size_v<V> == 1, std::variant_alternative_t<0, V>, V>;
}
