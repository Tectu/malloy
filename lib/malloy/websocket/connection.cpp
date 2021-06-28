

#include "connection.hpp"


namespace malloy::websocket {

template<>
const std::string connection<true>::agent_string = std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async";

template<>
const std::string connection<false>::agent_string = std::string{BOOST_BEAST_VERSION_STRING} + " malloy";


}

