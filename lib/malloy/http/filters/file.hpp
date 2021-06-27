
#pragma once

#include "malloy/http/response.hpp"
#include "malloy/http/request.hpp"

#include <filesystem>
#include <variant>

namespace malloy::http::filters {

template<bool isRequest>
struct basic_file {
	using response_type = malloy::http::response<boost::beast::http::file_body>;
	using request_type = malloy::http::request<boost::beast::http::file_body>;
	using value_type = boost::beast::http::file_body::value_type;
	using header_type = boost::beast::http::header<isRequest>;

	using setup_handler_t = std::function<void(const header_type&, value_type&)>;

	setup_handler_t setup;

	basic_file() = default;
	explicit basic_file(setup_handler_t setup_) : setup{ std::move(setup_) } {}

	basic_file(basic_file&&) noexcept = default;
	basic_file& operator=(basic_file&&) noexcept = default;

	static auto open(
		const std::filesystem::path& location, 
		std::function<void(malloy::error_code)> on_error, 
		boost::beast::file_mode mode) -> basic_file {
		return basic_file{ [location, mode, on_error = std::move(on_error)](auto&&, auto& body) {
			boost::beast::error_code ec;
			body.open(location.string().c_str(), mode, ec);
			if (ec && on_error) {
				on_error(ec);
			}
		} };

	}

	auto body_for(const header_type&) const -> std::variant<boost::beast::http::file_body> {
		return {};
	}

	void setup_body(const header_type& h, value_type& body) const {
		if (setup) {
			setup(h, body);
		}
	}
};

using file_request = basic_file<true>;
using file_response = basic_file<false>;

}
