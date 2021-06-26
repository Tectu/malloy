
#include "../../test.hpp"


#include <malloy/client/controller.hpp>
#include <malloy/server/controller.hpp>
#include <malloy/utils.hpp>
#include <malloy/server/routing/router.hpp>

namespace mc = malloy::client;
namespace ms = malloy::server;

namespace {
	constexpr auto server_msg = "Hello from server";
	constexpr auto cli_msg = "Hello from client";
}



TEST_SUITE("websockets") {

	TEST_CASE("roundtrip") {
		mc::controller c_ctrl;
		ms::controller s_ctrl;

		constexpr auto port = 501231;
		malloy::controller::config general_cfg;
		general_cfg.num_threads = 2;
		general_cfg.logger = spdlog::default_logger();

		ms::controller::config server_cfg{ general_cfg };
		server_cfg.interface = "127.0.0.1";
		server_cfg.port = port;

		CHECK(s_ctrl.init(server_cfg));
		CHECK(c_ctrl.init(general_cfg));

		bool server_recieved = false;
		std::promise<void> pstop;
		auto stop_token = pstop.get_future();

		s_ctrl.router()->add_websocket("/", [&server_recieved, pstop = &pstop](const auto& req, auto conn) mutable {
			server_recieved = true;
			conn->accept(req, [conn, pstop]() mutable {
				conn->send(malloy::buffer(server_msg, std::strlen(server_msg)), [conn, pstop](auto ec, auto size) mutable {
					CHECK(!ec);
					CHECK(size == std::strlen(server_msg));
					auto buffer = std::make_shared<boost::beast::flat_buffer>();
					conn->read(*buffer, [buffer, pstop](auto ec, auto size) mutable {
						CHECK(!ec);
						CHECK(size == std::strlen(cli_msg));
						
						CHECK(malloy::buffers_to_string(buffer->cdata()) == cli_msg);

						assert(pstop != nullptr);
						pstop->set_value();
						});
					});
				});
			
			});
		
		c_ctrl.make_websocket_connection("127.0.0.1", port, "/", [&](auto ec, auto conn) {
			CHECK(!ec);
			CHECK(conn);

			auto buffer = std::make_shared<boost::beast::flat_buffer>();
			conn->read(*buffer, [&, conn, buffer](auto ec, auto size) {
				CHECK(!ec);
				CHECK(size == std::strlen(server_msg));

				CHECK(malloy::buffers_to_string(buffer->cdata()) == server_msg);

				conn->send(malloy::buffer(cli_msg, std::strlen(cli_msg)), [&](auto ec, auto size) {
					CHECK(!ec);
					CHECK(size == std::strlen(cli_msg));
					});
				}
			);
			});

		s_ctrl.start();
		c_ctrl.start();

		stop_token.wait();
		CHECK(server_recieved);





	}


}
