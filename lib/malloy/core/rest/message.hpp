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

    struct empty_object
    {
        using id_type = std::size_t;

        [[nodiscard]]
        nlohmann::json
        to_json() const
        {
            return { };
        }

        bool
        from_json([[maybe_unused]] nlohmann::json&& j)
        {
            return true;
        }
    };

    /**
     * Message
     *
     * @brief The type forming the body of a REST HTTP request or response.
     */
    // ToDo: Provide wrapper (CRTP?) so deriving classes don't have to do this.
    struct message
    {
    protected:
        virtual
        nlohmann::json
        data_to_json() const
        {
            return { };
        }

        // ToDo: move?
        virtual
        bool
        data_from_json([[maybe_unused]] nlohmann::json&& j)
        {
            return true;
        }
    };

    /**
     * A REST API request.
     */
    struct request :
        message
    {
        template<object Object>
        [[nodiscard]]
        static
        std::optional<Object>
        from_http_request(malloy::http::request<> req)
        {
            try {
                auto json = nlohmann::json::parse(req.body());

                Object obj;
                if (!obj.from_json(std::move(json)))
                    return { };

                return obj;
            }
            catch (const std::exception& e)
            {
                return { };
            }
        }
    };

    /**
     * A REST API response.
     */
    struct response :
        message
    {
        [[nodiscard]]
        malloy::http::response<>
        to_http_response() const
        {
            malloy::http::response resp{ m_http_status };

            try {
                nlohmann::json json;

                // Error
                json["error"]["code"]    = m_api_error_code;
                json["error"]["message"] = m_api_error_msg;

                // Data
                json["data"] = data_to_json();

                resp.body() = json.dump();
            }
            catch (const std::exception& e) {
                return malloy::http::generator::server_error("exception during response assembly.");
            }

            resp.set(malloy::http::field::content_type, "application/json");

            return resp;
        }

        bool
        from_http_response(const malloy::http::response<>& resp)
        {
            // ToDo: Implement this
            (void)resp;

            return true;
        }

    protected:
        malloy::http::status m_http_status{ malloy::http::status::not_implemented };  // ToDo: Sane default?
        std::uint32_t m_api_error_code = std::numeric_limits<std::uint16_t>::max();
        std::string   m_api_error_msg;
    };

    template<object Object = empty_object>
    struct success :
        response
    {
        // ToDo: Move obj?
        explicit
        success(Object obj = { }) :
            m_obj{ obj }
        {
            m_http_status = malloy::http::status::ok;
            m_api_error_code = 0;
        }

    protected:
        nlohmann::json
        data_to_json() const override
        {
            if constexpr (std::same_as<Object, empty_object>)
                return { };
            else
                return m_obj.to_json();
        }

        bool
        data_from_json(nlohmann::json&& j) override
        {
            if constexpr (std::same_as<Object, empty_object>)
                return { };
            else
                return m_obj.from_json(std::move(j));
        }

    private:
        Object m_obj;
    };

    // ToDo: Add location header
    template<object Object = empty_object>
    struct created :
        success<Object>
    {
        explicit
        created(Object obj = { }) :
            success<Object>( std::move(obj) )
        {
            // Note: this-> is required here because the name lookup there doesn't include base classes that depend
            //       on template parameters.
            this->m_http_status = malloy::http::status::created;
            this->m_api_error_code = 0;
        }
    };

}
