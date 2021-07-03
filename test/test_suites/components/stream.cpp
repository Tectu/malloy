
#include "../../test.hpp"

#include <boost/beast/websocket/rfc6455.hpp>
#include <malloy/websocket/stream.hpp>


using namespace malloy::websocket;
namespace net = boost::asio;


TEST_SUITE("components - websocket - stream") {
    TEST_CASE("Closing an unopened connection does not cause a crash, but generates a truthy error code") {
        net::io_context ioc;
        stream unopened{boost::beast::websocket::stream<boost::beast::tcp_stream>{ioc}};
        
        bool cb_invoked = false;
        unopened.async_close(boost::beast::websocket::normal, [&cb_invoked](auto ec) mutable {
                CHECK(ec);
                cb_invoked = true;
        });
        ioc.run();
        CHECK(cb_invoked);
    }

}





