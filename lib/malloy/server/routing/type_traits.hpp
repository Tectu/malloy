#pragma once

#include <concepts> 


namespace malloy::server::concepts {
template<typename H, typename Req>
concept advanced_route_handler = std::invocable<H, Req> && requires(H handler, const typename Req::header_type& h) {
            handler.body_for(h);
};

}

