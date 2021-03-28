#pragma once

#include <boost/beast/http/verb.hpp>

namespace malloy::http
{
    using method = boost::beast::http::verb;

    using status = boost::beast::http::status;
}
