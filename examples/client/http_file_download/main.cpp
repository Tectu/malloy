#include <malloy/core/http/filters/file.hpp>
#include <malloy/client/controller.hpp>
#include <spdlog/spdlog.h>

void
log_error(malloy::error_code ec)
{
	if (ec)
		spdlog::error(ec.message());
}

int
main()
{
    // Create controller configuration
	malloy::client::controller::config cfg;
	cfg.logger      = spdlog::default_logger();
	cfg.num_threads = 1;

	// Create controller
	malloy::client::controller ctrl{cfg};

    // Start
    [[maybe_unused]] auto session = start(ctrl);

	// Make request (with file response filter)
	auto stop_token = ctrl.http_request(
        malloy::http::method::get,
        "http://www.google.com",
        [](auto&&) {},
		malloy::http::filters::file_response::open("./google.com.html", log_error, boost::beast::file_mode::write)
    );

	// Wait for completion and check for errors
	const auto ec = stop_token.get();
	if (ec) {
		spdlog::error(ec.message());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
