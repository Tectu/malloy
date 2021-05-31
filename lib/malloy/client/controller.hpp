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

        template<class Connection>
        [[nodiscard]]
        std::future<http::response>
        http_request(malloy::http::request req)
        {
            // ToDo: This can maybe be rewritten to use coroutines
            http::response ret_resp;

            return std::async(
                std::launch::async,
                [req = std::move(req), &ret_resp, this] {
                    std::atomic_bool done = false;      // ToDo: Use std::atomic_flag instead
                    // Create connection
                    auto conn = std::make_shared<Connection>(
                        m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                        io_ctx()
                    );

                    // Launch
                    const std::string& port_str = std::to_string(req.port()); // Keep the string alive
                    conn->run(
                        port_str.c_str(),
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
