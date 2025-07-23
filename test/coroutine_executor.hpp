#pragma once

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>

/**
 * Used to invoke a coroutine and optain the result.
 *
 * This is needed because we can't make TEST_CASE or SUBCASE a coroutine.
 */
template<typename F, typename... Args>
void
co_spawn(F&& f, Args&&... args)
{
    boost::asio::io_context ioc;

    boost::asio::co_spawn(
        ioc,
        f(std::forward<Args>(args)...),
        boost::asio::use_future
    );

    ioc.run();
}
