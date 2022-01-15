#include "page_master.hpp"
#include "page_content.hpp"
#include "../../core/http/generator.hpp"

#include <inja/inja.hpp>

using namespace malloy::server::app_fw;

page_master::page_master(std::filesystem::path template_path) :
    m_tmpl_path(std::move(template_path))
{
}

malloy::http::response<>
page_master::render_impl(const nlohmann::json& data, const page_content* content) const
{
    using namespace malloy::http;

    // Sanity check
    if (!std::filesystem::is_regular_file(m_tmpl_path))
        return generator::server_error("could not locate master template file.");

    // Render
    std::string body;
    try {
        // Create inja environment
        inja::Environment env;

        // Parse content
        if (content) {
            auto parsed_content_template = env.parse_template(content->template_path().string());
            env.include_template("content", parsed_content_template);
        }

        // Parse master template
        inja::Template tmpl = env.parse_template(m_tmpl_path.string());

        // Render
        body = env.render(tmpl, data);
    }
    catch ([[maybe_unused]] const nlohmann::json::exception& e) {
        return generator::server_error("JSON exception during template rendering.");
    }
    catch ([[maybe_unused]] const inja::RenderError& e) {
        return generator::server_error("Exception during template rendering.");
    }
    catch ([[maybe_unused]] const std::exception& e) {
        return generator::server_error("General exception during template rendering.");
    }

    // Assemble HTTP response
    auto resp = generator::ok();
    resp.body() = body;
    resp.set(field::content_type, "text/html");
    resp.prepare_payload();

    return resp;
}

malloy::http::response<>
page_master::render() const
{
    return render_impl(data(), nullptr);
}

malloy::http::response<>
page_master::render(const page_content& content) const
{
    // Prepare data
    nlohmann::json jd = data();
    jd["content"] = content.data();

    return render_impl(jd, &content);
}
