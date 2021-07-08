#pragma once

#include <boost/beast/http/type_traits.hpp>

namespace malloy::http::concepts
{

    template<typename B>
    concept body = boost::beast::http::is_body<B>::value;

}
