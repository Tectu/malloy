
#pragma once

namespace malloy {
    /**
    * @brief Provides storage of types
    * @class type
    * @description Can be used to store types for later use or returning from functions, as an alternative to the actual type which may be expensive construct, move, etc
    */
    template<typename T>
    class type {
        using type = T;
    };
}
