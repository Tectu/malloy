
#pragma once 

#include <variant>

#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "malloy/type_traits.hpp"

namespace malloy::websocket {
	namespace detail {
		using tls_stream = boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>;
		using websocket_t = std::variant<
			tls_stream,
			boost::beast::websocket::stream<boost::asio::ip::tcp::socket>
		>;

	}
	/**
	* @brief Websocket stream. May use TLS
	* @class stream
	*/
	class stream {
		using ws_t = detail::websocket_t;
	public:


		template<concepts::const_buffer_sequence Buff>
		auto write(const Buff& buffers) -> std::size_t {
			return std::visit([&buffers](auto& stream) { return stream.write(buffers); }, underlying_conn_);
		}

		template<concepts::dynamic_buffer Buff>
		auto read();

		void set_option(auto opt) {
			std::visit([opt](auto& s) { s.set_option(opt);  }, underlying_conn_);
		}

		template<typename Body, typename Fields>
		auto accept(const boost::beast::http::request<Body, Fields>& req) -> boost::beast::error_code {
			return std::visit([done = std::forward<decltype(done)>(done), req](auto& s) { return s.accept(req); }, underlying_conn_);
		}

		constexpr auto uses_tls() -> bool { return std::holds_alternative<detail::tls_stream>(underlying_conn_);  }
		
	private:
		ws_t underlying_conn_;
				

	};

}
