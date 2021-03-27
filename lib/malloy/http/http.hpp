#pragma once

#include <boost/beast/http/verb.hpp>

namespace malloy::server::http
{
    using method = boost::beast::http::verb;

    using status = boost::beast::http::status;
}
