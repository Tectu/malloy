
#pragma once

namespace malloy::server {
    template<typename T>
    struct body_type {
        using value_type = T;

        body_type() = default;
    };
}
