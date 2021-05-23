#pragma once

namespace malloy::server
{
    class route_websocket
    {
    public:
        using handler_t = std::function<std::string(const std::string&)>;

        std::string resource;
        handler_t handler;

        [[nodiscard]]
        std::string handle(const std::string& payload) const
        {
            if (handler)
                return handler(payload);

            return { };
        }
    };
}
