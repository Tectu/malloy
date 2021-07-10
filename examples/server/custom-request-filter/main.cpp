#include <boost/beast/http/file_body.hpp>
#include <malloy/core/http/request.hpp>
#include <malloy/server/controller.hpp>
#include <malloy/server/routing/router.hpp>

namespace ms = malloy::server;
namespace fs = std::filesystem;

constexpr auto download_path = "./downloads";

struct request_filter {
	using request_type = malloy::http::request<boost::beast::http::file_body>;
	using header_type = malloy::http::request_header<>;

	void setup_body(const header_type& header, typename request_type::body_type::value_type& file) const {
		auto path = std::filesystem::path{ download_path };
		path += malloy::http::resource_string(header);
		if (!fs::exists(path)) {
			fs::create_directories(path.parent_path());
		}
		malloy::error_code ec;
		file.open(path.string().c_str(), boost::beast::file_mode::write, ec);
		if (ec) {
			spdlog::error("Failed to open download path: '{}'", ec.message());
		}
	}

};

int main() {

	ms::controller ctrl;
	ms::controller::config cfg;
	cfg.num_threads = 2;
	cfg.interface = "0.0.0.0";
	cfg.port = 8080;
	cfg.logger = spdlog::default_logger();
	cfg.doc_root = "/";

	spdlog::set_level(spdlog::level::debug);


	if (!ctrl.init(cfg)) {
		spdlog::critical("Failed to init server controller");
		return EXIT_FAILURE;
	}

	ctrl.router()->add(malloy::http::method::post, "/.+", [](auto&&) {
		return malloy::http::generator::ok();
		}, request_filter{});

	if (!ctrl.start()) {
		spdlog::critical("Failed to start server");
		return EXIT_FAILURE;
	}

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

}
