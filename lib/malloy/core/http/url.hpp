#pragma once

#include <boost/url/parse.hpp>
#include <boost/url/url.hpp>

#include "request.hpp"
#include "types.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace malloy::http
{

    template<typename Body>
    [[nodiscard]]
    std::optional<request<Body>>
    build_request(const method method_, const boost::urls::url& url)
    {
        // Decompose
        std::string host;
        std::uint16_t port = 0;
        std::string target;
        bool use_tls = true;
        {
            // Scheme
            switch (url.scheme_id()) {
                case boost::urls::scheme::http:
                    use_tls = false;
                    break;

                case boost::urls::scheme::https:
                    use_tls = true;
                    break;

                default:    // ToDo: Handle more
                    return std::nullopt;
            }

            // Host
            host = url.host();

            // Port
            port = url.port_number();
            if (port == 0) {
                switch (url.scheme_id()) {
                    case boost::urls::scheme::http:
                        port = 80;
                        break;

                    case boost::urls::scheme::https:
                        port = 443;
                        break;

                    default:
                        break;
                }
            }

            // Target
            target = url.encoded_target();
            if (std::empty(target))
                target = "/";
        }

        // ToDo: Does host need to stay alive?!

        request<Body> req(method_, host, port, target);
        req.use_tls(use_tls);

        return req;
    }

    template<typename Body>
    [[nodiscard]]
    std::optional<request<Body>>
    build_request(const method method_, const std::string_view& url)
    {
        // Parse URL
        auto parsed_url = boost::urls::parse_uri(url);
        if (!parsed_url)
            return std::nullopt;

        // Build request
        return build_request<Body>(method_, *parsed_url);
    }

}
