#pragma once

#include "../../core/http/response.hpp"

#include <string>

namespace malloy::server::app_fw
{

    /**
     * A type to represent a single HTML page.
     */
    class page
    {
    public:
        page() = default;
        page(const page&) = default;
        page(page&&) noexcept = default;
        virtual ~page() noexcept = default;

        page& operator=(const page&) = default;
        page& operator=(page&&) noexcept = default;

        [[nodiscard]]
        virtual
        malloy::http::response<>
        render() const = 0;
    };

}
