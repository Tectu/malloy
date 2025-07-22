#pragma once

#include <boost/beast/core/rate_policy.hpp>

namespace malloy::tcp::rate_policy
{

    /**
     * Unlimited rate policy.
     *
     * @details This policy does not limit the read or write rates. Free for all!
     */
    using unlimited = boost::beast::unlimited_rate_policy;

}
