#include <malloy/server/app_fw/page_content.hpp>

namespace apps::gallery::pages
{

    class overview :
        public malloy::server::app_fw::page_content
    {
    public:
        explicit
        overview(
            std::shared_ptr<malloy::server::app_fw::page_master> master_page
        ) :
            malloy::server::app_fw::page_content(
                "assets/templates/overview.html",
                std::move(master_page)
            )
        {
        }

    protected:
        [[nodiscard]]
        nlohmann::json
        data() const override
        {
            return { };
        }
    };

}
