#pragma once

#include "../type_traits.hpp"
#include "../../core/error.hpp"
#include "../../core/mp.hpp"

#include <expected>

// ToDo: This file is no longer necessary, move the type declaration to somewhere else

namespace malloy::client::http
{

    /**
     * Result of an HTTP request
     */
    template<concepts::response_filter Filter>
    using request_result = std::expected< malloy::mp::filter_resp_t<Filter>, malloy::error_code >;

}
