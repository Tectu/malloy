#include <malloy/core/http/filters/file.hpp>
#include <malloy/client/controller.hpp>
#include <spdlog/spdlog.h>

namespace mc = malloy::client;

void log_error(malloy::error_code ec)
{
	if (ec)
		spdlog::error(ec.message());
}

int main()
{
    // Create controller configuration
	mc::controller::config cfg;
	cfg.logger = spdlog::default_logger();
	cfg.num_threads = 1;

	// Create controller
	mc::controller ctrl{cfg};
    [[maybe_unused]] auto session = start(ctrl);

	// Create request
	malloy::http::request req{
		malloy::http::method::get,
		"www.google.com",
		80,
		"/"
	};

	// Perform request (with file response filter)
	auto stop_token = ctrl.http_request(req, [](auto&&) {},
		malloy::http::filters::file_response::open("./google.com.html", log_error, boost::beast::file_mode::write));

	// Wait for completion and check for errors
	const auto ec = stop_token.get();
	if (ec) {
		spdlog::error(ec.message());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
