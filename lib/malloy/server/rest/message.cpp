#include "message.hpp"

#include <malloy/core/http/generator.hpp>

using namespace malloy::server::rest;

malloy::http::response<>
message::to_http_response() const
{
    malloy::http::response resp{ m_http_status };

    try {
        resp.body() = "ToDo: Serialize JSON";
    }
    catch (const std::exception& e) {
        return malloy::http::generator::server_error("Exception during response assembly.");
    }

    // ToDo: Probably not necessary as router does that when sending
    resp.prepare_payload();

    return resp;
}
