
#pragma once

#include <concepts>

namespace malloy::client::concepts {

template<typename F>
concept resp_filter = std::move_constructible<F> && requires(const F& f, const typename F::response_type::header_type& h, typename F::request_type::body_type::value_type& v) {
    f.setup_body(h, v);
};


}
