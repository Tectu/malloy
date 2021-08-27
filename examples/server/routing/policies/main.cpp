#include "../../../example.hpp"

#include <malloy/core/http/generator.hpp>
#include <malloy/server/controller.hpp>
#include <malloy/server/routing/router.hpp>
#include <malloy/server/auth/basic.hpp>

#include <iostream>

/**
 * Simple base64 decoder implementation.
 * This is used for the HTTP basic auth policy.
 *
 * Borrowed from: https://stackoverflow.com/a/5291537/12448530
 */
struct b64_decode
{
    template<malloy::concepts::dynamic_buffer Buff>
    static void decode(std::string_view in, Buff& out_raw)
    {
        constexpr char reverse_table[128] = {
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
            64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
            64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
        };

        const auto needed = static_cast<std::size_t>(std::ceil(4 * (static_cast<double>(in.size()) / 3.0)));
        auto out = out_raw.prepare(needed);
        const auto last = in.end();
        int bits_collected = 0;
        unsigned int accumulator = 0;
        auto out_iter = reinterpret_cast<unsigned char*>(out.data());

        for (auto i = in.begin(); i != last; ++i) {
            const int c = *i;
            if (std::isspace(c) || c == '=') {
                // Skip whitespace and padding. Be liberal in what you accept.
                continue;
            }
            if ((c > 127) || (c < 0) || (reverse_table[c] > 63)) {
                throw std::invalid_argument("This contains characters not legal in a base64 encoded string.");
            }
            accumulator = (accumulator << 6) | reverse_table[c];
            bits_collected += 6;
            if (bits_collected >= 8) {
                bits_collected -= 8;
                *out_iter = static_cast<char>((accumulator >> bits_collected) & 0xffu);
                ++out_iter;
            }
        }
    }
};

int main()
{
    // Create malloy controller config
    malloy::server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = examples_doc_root;
    cfg.num_threads = 5;
    cfg.logger      = create_example_logger();

    // Create malloy controller
    malloy::server::controller c;
    if (!c.init(cfg)) {
        std::cerr << "could not start controller." << std::endl;
        return EXIT_FAILURE;
    }

    // Setup the router
    {
        using namespace malloy::http;
        auto router = c.router();

        // A policy using HTTP basic auth
        router->add_policy("/", malloy::server::http::auth::basic<b64_decode>{"user01", "malloy", "My Realm"});
        router->add(method::get, "/", [](const auto &req)
        {
            response res{status::ok};
            res.body() = "<html><body><h1>Hello World!</h1><p>some content...</p></body></html>";
            return res;
        });
    }

    // Start
    c.start();

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}