#pragma once

namespace malloy::server
{

    /**
     * A generic endpoint.
     */
    struct endpoint
    {
        endpoint() = default;
        endpoint(const endpoint& other) = default;
        endpoint(endpoint&& other) noexcept = default;
        virtual ~endpoint() = default;

        endpoint& operator=(const endpoint& rhs) = default;
        endpoint& operator=(endpoint&& rhs) noexcept = default;
    };

}
