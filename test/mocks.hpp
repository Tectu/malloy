
#pragma once

#include <malloy/http/request.hpp>

#define MALLOY_INTERNAL_TESTING    // Enable testing overloads


namespace malloy::mock::http
{
    class connection
    {
    public:
        class request_generator
        {
        public:
            malloy::http::request_header<> header_;

            explicit request_generator(malloy::http::request_header<> header) :
                header_{std::move(header)} {}

            auto header() const -> const malloy::http::request_header<>&
            {
                return header_;
            }

            template<typename Body, std::invocable<malloy::http::request<Body>&&> Callback, typename Setup>
            void body(Callback&& done, Setup&& s)
            {
                malloy::http::request<Body> r;
                r.base() = header_;
                s(r.body());
                done(r);
            }

            template<typename Body, std::invocable<malloy::http::request<Body>&&> Callback>
            void body(Callback&& done)
            {
                return done(malloy::http::request<Body>{header_});
            }
        };
    };
}    // namespace malloy::mock::http
