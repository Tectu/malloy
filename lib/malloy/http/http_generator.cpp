#include "http_generator.hpp"
#include "response.hpp"
#include "request.hpp"

using namespace malloy::http;

response http_generator::bad_request(std::string_view reason)
{
    response res(status::bad_request);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.body() = reason;
    res.prepare_payload();

    return res;
}

response http_generator::not_found(std::string_view resource)
{
    response res(status::not_found);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.body() = "The resource '" + std::string(resource) + "' was not found.";
    res.prepare_payload();

    return res;
}

response http_generator::server_error(std::string_view what)
{
    response res(status::internal_server_error);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();

    return res;
}

response http_generator::file(const request& req, const std::filesystem::path& storage_base_path)
{
    // Sanity check the rel_path
    std::string_view rel_path = req.uri().resource_string();
    {
        // Check for relative paths
        if (rel_path.find("..") != std::string::npos)
            return bad_request("resource path must not contain \"..\"");
    }

    // Drop any leading slash
    if (rel_path.starts_with('/'))
        rel_path = rel_path.substr(1);

    // Create the filesystem path
    std::filesystem::path path = storage_base_path / rel_path;

    std::cout << "stor path: "  << storage_base_path << std::endl;
    std::cout << "rel_path : " << rel_path << std::endl;
    std::cout << "full path: " << path << std::endl;

    // Check whether this is a valid file path
    if (not std::filesystem::is_regular_file(path))
        return not_found(rel_path);

    // Get file content
    const std::string& file_content = malloy::file_contents(path);

    // Get mime type
    const std::string_view& mime_type = malloy::mime_type(path);

    switch (req.method()) {
        // GET request
        case method::get: {
            response resp{status::ok};
            resp.set(boost::beast::http::field::content_type, boost::string_view{mime_type.data(), mime_type.size()});
            resp.body() = file_content;
            return resp;
        }

        // HEAD request
        case method::head: {
            // ToDo!
            // Current problem: router::send_response() calls response::prepare_payload() which makes the manually set content_length vanish.
            /*
            response resp{ status::ok };
            resp.set(boost::beast::http::field::content_type, boost::string_view{mime_type.data(), mime_type.size()});
            resp.content_length(file_content.size());

            return resp;
            */
            [[fallthrough]];
        }

        default: {
            return bad_request("unknown HTTP method.");
        }
    }
}
