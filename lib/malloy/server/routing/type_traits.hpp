#pragma once

#include <concepts> 
#include <variant>

#include "malloy/core/type_traits.hpp"

namespace malloy::server::concepts
{
    namespace detail
    {
        template<typename T>
        concept route_handler_retr = malloy::concepts::is_container_of<T, malloy::http::response, std::variant> || malloy::concepts::is<T, malloy::http::response>;
        template<typename Func, typename... Args>
        concept route_handler_helper = std::invocable<Func, Args...> && requires(Func f, Args... args) {
            { std::invoke(f, args...) } -> route_handler_retr;
        };
    }

    template<typename F, typename Req>
    concept route_handler =
        detail::route_handler_helper<F, const Req&> ||
        detail::route_handler_helper<F, const Req&, const std::vector<std::string>>;

    template<typename Func>
    concept websocket_handler = std::invocable <Func, const malloy::http::request_header<>&, const std::shared_ptr<websocket::connection>&>;

    template<typename H>
    concept request_filter = std::move_constructible<H> && requires(const H& f, const typename H::request_type::header_type& h, typename H::request_type::body_type::value_type& v)

    // clang-format off
    {
        { f.setup_body(h, v) };
    // clang-format on
    };

    template<typename P>
    concept request_validator = requires(P p, const boost::beast::http::request_header<>& h) {
        { std::invoke(p, h) } -> malloy::concepts::is_container_of<malloy::http::response, std::optional>;
    };

} // namespace malloy::server::concepts

/**
 * @page route_concepts Route concepts 
 * @section route_handler 
 * @par A callback type for router::add. Requires either f(r) or f(r, v) where f is
 * the (non-const) type, r is a malloy::http::request<T> and v is
 * std::vector<std::string> holding the capture results of the regex expression.
 * 
 * @par If the capture group parameter is omitted the matches will not be
 * extracted from the input regex.
 *
 * @section request_filter 
 * @par A filter type for processing requests before they are passed onto the
 * handler. Must satisfy std::move_constructible and the expression:
 * `f.setup_body(h, v)` must be valid, where f is `const F&`, `F` is the filter
 * type, h is `const F::request_type::header_type&` and v is `F::request_type::body_type::value_type&`.
 *
 * @section websocket_handler 
 * @par A callback type for router::add_websocket. The expression `f(h, c)` must
 * be valid, where `f` is `T&`, h is `const malloy::http::request_header<>&` and
 * `c` is `const std::shared_ptr<malloy::server::websocket::connection>&`.
 * 
 */
