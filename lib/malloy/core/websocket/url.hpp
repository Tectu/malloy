#pragma once

#include <boost/url/parse.hpp>
#include <boost/url/url.hpp>

#include "types.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace malloy::websocket
{

    // ToDo: Find a better name
    struct endpoint
    {
        bool use_tls = false;
        std::string host;
        std::uint16_t port = 0;
        std::string target;
    };

    [[nodiscard]]
    inline
    std::optional<endpoint>
    build_endpoint(const boost::urls::url& url)
    {
        // Decompose
        endpoint ep;
        {
            // Scheme
            switch (url.scheme_id()) {
                case boost::urls::scheme::ws:
                    ep.use_tls = false;
                    break;

                case boost::urls::scheme::wss:
                    ep.use_tls = true;
                    break;

                default:    // ToDo: Handle more
                    return std::nullopt;
            }

            // Host
            ep.host = url.host();

            // Port
            ep.port = url.port_number();
            if (ep.port == 0) {
                switch (url.scheme_id()) {
                    case boost::urls::scheme::ws:
                        ep.port = 80;
                        break;

                    case boost::urls::scheme::wss:
                        ep.port = 443;
                        break;

                    default:
                        break;
                }
            }

            // Target
            ep.target = url.encoded_target();
            if (std::empty(ep.target))
                ep.target = "/";
        }

        return ep;
    }

    [[nodiscard]]
    inline
    std::optional<endpoint>
    build_endpoint(const std::string_view url)
    {
        // Parse URL
        auto parsed_url = boost::urls::parse_uri(url);
        if (!parsed_url)
            return std::nullopt;

        // Build endpoint
        return build_endpoint(*parsed_url);
    }

}
