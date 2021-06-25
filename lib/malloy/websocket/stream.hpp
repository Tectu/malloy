
#pragma once 

#include <variant>

#include <boost/beast/core.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/websocket/error.hpp>


#if MALLOY_FEATURE_TLS
	#include <boost/beast/ssl.hpp>
	#include <boost/asio/ssl/stream.hpp>
#endif

#include "malloy/type_traits.hpp"

namespace malloy::websocket {
	namespace detail {
#if MALLOY_FEATURE_TLS
		using tls_stream = boost::beast::websocket::stream<
			boost::beast::ssl_stream<boost::asio::ip::tcp::socket>
		>;
#endif
		using websocket_t = std::variant<
#if MALLOY_FEATURE_TLS
			tls_stream,
#endif
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
		stream(detail::websocket_t&& ws) : underlying_conn_{ std::move(ws) } {}

		stream(boost::beast::websocket::stream<boost::asio::ip::tcp::socket>&& ws) : underlying_conn_{std::move(ws)} {}
#if MALLOY_FEATURE_TLS
		stream(detail::tls_stream&& ws) : underlying_conn_{std::move(ws)} {}
#endif


		template<concepts::const_buffer_sequence Buff>
		auto write(const Buff& buffers) -> std::size_t {
			return std::visit([&buffers](auto& stream) mutable { return stream.write(buffers); }, underlying_conn_);
		}

		template<concepts::dynamic_buffer Buff>
		auto read(Buff& buff, boost::beast::error_code& ec) -> std::size_t {
			return std::visit([&buff, &ec](auto& s) mutable { return s.read(buff, ec); }, underlying_conn_);
		};

		auto close() -> boost::beast::websocket::close_reason {
			return std::visit([](auto& s) { boost::beast::websocket::close_reason ec; s.close(ec); return ec; }, underlying_conn_);
		}

		void set_option(auto opt) {
			std::visit([opt](auto& s) { s.set_option(opt);  }, underlying_conn_);
		}

		template<typename Body, typename Fields>
		auto accept(const boost::beast::http::request<Body, Fields>& req) -> boost::beast::error_code {
			return std::visit([req](auto& s) { return s.accept(req); }, underlying_conn_);
		}

		constexpr auto is_tls() -> bool { 
#if MALLOY_FEATURE_TLS
			return std::holds_alternative<detail::tls_stream>(underlying_conn_);  
#else 
			return false;
#endif
		
		}
		
	private:
		ws_t underlying_conn_;
				

	};

}
