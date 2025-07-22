#pragma once

#include "../../core/http/response.hpp"

#include <span>

namespace malloy::server::http
{

    struct preflight_config
    {
        std::string origin = "http://127.0.0.1:8080";

        /**
         * Setup a response.
         *
         * @warning The lifetime of `this` must exceed the lifetime of the passed parameters.
         *
         * @param resp The response to setup.
         * @param methods List of method strings supported.
         */
        // ToDo: Add template for response body
        void
        setup_response(
            malloy::http::response<>& resp,
            std::span<malloy::http::method> methods
        ) const;
    };

}
