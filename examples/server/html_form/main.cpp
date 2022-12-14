#include "../../example.hpp"

#include <malloy/core/html/form.hpp>
#include <malloy/core/html/form_renderer.hpp>
#include <malloy/core/http/generator.hpp>
#include <malloy/server/routing_context.hpp>
#include <malloy/server/routing/router.hpp>

#include <iostream>

int main()
{
    using namespace malloy;

    // Create malloy controller config
    server::routing_context::config cfg;
    setup_example_config(cfg);
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;

    // Create malloy controller
    server::routing_context c{cfg};

    // Create form1
    // This is a simple login form using application/x-www-form-urlencoded
    html::form form1(
        http::method::post,
        "http://127.0.0.1:8080/form1",
        html::form::encoding::url
    );
    {
        // Username field
        form1.add_field({
           .name = "username",
           .type = "text",
           .label = "Username:",
           .required = true
        });

        // Password field
        form1.add_field({
            .name = "password",
            .type = "password",
            .label = "Password",
            .required = true
        });

        // Submit button
        form1.add_field({
            .name = "Login",
            .type = "submit"
        });
    }

    // Create form2
    // This is an image-upload form using multipart/form-data
    html::form form2(
        http::method::post,
        "http://127.0.0.1:8080/form2",
        html::form::encoding::multipart
    );
    {
        // Caption field
        form2.add_field({
            .name = "caption",
            .type = "text",
            .label = "Caption:",
            .required = true
        });

        // Image field
        form2.add_field({
            .name = "image",
            .type = "file",
            .label = "File:",
            .required = true
        });

        // Submit button
        form2.add_field({
            .name = "Upload!",
            .type = "submit"
        });
    }

    // Create the router
    auto& router = c.router();
    {
        using namespace malloy::html;
        using namespace malloy::http;

        // Serve root page
        router.add(method::get, "/", [](const auto& req) {
            return generator::ok();
        });

        // GET form1
        router.add(method::get, "/form1", [&form1](const auto& req) {
            // Prepare renderer
            html::form_renderer_basic renderer;

            // Prepare body
            std::string body;
            body += "<html><body>";
            body += renderer.render(form1);
            body += "</body></html>";

            // Respond
            auto resp = generator::ok();
            resp.body() = body;

            return resp;
        });

        // POST form1
        router.add(method::post, "/form1", [&form1](const auto& req) {
            // Parse the form
            const auto& data = form1.parse(req);
            if (!data)
                return generator::bad_request("invalid form data.");

            // Print the form data
            std::cout << "form1 data:\n";
            std::cout << data->dump() << "\n";

            // Redirect
            return generator::redirect(status::see_other, "/");
        });

        // GET form2
        router.add(method::get, "/form2", [&form2](const auto& req) {
            // Prepare renderer
            html::form_renderer_basic renderer;

            // Prepare body
            std::string body;
            body += "<html><body>";
            body += renderer.render(form2);
            body += "</body></html>";

            // Respond
            auto resp = generator::ok();
            resp.body() = body;

            return resp;
        });

        // POST form1
        router.add(method::post, "/form2", [&form2](const auto& req) {
            // Parse the form
            const auto& data = form2.parse(req);
            if (!data)
                return generator::bad_request("invalid form data.");

            // Print the form data
            std::cout << "form2 data:\n";
            std::cout << data->dump() << "\n";

            // Redirect
            return generator::redirect(status::see_other, "/");
        });
    }

    // Start
    [[maybe_unused]] auto session = start(std::move(c));

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
