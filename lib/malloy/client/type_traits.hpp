
#pragma once

#include <concepts>
#include <variant>

#include "malloy/http/type_traits.hpp"
#include "malloy/type_traits.hpp"

namespace malloy::client::concepts
{
    namespace detail
    {
        template<typename F>
        struct response_filter_body_helper {
            template<malloy::http::concepts::body T>
            void operator()(T&& v)
            {
                F f2;
                typename F::header_type h2;
                typename std::decay_t<T>::value_type r;
                f2.setup_body(h2, r);
            }
        };
    }    // namespace detail

    template<typename F>
    concept response_filter = std::move_constructible<F> && requires(const F& f, const typename F::header_type& h)
    {
        {
            f.body_for(h)
            } -> malloy::concepts::is_variant;
        {
            std::visit(detail::response_filter_body_helper<F>{}, f.body_for(h))};
    };

}    // namespace malloy::client::concepts

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
