#pragma once 

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>

#include <queue>
#include <coroutine>

namespace malloy::detail {

class action_queue {
    using act_t = boost::asio::awaitable<void>;
    using acts_t = std::queue<act_t>;
public:
    explicit action_queue(boost::asio::io_context& ioc) : ioc_{ioc} {}

    void push(act_t act) {
        boost::asio::post(ioc_, [this, act = std::move(act)]() mutable -> void { 
            acts_.push(std::move(act));
            if (acts_.size() == 1) {
                boost::asio::co_spawn(ioc_, exe_next(), boost::asio::use_future);
            }
        });
    }


private:
    auto exe_next() -> act_t {
        if (!acts_.empty()) {
            co_await boost::asio::post(ioc_, boost::asio::use_awaitable);
            auto act = std::move(acts_.front());
            co_await std::move(act);
            assert(!acts_.empty());
            acts_.pop();
            co_await exe_next();
        }
        
    }

    boost::asio::io_context& ioc_;
    acts_t acts_;


};

}



