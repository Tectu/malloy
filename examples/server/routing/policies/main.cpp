#include "../../../example.hpp"

#include <malloy/core/http/generator.hpp>
#include <malloy/server/routing_context.hpp>
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

            // Skip whitespace and padding. Be liberal in what you accept.
            if (std::isspace(c) || c == '=')
                continue;

            if ((c > 127) || (c < 0) || (reverse_table[c] > 63))
                throw std::invalid_argument("This contains characters not legal in a base64 encoded string.");

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
    malloy::server::routing_context::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = examples_doc_root;
    cfg.num_threads = 5;
    cfg.logger      = create_example_logger();

    // Create malloy controller
    malloy::server::routing_context c{cfg};

    // Setup simple HTTP basic auth policy
    //   - Username: user01
    //   - Password: malloy
    malloy::server::http::auth::basic<b64_decode> policy{"user01", "malloy", "My Realm"};

    // Setup the router
    {
        using namespace malloy::http;
        auto& router = c.router();

        // Root page (no access restrictions)
        router.add(method::get, "/", [](const auto& req){
            response res{ status::ok };
            res.body() = "<html><body>"
                         "  <h1>Malloy Access Policy demo</h1>"
                         "   <p><a href=\"./restricted\">Access restricted resource</a></p>"
                         "   <p><a href=\"./admin\">Access restricted sub-router</a></p>"
                         "   <p>Username: user01</p><p>Password: malloy</p></body></html>";
            return res;
        });

        // Restricted endpoint
        router.add_policy("/restricted", policy);
        router.add(method::get, "/restricted", [](const auto &req) {
            response res{status::ok};
            res.body() = "<html><body><h1>Hello User01!</h1><p>some content...</p></body></html>";
            return res;
        });

        // Restricted sub-router
        auto sub_router = std::make_unique<malloy::server::router>();
        {
            // Add simple endpoint
            sub_router->add(method::get, "", [](const auto &req) {
                response res{status::ok};
                res.body() = "<html><body><h1>Hello User01!</h1><p>Welcome to the access restricted admin panel!</p></body></html>";
                return res;
            });

            // Add simple endpoint
            sub_router->add(method::get, "/foo", [](const auto &req) {
                response res{status::ok};
                res.body() = "<html><body>Foo</body></html>";
                return res;
            });

            // Serve files
            sub_router->add_file_serving("/files", cfg.doc_root);
        }
        router.add_policy("/admin/.+", policy);
        router.add_subrouter("/admin", std::move(sub_router));
    }

    // Start
    [[maybe_unused]] auto session = start(std::move(c));

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
