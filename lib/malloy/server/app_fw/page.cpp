#include "page.hpp"
#include "../../core/http/generator.hpp"

using namespace malloy::server::app_fw;

malloy::http::response<>
page::response() const
{
    using namespace malloy::http;

    try {
        auto resp = generator::ok();
        resp.set(field::content_type, "text/html");
        resp.body() = render();
        resp.prepare_payload();

        return resp;
    }
    catch (const std::exception&) {
        return generator::server_error("exception during page rendering.");
    }
}
