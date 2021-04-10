#include "malloy/controller.hpp"
#include "malloy/listener.hpp"
#include "malloy/http/router.hpp"

#include <iostream>
#include <memory>

int main(int argc, char* argv[])
{
    const std::filesystem::path doc_root = "../../../../examples/static_content";

    // Create top-level router
    auto router = std::make_shared<malloy::http::server::router>(logger->clone("router"));
    {
        using namespace malloy::http;

        router->add(method::get, "/", [](const auto& req) {
            response res{ status::ok };
            res.body() = "<html><body><h1>Hello World!</h1><p>some content...</p></body></html>";
            return res;
        });

        // Create nested router 1
        auto nested_router_1 = std::make_shared<malloy::http::server::router>(logger->clone("router 1"));
        nested_router_1->add(method::get, "/", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/\"";
            return resp;
        });
        nested_router_1->add(method::get, "/foo", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/foo\"";
            return resp;
        });
        nested_router_1->add(method::get, "/foo/bar", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/foo/bar\"";
            return resp;
        });
        nested_router_1->add(method::get, "/foo/\\w+", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/foo/\\w+\"";
            return resp;
        });
        router->add("/router1", std::move(nested_router_1));

        // Create nested router 2
        auto nested_router_2 = std::make_shared<malloy::http::server::router>(logger->clone("router 2"));
        nested_router_2->add(method::get, "/", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 2 target \"/\"";
            return resp;
        });
        nested_router_2->add(method::get, "/foo", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 2 target \"/foo\"";
            return resp;
        });
        router->add("/router2", std::move(nested_router_2));
    }

    // Create malloy controller config
    malloy::server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = doc_root;
    cfg.num_threads = 5;
    cfg.router      = router;

    // Create malloy controller
    malloy::server::controller c;
    if (not c.init(cfg)) {
        std::cerr << "could not start controller." << std::endl;
        return EXIT_FAILURE;
    }

    // Start
    c.start();

    return EXIT_SUCCESS;
}
