#pragma once

namespace malloy::detail {
    /**
     * @class task 
     * @brief An eagerly started coroutine which returns no values
     */
    struct async_task {
      struct promise_type {
        auto get_return_object() -> async_task { return {}; }
        auto initial_suspend() -> std::suspend_never { return {}; }
        auto final_suspend() noexcept -> std::suspend_never { return {}; }
        void return_void() {}
        void unhandled_exception() {
            throw std::current_exception();
        }
      };
    };
}
