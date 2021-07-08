#include "../../example_logger.hpp"

#include <malloy/core/html/form.hpp>
#include <malloy/core/http/generator.hpp>
#include <malloy/server/controller.hpp>
#include <malloy/server/routing/router.hpp>

#include <iostream>

int main()
{
    const std::filesystem::path doc_root = "../../../../examples/server/static_content";

    // Create malloy controller config
    malloy::server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = doc_root;
    cfg.num_threads = 1;
    cfg.logger      = create_example_logger();

    // Create malloy controller
    malloy::server::controller c;
    if (!c.init(cfg)) {
        std::cerr << "could not start controller." << std::endl;
        return EXIT_FAILURE;
    }

    // Create the router
    auto router = c.router();
    if (router) {
        using namespace malloy::html;
        using namespace malloy::http;

        // Serve form
        router->add(method::get, "/", [doc_root](const auto& req) {
            return generator::file(doc_root, "form1.html");
        });

        // Handle form submits
        router->add(method::post, "/form1", [](const auto& req) {
            // Parse the form
            form f;
            if (!f.parse(req))
                return generator::bad_request("invalid form data.");

            // Print the form data
            std::cout << "form data:\n";
            for (const auto& [key, value] : f.values())
                std::cout << "  " << key << " = " << value << "\n";

            // Redirect back to the same page
            return generator::redirect(status::see_other, "/");
        });
    }

    // Start
    c.start();

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
