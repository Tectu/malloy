#pragma once

#include "rate_policy.hpp"

#include <boost/beast/core/tcp_stream.hpp>

namespace malloy::tcp
{

    /**
     * A TCP stream.
     */
    // ToDo: RatePolicy template parameter
    //template<class RatePolicy = malloy::tcp::rate_policy::unlimited>
    using stream = boost::beast::basic_stream<
        boost::asio::ip::tcp,
        boost::asio::any_io_executor,
        malloy::tcp::rate_policy::unlimited
    >;

}
