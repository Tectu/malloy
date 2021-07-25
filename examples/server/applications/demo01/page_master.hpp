#include <malloy/server/app_fw/page_master.hpp>

class page_master :
    public malloy::server::app_fw::page_master
{
public:
    page_master() :
        malloy::server::app_fw::page_master("assets/templates/master.html")
    {
    }

private:
    [[nodiscard]]
    nlohmann::json
    data() const override
    {
        return { };
    }
};
