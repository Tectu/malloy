#pragma once

#include "doctest.hpp"

/*
 * These includes get included by doctest.hpp. However, there is a note:
 *
 * "required includes - will go only in one translation unit!"
 *
 * which means that we have to include them again as we generate multiple translation units.
 */
#include <iostream>
#include <functional>
#include <malloy/client/controller.hpp>
#include <malloy/server/controller.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/co_spawn.hpp>


namespace malloy::test
{

#if MALLOY_TESTS_ENABLE_CORO
    using boost::asio::awaitable;

    inline void roundtrip_coro(
        const uint16_t port,
        std::function<awaitable<void>(malloy::client::controller&)> setup_client,
        std::function<void(malloy::server::controller&)> setup_server)
    {
        namespace mc = malloy::client;
        namespace ms = malloy::server;
        mc::controller c_ctrl;
        ms::controller s_ctrl;

        malloy::controller::config general_cfg;
        general_cfg.num_threads = 2;
        general_cfg.logger = spdlog::default_logger();

        ms::controller::config server_cfg{general_cfg};
        server_cfg.interface = "127.0.0.1";
        server_cfg.port = port;

        mc::controller::config cli_cfg{general_cfg};

        REQUIRE(s_ctrl.init(server_cfg));
        REQUIRE(c_ctrl.init(cli_cfg));

        setup_server(s_ctrl);

        REQUIRE(s_ctrl.start());
        CHECK(c_ctrl.start());

        boost::asio::io_context ioc;
        boost::asio::co_spawn(ioc, setup_client(c_ctrl), boost::asio::use_future);
        ioc.run();
        c_ctrl.stop().get();
    }
#endif


    /**
     * Performs a roundtrip using server & client components.
     *
     * @param port The port to perform the test on.
     * @param setup_client The client setup routine.
     * @param setup_server The server setup routine.
     */
    inline void roundtrip(
        const uint16_t port,
        std::function<void(malloy::client::controller&)> setup_client,
        std::function<void(malloy::server::controller&)> setup_server)
    {
        namespace mc = malloy::client;
        namespace ms = malloy::server;
        mc::controller c_ctrl;
        ms::controller s_ctrl;

        malloy::controller::config general_cfg;
        general_cfg.num_threads = 2;
        general_cfg.logger = spdlog::default_logger();

        ms::controller::config server_cfg{general_cfg};
        server_cfg.interface = "127.0.0.1";
        server_cfg.port = port;

        mc::controller::config cli_cfg{general_cfg};

        REQUIRE(s_ctrl.init(server_cfg));
        REQUIRE(c_ctrl.init(cli_cfg));

        setup_server(s_ctrl);

        REQUIRE(s_ctrl.start());

        CHECK(c_ctrl.start());

        setup_client(c_ctrl);
        c_ctrl.stop().get();
    }
}
