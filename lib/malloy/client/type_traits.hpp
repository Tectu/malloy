
#pragma once

#include <concepts>
#include <variant>

#include "malloy/type_traits.hpp"

namespace malloy::client::concepts {

    template<typename F>
    void filter_helper(const F& f) {
    }

template<typename F>
concept response_filter = std::move_constructible<F> && requires(const F& f, const typename F::header_type& h) {
    { f.body_for(h) } -> malloy::concepts::is_variant;
    {
        std::visit([](auto&& v) {
            F f2;
            typename F::header_type h2;
            typename std::decay_t<decltype(v)>::value_type r;
            f2.setup_body(h2, r);
            }, f.body_for(h)) 
    };
}; 


}
