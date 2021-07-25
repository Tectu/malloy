#include "page_content.hpp"
#include "page_master.hpp"

using namespace malloy::server::app_fw;

page_content::page_content(
    std::filesystem::path template_path,
    std::shared_ptr<page_master> master
):
    m_template_path(std::move(template_path)),
    m_master(std::move(master))
{
    // Sanity check
    if (!m_master)
        throw std::invalid_argument("no valid master page provided.");
}

malloy::http::response<>
page_content::render() const
{
    // Rendering is done by the page_master class
    return m_master->render(*this);
}
