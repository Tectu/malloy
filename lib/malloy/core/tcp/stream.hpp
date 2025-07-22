#pragma once

#include "rate_policy.hpp"

#include <boost/beast/core/tcp_stream.hpp>

namespace malloy::tcp
{

    /**
     * A TCP stream.
     *
     * @details A TCP stream has a rate policy. The default rate policy is `malloy::tcp::rate_policy::unlimited`.
     *
     * @tparam RatePolicy the rate policy.
     */
    template<class RatePolicy = malloy::tcp::rate_policy::unlimited>
    using stream = boost::beast::basic_stream<
        boost::asio::ip::tcp,
        boost::asio::any_io_executor,
        RatePolicy
    >;

}
