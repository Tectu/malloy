#pragma once

namespace malloy::server::app_fw
{

    /**
     * Environment. Mainly used by @p app.
     */
    struct environment
    {
        /**
         * Site related stuff.
         */
        struct {
            std::string base_url;
        } site;

        /**
         * App related stuff.
         */
        struct {
            std::string base_url;
            std::filesystem::path assets_fs_path;
        } app;

        /**
         * Makes a sub-environment.
         *
         * This is mainly used by apps to create an environment for a sub app.
         *
         * @param name The name of the sub-app.
         * @return The sub-environment.
         */
        [[nodiscard]]
        environment
        make_sub_environment(const std::string& name)
        {
            environment env{ *this };
            env.app.base_url += "/" + name;
            env.app.assets_fs_path /= name;

            return env;
        }
    };

}
