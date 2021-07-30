#pragma once 

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <queue>
#include <coroutine>

namespace malloy::detail {

/**
 * @class action_queue
 * @brief Stores and executes coroutine producing functions
 * @details The next queued function is not executed until the coroutine of the last function finishes
 * @note All methods in this class are threadsafe
 * @tparam Executor The executor to use
 */
template<typename Executor>
class action_queue {
    using act_t = std::function<boost::asio::awaitable<void>()>;
    using acts_t = std::queue<act_t>;
    using ioc_t = boost::asio::strand<Executor>;
public:
    /**
     * @brief Construct the action queue. It will not execute anything until run() is called
     * @param ioc The strand to use for synchronisation
     */
    explicit action_queue(ioc_t ioc) : ioc_{std::move(ioc)} {}

    /**
     * @brief Add an action to the queue
     * @note If run() has been called and the queue is currently empty, the new action will run immediately
     * @param act
     */
    void push(act_t act) {
        boost::asio::post(ioc_, [this, act = std::move(act)]() mutable -> void { 
            acts_.push(std::move(act));
            if (acts_.size() == 1 && running_) {
                run();
            }
        });
    }
    /**
     * @brief Starts the queue running, if it isn't already
     * @note Only needs to be called once
     */
    void run() {
        running_ = true;
        boost::asio::co_spawn(ioc_, exe_next(), boost::asio::use_future);
    }


private:
    auto exe_next() -> boost::asio::awaitable<void> {
        if (!acts_.empty()) {
            co_await boost::asio::post(ioc_, boost::asio::use_awaitable);
            auto act = std::move(acts_.front());
            co_await std::invoke(std::move(act));
            assert(!acts_.empty());
            acts_.pop();
            co_await exe_next();
        }
        
    }

    ioc_t ioc_;
    acts_t acts_;
    std::atomic_bool running_{false};

};

}



