
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
	* @class stream
	* @brief Websocket stream. May use TLS
    * @details Provides an interface for different types of websocket streams,
    * allowing TLS and non-TLS streams to be used transparently. 
    * @note Not all of the interface has explicit documentation. You can assume
    * that anything without documentation simply calls the function of the same
    * name on the underlying stream
	*/
	class stream {
		using ws_t = detail::websocket_t;
	public:
		explicit stream(detail::websocket_t&& ws) : underlying_conn_{ std::move(ws) } {}

		explicit stream(boost::beast::websocket::stream<boost::beast::tcp_stream>&& s) : underlying_conn_{ std::move(s) } {}

#if MALLOY_FEATURE_TLS
		explicit stream(detail::tls_stream&& ws) : underlying_conn_{std::move(ws)} {}
#endif


		template<concepts::const_buffer_sequence Buff, concepts::async_read_handler Callback>
		void async_write(const Buff& buffers, Callback&& done)  {
			std::visit([&buffers, done = std::forward<Callback>(done)](auto& stream) mutable { return stream.async_write(buffers, std::forward<Callback>(done)); }, underlying_conn_);
		}
		template<concepts::const_buffer_sequence Buff>
		auto write(const Buff& buffers) -> std::size_t {
			return std::visit([&buffers](auto& stream) mutable { return stream.write(buffers); }, underlying_conn_);
		}

		template<concepts::dynamic_buffer Buff, concepts::async_read_handler Callback>
		void async_read(Buff& buff, Callback&& done)   {
			std::visit([&buff, done = std::forward<Callback>(done)](auto& s) mutable { s.async_read(buff, std::forward<Callback>(done)); }, underlying_conn_);
		};
		template<concepts::dynamic_buffer Buff>
		auto read(Buff& buff, boost::beast::error_code& ec) -> std::size_t {
			return std::visit([&buff, &ec](auto& s) mutable { return s.read(buff, ec); }, underlying_conn_);
		};

        template<concepts::accept_handler Callback>
		void async_close(boost::beast::websocket::close_reason why, Callback&& done) {
            std::visit([why, done = std::forward<Callback>(done)](auto& s) mutable { 
                    s.async_close(why, std::forward<Callback>(done));
            }, underlying_conn_);
		}

		void set_option(auto&& opt) {
			std::visit([opt = std::forward<decltype(opt)>(opt)](auto& s) mutable { s.set_option(std::forward<decltype(opt)>(opt));  }, underlying_conn_);
		}
		template<typename Body, typename Fields>
		auto async_accept(const boost::beast::http::request<Body, Fields>& req, concepts::accept_handler auto&& done) {
			std::visit([req, done = std::forward<decltype(done)>(done)](auto& s) mutable { return s.async_accept(req, std::forward<decltype(done)>(done)); }, underlying_conn_);
		}
		template<typename Body, typename Fields>
		auto accept(const boost::beast::http::request<Body, Fields>& req) -> boost::beast::error_code {
			return std::visit([req](auto& s) { return s.accept(req); }, underlying_conn_);
		}
		template<concepts::accept_handler Callback>
		void async_handshake(std::string host, std::string target, Callback&& done)  {
			std::visit([host = std::move(host), target = std::move(target), done = std::forward<Callback>(done)](auto& s) mutable {
				s.async_handshake(host, target, std::forward<Callback>(done)); 
			}, underlying_conn_);
		}

        /** 
         * @brief Access get_lowest_layer of wrapped stream type 
         * @param visitor Visitor function over `boost::beast::get_lowest_layer(t) for t in detail::websocket_t`
         * @details Example:
         * @code
         * void set_expires(stream& s) {
         *      s.get_lowest_layer([](auto& st) { st.expires_never(); });
         * }
         * @endcode
         */
		template<typename Func>
		void get_lowest_layer(Func&& visitor) {
			std::visit([vistor = std::forward<Func>(visitor)](auto& s) mutable { vistor(boost::beast::get_lowest_layer(s)); }, underlying_conn_);
		}

        /**
         * @brief Get executor of the underlying stream
         * @return s.get_executor() where s is any of the types in
         * detail::websocket_t
         */
		auto get_executor() {
			return std::visit([](auto& s) { return s.get_executor(); }, underlying_conn_);
		}

        /** 
         * @brief Whether the underlying stream is TLS or not 
         * @note Always false if MALLOY_FEATURE_TLS == 0
         */
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
