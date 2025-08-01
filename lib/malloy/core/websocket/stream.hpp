#pragma once

#include "../awaitable.hpp"
#include "../tcp/stream.hpp"
#include "../type_traits.hpp"

#include <boost/beast/core.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/ip/basic_resolver_results.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/websocket/error.hpp>

#if MALLOY_FEATURE_TLS
	#include <boost/beast/ssl.hpp>
	#include <boost/asio/ssl/stream.hpp>
#endif

#include <coroutine>
#include <expected>
#include <variant>

namespace malloy::websocket
{

    namespace detail
    {
        using plain_stream = boost::beast::websocket::stream<malloy::tcp::stream<>>;

#if MALLOY_FEATURE_TLS
		using tls_stream = boost::beast::websocket::stream<
			boost::beast::ssl_stream<malloy::tcp::stream<>>
		>;
#endif

		using websocket_t = std::variant<
#if MALLOY_FEATURE_TLS
			tls_stream,
#endif
			plain_stream
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
	class stream
    {
		using ws_t = detail::websocket_t;

	public:
		explicit
        stream(detail::websocket_t&& ws) :
            m_underlying_conn{ std::move(ws) }
		{
		}

		explicit
        stream(boost::beast::websocket::stream<malloy::tcp::stream<>>&& s) :
            m_underlying_conn{ std::move(s) }
        {
        }

        explicit
        stream(malloy::tcp::stream<>&& from) :
            stream{boost::beast::websocket::stream<malloy::tcp::stream<>>{std::move(from)}}
        {
        }

#if MALLOY_FEATURE_TLS
		explicit
        stream(detail::tls_stream&& ws) :
            m_underlying_conn{std::move(ws)}
		{
		}

        explicit
        stream(boost::beast::ssl_stream<malloy::tcp::stream<>>&& from) :
            stream{malloy::websocket::detail::tls_stream{
                boost::beast::websocket::stream<
                    boost::beast::ssl_stream<malloy::tcp::stream<>>
                >{std::move(from)}}}
        {
        }
#endif

        // ToDo: also return ec
        awaitable<boost::asio::ip::tcp::resolver::results_type::endpoint_type>
        async_connect(const boost::asio::ip::tcp::resolver::results_type& target)
        {
            return std::visit(
                [&target](auto& stream) mutable {
                    return boost::beast::get_lowest_layer(stream).async_connect(target, boost::asio::use_awaitable);
                },
                m_underlying_conn
            );
        }

		template<concepts::const_buffer_sequence Buff>
		awaitable< std::expected<std::size_t, error_code> >
        async_write(const Buff& buffers)
		{
			co_return co_await std::visit(
                [&buffers](auto& stream) mutable -> awaitable< std::expected<std::size_t, error_code> > {
                    auto [ec, bytes_written] = co_await stream.async_write(buffers, boost::asio::as_tuple(boost::asio::use_awaitable));
                    if (ec)
                        co_return std::unexpected(ec);

                    co_return bytes_written;
                },
                m_underlying_conn
            );
		}

		template<concepts::const_buffer_sequence Buff>
		std::size_t
        write(const Buff& buffers)
		{
			return std::visit(
                [&buffers](auto& stream) mutable {
                    return stream.write(buffers);
                },
                m_underlying_conn
            );
		}

		template<concepts::dynamic_buffer Buff>
        awaitable< std::expected<std::size_t, error_code> >
        async_read(Buff& buff)
		{
			co_return co_await std::visit(
                [&buff](auto& s) mutable -> awaitable< std::expected<std::size_t, error_code> > {
                    auto [ec, bytes_read] = co_await s.async_read(buff, boost::asio::as_tuple(boost::asio::use_awaitable));
                    if (ec)
                        co_return std::unexpected(ec);

                    co_return bytes_read;
                },
                m_underlying_conn
            );
		}

		template<concepts::dynamic_buffer Buff>
		std::size_t
        read(Buff& buff, boost::beast::error_code& ec)
		{
			return std::visit(
                [&buff, &ec](auto& s) mutable {
                    return s.read(buff, ec);
                },
                m_underlying_conn
            );
		}

        template<boost::asio::completion_token_for<void(malloy::error_code)> CompletionToken>
		auto
        async_close(boost::beast::websocket::close_reason why, CompletionToken&& done)
        {
            return std::visit(
                [why, done = std::forward<CompletionToken>(done)](auto& s) mutable {
                    return s.async_close(why, std::forward<CompletionToken>(done));
                },
                m_underlying_conn
            );
		}

		void
        set_option(auto&& opt)
		{
			std::visit(
                [opt = std::forward<decltype(opt)>(opt)](auto& s) mutable {
                    s.set_option(std::forward<decltype(opt)>(opt));
                },
                m_underlying_conn
            );
		}

		template<typename Body, typename Fields, boost::asio::completion_token_for<void(malloy::error_code)> CompletionHandler>
		auto
        async_accept(const boost::beast::http::request<Body, Fields>& req, CompletionHandler&& done)
		{
			return std::visit(
                [req, done = std::forward<decltype(done)>(done)](auto& s) mutable {
                    return s.async_accept(req, std::forward<decltype(done)>(done));
                },
                m_underlying_conn
            );
		}

		template<typename Body, typename Fields>
		auto
        accept(const boost::beast::http::request<Body, Fields>& req) -> boost::beast::error_code
		{
			return std::visit(
                [req](auto& s) {
                    return s.accept(req);
                },
                m_underlying_conn
            );
		}

        // ToDo: This should return error_code
		awaitable<void>
        async_handshake(std::string host, std::string target)
		{
			return std::visit(
                [host = std::move(host), target = std::move(target)](auto& s) mutable {
				    return s.async_handshake(host, target, boost::asio::use_awaitable);
			    },
                m_underlying_conn
            );
		}

        /**
         * @brief Controls whether outgoing message will be indicated text or binary.
         *
         * @param enabled Whether to enable binary mode.
         *
         * @sa binary()
         */
        void
        set_binary(const bool enabled)
        {
            std::visit(
                [enabled](auto& s) mutable {
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
        bool
        binary() const
        {
		    return std::visit(
                [](auto& s) {
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
		void
        get_lowest_layer(Func&& visitor)
		{
			std::visit(
                [visitor = std::forward<Func>(visitor)](auto& s) mutable {
                    visitor(boost::beast::get_lowest_layer(s));
                },
                m_underlying_conn
            );
		}

        /**
         * @brief Get executor of the underlying stream
         * @return s.get_executor() where s is any of the types in
         * detail::websocket_t
         */
		auto
        get_executor()
		{
			return std::visit(
                [](auto& s) {
                    return s.get_executor();
                },
                m_underlying_conn
            );
		}

        /**
         * @brief Returns `true` if the stream is open.
         * @details The stream is open after a successful handshake, and when no error has occurred.
         *
         * @return Whether the stream is open.
         */
        [[nodiscard]]
        bool
        is_open() const
        {
            return std::visit(
                [](auto& s){
                    return s.is_open();
                },
                m_underlying_conn
            );
        }

        /** 
         * @brief Whether the underlying stream is TLS or not 
         * @note Always false if MALLOY_FEATURE_TLS == 0
         */
        [[nodiscard]]
        constexpr
        bool
        is_tls() const
		{
#if MALLOY_FEATURE_TLS
			return std::holds_alternative<detail::tls_stream>(m_underlying_conn);
#else 
			return false;
#endif
		}

#if MALLOY_FEATURE_TLS
        awaitable<error_code>
        async_handshake_tls(const boost::asio::ssl::stream_base::handshake_type type)
        {
            // ToDo: return ec instead of throwing
            // ToDo: Is this actually necessary given that we have a visitor checking for the tls stream type?
			if (!is_tls())
                throw std::logic_error{"async_handshake_tls called on non-tls stream"};

            co_return co_await std::visit(
                [type](auto& s) mutable -> awaitable<error_code> {
                    if constexpr (std::same_as<std::decay_t<decltype(s)>, detail::tls_stream>)
                        co_await s.next_layer().async_handshake(type, boost::asio::use_awaitable);

                    co_return error_code{ };      // ToDo: Return actual error code from async_handshake()?
                },
                m_underlying_conn
            );
        }
		#endif

	private:
		ws_t m_underlying_conn;
	};

}
