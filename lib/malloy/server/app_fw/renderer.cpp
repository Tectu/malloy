#include "renderer.hpp"

#include <inja/inja.hpp>

using namespace malloy::server::app_fw;

std::string
renderer::render_single_page(const std::filesystem::path& tmpl_path, const nlohmann::json& data)
{
    // Sanity check path
    if (not std::filesystem::is_regular_file(tmpl_path))
        return { };

    try {
        // Create inja environment
        inja::Environment env;

        // Parse templaet
        inja::Template tmpl = env.parse_template(tmpl_path.string());

        // Render
        return env.render(tmpl, data);
    }
    catch (const nlohmann::json::exception& e) {
        //m_env.logger->error("json exception during template rendering: {}", e.what());
        return { };
    }
    catch (const inja::RenderError& e) {
        //m_env.logger->error("inja exception during template rendering: {}", e.what());
        return { };
    }
    catch (const std::exception& e) {
        //m_env.logger->error("exception during template rendering: {}", e.what());
        return { };
    }
}
