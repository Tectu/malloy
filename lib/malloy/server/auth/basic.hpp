#pragma once

#include "../../core/http/type_traits.hpp"
#include "../../core/http/response.hpp"
#include "../../core/http/generator.hpp"

#include <fmt/format.h>

#include <iostream>

namespace malloy::server::http::auth
{
    namespace detail
    {
        struct auth_field
        {
            std::string method;
            std::string username;
            std::string password;
        };

        template<typename D>
        struct base64_decoder_helper
        {
            void operator()(std::string_view v, std::string s)
            {
                auto b = boost::asio::dynamic_buffer(s);
                D::decode(v, s);
            }
        };

        template<typename D>
        concept base64_decoder = requires(std::string_view v, std::string s)
        {
            { base64_decoder_helper<D>{}(v, s) };
        };

        template<base64_decoder Decoder>
        inline
        std::optional<auth_field>
        parse_auth_field(std::string_view txt)
        {
            auto method_pos = txt.find_first_of(' ');
            if (method_pos == std::string::npos)
                return std::nullopt;

            auto encoded_usepass = txt.substr(method_pos+1);
            try {
                std::string out;
                auto outbuff = boost::asio::dynamic_buffer(out);
                Decoder::decode(encoded_usepass, outbuff);
                out.erase(std::find(out.begin(), out.end(), '\0'), out.end());
                const auto parsed_usepass = malloy::split(out, ":");
                if (parsed_usepass.size() != 2)
                    return std::nullopt;

                return auth_field{std::string{txt.begin(), txt.begin() + method_pos}, std::string{parsed_usepass.at(0)}, std::string{parsed_usepass.at(1)}};
            }
            catch (const std::invalid_argument&) {
                return std::nullopt;
            }
        }
    }

    template<detail::base64_decoder Decoder, malloy::http::concepts::body BodyType = boost::beast::http::string_body>
    class basic
    {
        static constexpr auto cli_auth_field_name = "Authorization";

    public:
        using body_type = BodyType;
        using response_type = malloy::http::response<body_type>;

        basic(std::string username, std::string password, std::string realm) :
            m_username{std::move(username)},
            m_password{std::move(password)},
            m_realm{std::move(realm)}
        {
            rebuild_www_auth_txt();
        }

        void
        set_charset(const std::string& chset)
        {
            m_charset = chset;
            rebuild_www_auth_txt();
        }

        std::optional<response_type>
        operator()(const boost::beast::http::request_header<>& h)
        {
            auto auth_iter = h.find(cli_auth_field_name);
            if (auth_iter == h.end()) {
                // No auth header
                return not_authed_resp();
            }

            auto maybe_parsed = detail::parse_auth_field<Decoder>(auth_iter->value());
            if (!maybe_parsed)
                return not_authed_resp();

            const auto auth_details = std::move(*maybe_parsed);
            const auto r1 = auth_details.method != "Basic";
            const auto r2 = auth_details.username != m_username;
            const auto r3 = auth_details.password != m_password;
            if ((auth_details.method != std::string_view{"Basic"}) || (auth_details.username != m_username) || (auth_details.password != m_password))
                return not_authed_resp();
            else
                return std::nullopt;
        }

    private:
        void
        rebuild_www_auth_txt()
        {
            m_www_auth = fmt::format(R"(Basic realm="{}", charset="{}")", m_realm, m_charset);
        }

        response_type
        not_authed_resp()
        {
            response_type resp{malloy::http::status::unauthorized};
            resp.set(malloy::http::field::www_authenticate, m_www_auth);
            resp.prepare_payload();
            return resp;
        }

        std::string m_username;
        std::string m_password;
        std::string m_realm;
        std::string m_www_auth;
        std::string m_charset{"UTF-8"};
    };

}
