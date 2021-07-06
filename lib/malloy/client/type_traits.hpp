
#pragma once

#include <concepts>
#include <variant>

#include "malloy/type_traits.hpp"

namespace malloy::client::concepts {
    namespace detail
    {
        template<typename Callback>
        struct http_cb_helper {
            Callback cb;

            template<typename V>
            void operator()(V&& v)
            {
                using body_t = std::decay_t<V>;
                malloy::http::response<body_t> res;

                cb(std::move(res));
            }
        
        };
    
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

    template<typename F, typename Filter>
    concept http_callback = response_filter<Filter>&& std::move_constructible<F>&& requires(F cb, const Filter& f, const typename Filter::header_type& h){
        std::visit(detail::http_cb_helper<F>{std::move(cb)}, f.body_for(h));
    };

}

/** 
 * @page client_concepts Client Concepts 
 *
 * @section response_filter 
 * @par Filter type used to setup responses before being passed to the handler
 * of malloy::client::controller::http_request (or https_request). 
 *
 * @par Requires:
 * - `std::move_constructible`
 * - `f.body_for(h) -> std::variant<Ts...>`
 * - `std::visit([](auto& v){ decltype(v)::value_type r; f.setup_body(h, r); }, f.body_for(h))` 
 *   (setup_body must be a visitor over the value_types of the response bodies returned by `f.body_for(h)`)
 *
 *
 */
