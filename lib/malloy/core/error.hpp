#pragma once

#include <boost/beast/core/error.hpp>

namespace malloy
{

    /// Error code used to signify errors without throwing. Truthy means it holds an error.
	using error_code = boost::beast::error_code;

}
