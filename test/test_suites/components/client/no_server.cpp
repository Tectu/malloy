#include "../../../test.hpp"

#include <malloy/client/controller.hpp>

namespace mc = malloy::client;

TEST_SUITE("client - http")
{

    TEST_CASE("nonexisting host")
    {
        mc::controller::config cfg;
        cfg.logger = spdlog::default_logger();
        cfg.num_threads = 1;
        mc::controller ctrl{cfg};

        auto tkn = start(ctrl);

        // Make request
        auto stop_token = ctrl.http_request(
            malloy::http::method::get,
            "http://www.doesnotexistforsure2390124.com",
            [](auto&& resp) {
                // ToDo: Can we add a check that ensure that we never end up in here?
            }
        );
        const auto ec = stop_token.get();
        REQUIRE(ec);
    }

}
