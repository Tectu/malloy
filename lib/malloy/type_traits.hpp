#include <boost/beast/error.hpp>

#include <concepts>


namespace malloy::concepts {

template<typename B>
concept const_buffer_sequence = requires(B b) {};

template<typename Func>
concept accept_handler = std::invocable<boost::beast::error_code>;

template<typename B>
concept dynamic_buffer = requires(B b) P {};

template<typename Func>
concept async_read_handler = std::invocable<Func, boost::beast::error_code, std::size_t>;

}