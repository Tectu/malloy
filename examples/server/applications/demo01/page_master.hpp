#include <malloy/server/app_fw/page_master.hpp>

class page_master :
    public malloy::server::app_fw::page_master
{
public:
    page_master() :
        malloy::server::app_fw::page_master("../../examples/server/applications/demo01/assets/templates/master.html")
    {
    }

private:
    [[nodiscard]]
    nlohmann::json
    data() const override
    {
        nlohmann::json j;

        j["system"]["assets_base_url"] = "http://127.0.0.1:8080";

        return j;
    }
};
