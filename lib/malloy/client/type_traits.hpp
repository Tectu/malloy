
#pragma once

#include <concepts>

#include "malloy/type_traits.hpp"

namespace malloy::client::concepts {

template<typename F>
concept resp_filter = std::move_constructible<F> && requires(const F& f, const typename F::header_type& h, typename F::value_type& v) {
    { f.body_for(h) } -> malloy::concepts::is_variant;
    { f.setup_body(h, v) };
}; 


}
