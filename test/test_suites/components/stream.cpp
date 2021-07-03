
#include "../../test.hpp"

#include <malloy/websocket/stream.hpp>


using namespace malloy::websocket;
namespace net = boost::asio;


TEST_SUITE("components - websocket - stream") {
    TEST_CASE("Closing an unopened connection does not cause a crash") {
        net::io_context ioc;
        stream unopened{boost::beast::websocket::stream<boost::beast::tcp_stream>{ioc}};
        CHECK_NOTHROW(unopened.close());

    }

}





