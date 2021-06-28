#include "generator.hpp"
#include "response.hpp"
#include "request.hpp"
#include <boost/beast/core/file_base.hpp>

using namespace malloy::http;

response<> generator::ok()
{
    response resp { status::ok };

    return resp;
}

response<> generator::redirect(const status code, const std::string_view location)
{
    const int& icode = static_cast<int>(code);
    if (icode < 300 || icode >= 400)
        return generator::server_error("invalid redirection status code.");

    response resp{ code };
    resp.set(field::location, location);
    resp.prepare_payload();

    return resp;
}

response<> generator::bad_request(std::string_view reason)
{
    response res(status::bad_request);
    res.set(field::content_type, "text/html");
    res.body() = reason;
    res.prepare_payload();

    return res;
}

response<> generator::not_found(std::string_view resource)
{
    response res(status::not_found);
    res.set(field::content_type, "text/html");
    res.body() = "The resource '" + std::string(resource) + "' was not found.";
    res.prepare_payload();

    return res;
}

response<> generator::server_error(std::string_view what)
{
    response res(status::internal_server_error);
    res.set(field::content_type, "text/html");
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();

    return res;
}



auto generator::file(const std::filesystem::path& storage_base_path, std::string_view rel_path) -> file_response
{
    // Sanitize rel_path
    {
        // Check for relative paths
        if (rel_path.find("..") != std::string::npos)
            return bad_request("resource path must not contain \"..\"");

        // Drop leading slash, if any
        if (rel_path.starts_with("/"))
            rel_path = rel_path.substr(1);
    }

    const std::filesystem::path& path = storage_base_path / rel_path;

    // Check whether this is a valid file path
    if (!std::filesystem::is_regular_file(path))
        return not_found(rel_path);

    // Get mime type
    const std::string_view& mime_type = malloy::mime_type(path);

    // Create response
    response<boost::beast::http::file_body> resp{status::ok};
    resp.set(field::content_type, mime_type);

    boost::beast::error_code ec;
    resp.body().open(path.string().c_str(), boost::beast::file_mode::scan, ec);
    if (ec) {
        return server_error(ec.message());
    }

    return resp;
}
