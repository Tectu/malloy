#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>

#include <spdlog/spdlog.h>

/**
 * Main function invoking a coroutine provided by the individual example.
 */
int
main()
{
    boost::asio::io_context ioc;

    boost::asio::co_spawn(
        ioc,
        example(),
        [](const std::exception_ptr& e) {
            try {
                if (e)
                    std::rethrow_exception(e);
            }
            catch (const std::exception& e) {
                spdlog::error("exception: {}", e.what());
            }
        }
    );

    ioc.run();
}
