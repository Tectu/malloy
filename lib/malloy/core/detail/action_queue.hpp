#pragma once 

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

#include <queue>
#include <coroutine>

namespace malloy::detail {

class action_queue {
    using act_t = net::awaitable<void>;
    using acts_t = std::queue<act_t>;
public:
    explicit act_queue(net::io_context& ioc) : ioc_{ioc} {}

    void push(act_t act) {
        boost::asio::post(ioc_, [this, act = std::move(act)]() mutable -> void { 
            acts_.push(std::move(act));
            if (acts_.size() == 1) {
                boost::asio::co_spawn(ioc_, exe_next(), boost::asio::use_future);
            }
        });
    }


private:
    auto exe_next() -> net::awaitable<void> {
        if (!acts_.empty()) {
            co_await net::post(ioc_, net::use_awaitable);
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



