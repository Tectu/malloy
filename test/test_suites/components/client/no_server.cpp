#include "../../../test.hpp"
#include "../../../coroutine_executor.hpp"

#include <malloy/client/controller.hpp>

namespace mc = malloy::client;

TEST_SUITE("client - http")
{

    TEST_CASE("nonexisting host")
    {
        co_spawn([]() -> malloy::awaitable<void> {
            mc::controller::config cfg;
            cfg.logger = spdlog::default_logger();
            cfg.num_threads = 1;
            mc::controller ctrl{cfg};

            auto tkn = start(ctrl);

            // Make request
            auto resp = co_await ctrl.http_request(
                malloy::http::method::get,
                "http://www.doesnotexistforsure2390124.com");
            REQUIRE(!resp);
            //CHECK_EQ(resp.error(), malloy::error_code::host_unreachable); // ToDo
        });
    }

}
