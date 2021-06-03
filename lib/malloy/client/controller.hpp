#pragma once

#include "../controller.hpp"
#include "../http/request.hpp"
#include "../http/response.hpp"
#include "../websocket/types.hpp"

#include <spdlog/logger.h>

namespace malloy::client
{
    namespace websocket
    {
        class connection_plain;
    }

    /**
     * High-level controller for client activities.
     */
    class controller :
        public malloy::controller
    {
    public:
        struct config :
            malloy::controller::config
        {
        };

        controller() = default;
        ~controller() = default;

        /**
         * Issue an HTTP request.
         *
         * @tparam Connection The type of connection to use.
         * @param req The request to perform.
         * @return The corresponding response.
         */
        template<class Connection>
        [[nodiscard]]
        std::future<http::response>
        http_request(malloy::http::request req)
        {
            return std::async(
                std::launch::async,
                [req = std::move(req), this] {
                    http::response ret_resp;
                    std::atomic_bool done = false;      // ToDo: Use std::atomic_flag instead

                    // Create connection
                    auto conn = std::make_shared<Connection>(
                        m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                        io_ctx()
                    );

                    // Launch
                    conn->run(
                        std::to_string(req.port()).c_str(),
                        req,
                        [&ret_resp, &done](http::response&& resp){
                            ret_resp = std::move(resp);
                            done = true;
                        }
                    );

                    while (!done.load()) {
                        using namespace std::chrono_literals;
                        std::this_thread::sleep_for(5ms);
                    }

                    return ret_resp;
                }
            );
        }

        /**
         * Create a websocket connection.
         *
         * @tparam Connection The type of connection to use.
         * @param host The host.
         * @param port The port.
         * @param endpoint The endpoint.
         * @param handler Handler that gets called when data is received.
         *
         * @return The connection.
         */
        template<class Connection>
        [[nodiscard]]
        std::shared_ptr<Connection>
        make_websocket_connection(const std::string& host, std::uint16_t port, const std::string& endpoint, malloy::websocket::handler_t&& handler)
        {
            // Sanity check
            if (!handler)
                return { };

            // Create connection
            auto conn = std::make_shared<Connection>(m_cfg.logger->clone("connection"), io_ctx(), std::move(handler));

            // Launch the connection
            conn->connect(host, std::to_string(port), endpoint);

            return conn;
        }

        void test_tls();
    };

}
