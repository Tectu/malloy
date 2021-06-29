
#include <malloy/client/controller.hpp>
#include <malloy/http/response.hpp>

namespace mc = malloy::client;

constexpr auto download_path = "./downloaded.html";

struct response_filter {
	using response_type = std::variant<malloy::http::response<boost::beast::http::string_body>, malloy::http::response<boost::beast::http::file_body>>;
	using header_type = boost::beast::http::response_header<>;
	using value_type = std::variant<std::string, boost::beast::http::file_body::value_type>;

	auto body_for(const header_type& header) const -> std::variant<boost::beast::http::string_body, boost::beast::http::file_body> {
		return boost::beast::http::file_body{};
	}
	void setup_body(const header_type& header, auto& body) const {
		if constexpr (std::same_as<std::decay_t<decltype(body)>, boost::beast::http::file_body::value_type>) {
			malloy::error_code ec;
			body.open(download_path, boost::beast::file_mode::write, ec);
			if (ec) {
				spdlog::error("failed to open download file: '{}'", ec.message());
			}
		}
	}
};

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
	auto stop_token = ctrl.http_request(req, [](auto&& req) mutable {
		if constexpr (std::same_as<std::decay_t<decltype(req)>, malloy::http::response<boost::beast::http::string_body>>) {
			std::cout << req << '\n';
		}
		else {
			std::cout << "Downloaded webpage to: " << download_path << '\n';
		}
		}, response_filter{});

	const auto ec = stop_token.get();
	if (ec) {
		spdlog::error(ec.message());
		return EXIT_FAILURE;
	}
}
