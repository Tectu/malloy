#include <malloy/core/http/response.hpp>
#include <malloy/client/controller.hpp>

#include <iostream>

constexpr auto download_path = "./downloaded.html";

// Response filter
struct response_filter
{
	using response_type = std::variant<malloy::http::response<boost::beast::http::string_body>, malloy::http::response<boost::beast::http::file_body>>;
	using header_type = boost::beast::http::response_header<>;
	using value_type = std::variant<std::string, boost::beast::http::file_body::value_type>;

    std::variant<boost::beast::http::string_body, boost::beast::http::file_body>
    body_for(const header_type& header) const
    {
		return boost::beast::http::file_body{};
	}

	void
    setup_body(const header_type& header, auto& body) const
    {
		if constexpr (std::same_as<std::decay_t<decltype(body)>, boost::beast::http::file_body::value_type>) {
			malloy::error_code ec;
			body.open(download_path, boost::beast::file_mode::write, ec);
			if (ec)
				spdlog::error("failed to open download file: '{}'", ec.message());
		}
	}
};

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

	// Make request
	auto stop_token = ctrl.http_request(
        malloy::http::method::get,
        "http://www.google.com",
        [](auto&& resp) mutable {
            if constexpr (std::same_as<std::decay_t<decltype(resp)>, malloy::http::response<boost::beast::http::string_body>>)
                std::cout << resp << std::endl;
            else
                std::cout << "Downloaded webpage to: " << download_path << std::endl;
            },
        response_filter{ }
    );

	// Wait for completion and check for errors
	const auto ec = stop_token.get();
	if (ec) {
		spdlog::error("error: {}", ec.message());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
