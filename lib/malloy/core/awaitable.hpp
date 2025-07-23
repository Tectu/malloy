#pragma once

#include <boost/asio/awaitable.hpp>

namespace malloy
{

    template<typename T>
    using awaitable = boost::asio::awaitable<T>;

}
