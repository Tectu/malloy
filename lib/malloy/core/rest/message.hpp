#pragma once

#include "concepts.hpp"
#include "../http/response.hpp"
#include "../http/types.hpp"
#include "../http/generator.hpp"

#include <nlohmann/json.hpp>

#include <limits>
#include <string>

namespace malloy::rest
{

    /**
     * Message
     *
     * @brief The type forming the body of a REST HTTP request or response.
     */
    struct message
    {
        template<object Object>
        bool
        from_json(const nlohmann::json& j, Object& obj)
        {
            // ToDo
            (void)j;
            (void)obj;
        }

        template<object Object>
        nlohmann::json
        to_json(const Object& obj) const
        {
            // ToDo
            (void)obj;

            return { };
        }

        [[nodiscard]]
        malloy::http::response<>
        to_http_response() const
        {
            malloy::http::response resp{ m_http_status };

            try {
                resp.body() = "ToDo: Serialize JSON";
            }
            catch (const std::exception& e) {
                return malloy::http::generator::server_error("Exception during response assembly.");
            }

            // ToDo: Probably not necessary as router does that when sending
            resp.prepare_payload();

            return resp;
        }

    protected:
        malloy::http::status m_http_status{ malloy::http::status::not_implemented };  // ToDo: Sane default?
        std::uint32_t m_api_error_code = std::numeric_limits<std::uint16_t>::max();
        std::string   m_api_error_msg;
    };

    struct success :
        message
    {
        success()
        {
            m_http_status = malloy::http::status::ok;
            m_api_error_code = 0;
        }
    };
}
