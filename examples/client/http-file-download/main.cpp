
#include <malloy/client/controller.hpp>
#include <malloy/http/filters/file.hpp>

#include <spdlog/spdlog.h>

namespace mc = malloy::client;

void log_error(malloy::error_code ec) {
	if (ec) {
		spdlog::error(ec.message());
	}
}

int main() {
	mc::controller::config cfg;
	cfg.logger = spdlog::default_logger();
	cfg.num_threads = 1;

	mc::controller ctrl;
	if (!ctrl.init(cfg) || !ctrl.start()) {
		spdlog::critical("Failed to init controller");
		return EXIT_FAILURE;
	}

	malloy::http::request req{
		malloy::http::method::get,
		"www.google.com",
		80,
		"/"
	};
	auto stop_token = ctrl.http_request(req, [](auto&&) {}, 
		malloy::http::filters::file_response::open("./google.com.html", log_error, boost::beast::file_mode::write));

	const auto ec = stop_token.get();
	if (ec) {
		spdlog::error(ec.message());
		return EXIT_FAILURE;
	}
		

}

