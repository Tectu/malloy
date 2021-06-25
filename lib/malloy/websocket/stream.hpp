
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
			boost::beast::ssl_stream<boost::beast::tcp_stream>
		>;
#endif
		using websocket_t = std::variant<
#if MALLOY_FEATURE_TLS
			tls_stream,
#endif
			boost::beast::websocket::stream<boost::beast::tcp_stream>
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

		stream(boost::beast::websocket::stream<boost::beast::tcp_stream>&& s) : underlying_conn_{ std::move(s) } {}

#if MALLOY_FEATURE_TLS
		stream(detail::tls_stream&& ws) : underlying_conn_{std::move(ws)} {}
#endif


		template<concepts::const_buffer_sequence Buff>
		void write_asnyc(const Buff& buffers, const concepts::async_read_handler auto& done)  {
			std::visit([&buffers, done](auto& stream) mutable { return stream.write_async(buffers, done); }, underlying_conn_);
		}
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
		auto async_accept(const boost::beast::http::request<Body, Fields>& req, const concepts::accept_handler auto& done) {
			std::visit([req, done](auto& s) { return s.async_accept(req, done); }, underlying_conn_);
		}
		template<typename Body, typename Fields>
		auto accept(const boost::beast::http::request<Body, Fields>& req) -> boost::beast::error_code {
			return std::visit([req](auto& s) { return s.accept(req); }, underlying_conn_);
		}
		template<concepts::accept_handler Callback>
		void async_handshake(std::string_view host, std::string_view target, Callback&& done)  {
			std::visit([done = std::forward<Callback>(done)](auto& s) { s.async_handshake(host, target, std::forward<Callback>(done)); }, underlying_conn_);
		}

		template<typename Func>
		void get_lowest_layer(Func&& visitor) {
			std::visit([vistor = std::forward<Func>(visitor)](auto& s) { vistor(boost::beast::get_lowest_layer(s)); }, underlying_conn_);
		}

		auto get_executor() {
			return std::visit([](auto& s) { return s.get_executor(); }, underlying_conn_);
		}

		constexpr auto is_tls() const -> bool { 
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
