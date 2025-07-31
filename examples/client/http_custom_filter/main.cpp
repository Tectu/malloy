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

    [[nodiscard]]
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

malloy::awaitable<void>
example()
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
	auto resp = co_await ctrl.http_request(
        malloy::http::method::get,
        "http://www.google.com",
        response_filter{ }
    );
    if (!resp)
		spdlog::error("error: {}", resp.error().message());

    // Show result
    // ToDo: Make this work again (broken since change from callback to coroutine)
#if 0
    if constexpr (std::same_as<std::decay_t<decltype(*resp)>, malloy::http::response<boost::beast::http::string_body>>)
        std::cout << *resp << std::endl;
    else
        std::cout << "Downloaded webpage to: " << download_path << std::endl;
#endif
}

// Include main() which will invoke the example() coroutine
#include "../client_example_main.hpp"
