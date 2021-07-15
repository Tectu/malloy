#include "../../../example.hpp"

#include <malloy/server/controller.hpp>
#include <malloy/server/listener.hpp>
#include <malloy/server/routing/router.hpp>

#include <iostream>

int main(int argc, char* argv[])
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

    // Create top-level router
    auto router = c.router();
    if (router) {
        using namespace malloy;
        using namespace malloy::http;

        router->add(method::get, "/", [](const auto& req) {
            response res{ status::ok };
            res.body() = "<html><body><h1>Hello World!</h1><p>some content...</p></body></html>";
            return res;
        });

        // Create sub-router 1
        auto sub_router_1 = std::make_shared<server::router>();
        sub_router_1->add(method::get, "/", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/\"";
            return resp;
        });
        sub_router_1->add(method::get, "/foo", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/foo\"";
            return resp;
        });
        sub_router_1->add(method::get, "/foo/bar", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/foo/bar\"";
            return resp;
        });
        sub_router_1->add(method::get, "/foo/\\w+", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/foo/\\w+\"";
            return resp;
        });
        router->add_subrouter("/router1", sub_router_1);

        // Create sub-router 2
        auto sub_router_2 = std::make_shared<server::router>();
        sub_router_2->add(method::get, "/", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 2 target \"/\"";
            return resp;
        });
        sub_router_2->add(method::get, "/foo", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 2 target \"/foo\"";
            return resp;
        });
        router->add_subrouter("/router2", sub_router_2);
    }

    // Start
    c.start();

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
