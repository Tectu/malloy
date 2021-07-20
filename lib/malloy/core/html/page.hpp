#pragma once

#include "../http/response.hpp"

namespace malloy::html
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
        std::string
        render() const = 0;

        [[nodiscard]]
        malloy::http::response<>
        response() const;
    };

}
