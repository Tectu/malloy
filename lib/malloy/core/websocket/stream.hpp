#pragma once

#include "../type_traits.hpp"

#include <boost/beast/core.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/websocket/error.hpp>

#if MALLOY_FEATURE_TLS
	#include <boost/beast/ssl.hpp>
	#include <boost/asio/ssl/stream.hpp>
#endif

#include <variant>

namespace malloy::websocket
{

    namespace detail
    {

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
		explicit stream(detail::websocket_t&& ws) : m_underlying_conn{ std::move(ws) }
		{
		}

		explicit stream(boost::beast::websocket::stream<boost::beast::tcp_stream>&& s) : m_underlying_conn{ std::move(s) }
        {
        }

        explicit stream(boost::beast::tcp_stream&& from) :
            stream{boost::beast::websocket::stream<boost::beast::tcp_stream>{std::move(from)}}
        {
        }

#if MALLOY_FEATURE_TLS
		explicit stream(detail::tls_stream&& ws) : m_underlying_conn{std::move(ws)}
		{
		}

        explicit stream(boost::beast::ssl_stream<boost::beast::tcp_stream>&& from) :
            stream{malloy::websocket::detail::tls_stream{
                boost::beast::websocket::stream<
                    boost::beast::ssl_stream<boost::beast::tcp_stream>>{std::move(from)}}}
        {
        }
#endif

		template<concepts::const_buffer_sequence Buff, typename Callback>
		auto async_write(const Buff& buffers, Callback&& done)
		{
			return std::visit([&buffers, done = std::forward<Callback>(done)](auto& stream) mutable { return stream.async_write(buffers, std::forward<Callback>(done)); }, m_underlying_conn);
		}

		template<concepts::const_buffer_sequence Buff>
		auto write(const Buff& buffers) -> std::size_t
		{
			return std::visit([&buffers](auto& stream) mutable { return stream.write(buffers); }, m_underlying_conn);
		}

		template<concepts::dynamic_buffer Buff, typename Callback>
		auto async_read(Buff& buff, Callback&& done)
		{
			return std::visit([&buff, done = std::forward<Callback>(done)](auto& s) mutable { return s.async_read(buff, std::forward<Callback>(done)); }, m_underlying_conn);
		}

		template<concepts::dynamic_buffer Buff>
		auto read(Buff& buff, boost::beast::error_code& ec) -> std::size_t
		{
			return std::visit([&buff, &ec](auto& s) mutable { return s.read(buff, ec); }, m_underlying_conn);
		}
        template<typename Callback>
		auto async_close(boost::beast::websocket::close_reason why, Callback&& done) {
            return std::visit([why, done = std::forward<Callback>(done)](auto& s) mutable { 
                    return s.async_close(why, std::forward<Callback>(done));
            }, m_underlying_conn);
		}

		void set_option(auto&& opt)
		{
			std::visit([opt = std::forward<decltype(opt)>(opt)](auto& s) mutable { s.set_option(std::forward<decltype(opt)>(opt));  }, m_underlying_conn);
		}

		template<typename Body, typename Fields>
		auto async_accept(const boost::beast::http::request<Body, Fields>& req, concepts::accept_handler auto&& done)
		{
			std::visit([req, done = std::forward<decltype(done)>(done)](auto& s) mutable { return s.async_accept(req, std::forward<decltype(done)>(done)); }, m_underlying_conn);
		}

		template<typename Body, typename Fields>
		auto accept(const boost::beast::http::request<Body, Fields>& req) -> boost::beast::error_code
		{
			return std::visit([req](auto& s) { return s.accept(req); }, m_underlying_conn);
		}

		template<concepts::accept_handler Callback>
		void async_handshake(std::string host, std::string target, Callback&& done)
		{
			std::visit([host = std::move(host), target = std::move(target), done = std::forward<Callback>(done)](auto& s) mutable {
				s.async_handshake(host, target, std::forward<Callback>(done)); 
			}, m_underlying_conn);
		}

        /**
         * @brief Controls whether outgoing message will be indicated text or binary.
         *
         * @param enabled Whether to enable binary mode.
         *
         * @sa binary()
         */
        void set_binary(const bool enabled)
        {
            std::visit([enabled](auto& s) mutable {
                    s.binary(enabled);
                },
                m_underlying_conn
            );
        }

        /**
         * @brief Checks whether outgoing messages will be indicated as text or binary.
         * @return Whether messages are indicated as binary.
         *
         * @sa set_binary(bool)
         */
        [[nodiscard]]
        bool binary() const
        {
		    return std::visit([](auto& s) {
		            return s.binary();
		        },
                m_underlying_conn
            );
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
		void get_lowest_layer(Func&& visitor)
		{
			std::visit([visitor = std::forward<Func>(visitor)](auto& s) mutable { visitor(boost::beast::get_lowest_layer(s)); }, m_underlying_conn);
		}

        /**
         * @brief Get executor of the underlying stream
         * @return s.get_executor() where s is any of the types in
         * detail::websocket_t
         */
		auto get_executor()
		{
			return std::visit([](auto& s) { return s.get_executor(); }, m_underlying_conn);
		}

        /** 
         * @brief Whether the underlying stream is TLS or not 
         * @note Always false if MALLOY_FEATURE_TLS == 0
         */
		constexpr auto is_tls() const -> bool
		{
#if MALLOY_FEATURE_TLS
			return std::holds_alternative<detail::tls_stream>(m_underlying_conn);
#else 
			return false;
#endif
		}

#if MALLOY_FEATURE_TLS
		template<concepts::accept_handler Callback>
        void async_handshake_tls(boost::asio::ssl::stream_base::handshake_type type, Callback&& done) 
        {
			if (!is_tls())
                throw std::logic_error{"async_handshake_tls called on non-tls stream"};

            std::visit(
                [done = std::forward<Callback>(done), type](auto& s) mutable {
                    if constexpr (std::same_as<std::decay_t<decltype(s)>, detail::tls_stream>)
                        s.next_layer().async_handshake(type, std::forward<Callback>(done));
                },
                m_underlying_conn
            );
        }
		#endif

	private:
		ws_t m_underlying_conn;
	};

}
