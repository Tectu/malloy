#pragma once

#include "../type_traits.hpp"
#include "../../core/error.hpp"
#include "../../core/mp.hpp"

namespace malloy::client::http
{

    /**
     * Result of an HTTP request
     */
    // ToDo: Instead of this function, should we return a pair or tuple so we can write:  auto [ec, resp] = co_await foo();  ?
    template<concepts::response_filter Filter>
    struct request_result
    {
        /**
         * The error code.
         */
        malloy::error_code error_code;

        /**
         * The HTTP response.
         *
         * @details If the filter has only one body, this type is the response itself. If the filter has more than one
         *          body, this type is an std::variant<> of all bodies.
         *
         * @note The value of this is invalid/undefined if the error-code is true.
         */
        malloy::mp::filter_resp_t<Filter> response;

        request_result() = default;

        explicit
        request_result(malloy::error_code ec) :
            error_code{ ec }
        {
        }
    };

}
