#include "../../example.hpp"

#include <boost/beast/http/file_body.hpp>
#include <malloy/core/http/request.hpp>
#include <malloy/server/routing_context.hpp>
#include <malloy/server/routing/router.hpp>

static const std::filesystem::path download_path = "./downloads";

struct request_filter
{
	using request_type = malloy::http::request<boost::beast::http::file_body>;
	using header_type = malloy::http::request_header<>;

	void setup_body(const header_type& header, typename request_type::body_type::value_type& file) const
	{
		auto path = std::filesystem::path{ download_path };
		path += malloy::http::resource_string(header);
		if (!std::filesystem::exists(path))
			std::filesystem::create_directories(path.parent_path());

		malloy::error_code ec;
		file.open(path.string().c_str(), boost::beast::file_mode::write, ec);
		if (ec)
			spdlog::error("Failed to open download path: '{}'", ec.message());
	}

};

int
main()
{
	malloy::server::routing_context::config cfg;
    setup_example_config(cfg);
	cfg.interface = "127.0.0.1";
	cfg.port = 8080;
	cfg.doc_root = "/";

	malloy::server::routing_context ctrl{cfg};

	ctrl.router().add(
        malloy::http::method::post,
        "/.+",
        [](auto&&) {
		    return malloy::http::generator::ok();
		},
        request_filter{ }
    );

    start(std::move(ctrl)).run();

    return EXIT_SUCCESS;
}
