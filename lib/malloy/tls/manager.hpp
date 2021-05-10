#pragma once

#include <filesystem>
#include <memory>
#include <string>

namespace boost::asio::ssl
{
    class context;
}

namespace malloy::tls
{
    class manager
    {
    public:
        /**
         * The default ciphers if none were specified.
         */
        static constexpr const char* default_ciphers = {
            "ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-"
            "SHA256:ECDHE-RSA-AES128-GCM-"
            "SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-"
            "SHA256:DHE-RSA-AES256-GCM-"
            "SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-"
            "RSA-AES256-SHA384:ECDHE-"
            "RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:"
            "DHE-RSA-AES128-SHA256:"
            "DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:ECDHE-ECDSA-DES-CBC3-SHA:"
            "ECDHE-RSA-DES-CBC3-SHA:"
            "EDH-RSA-DES-CBC3-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:"
            "AES128-SHA:AES256-SHA:"
            "DES-CBC3-SHA:!DSS"
        };

        /**
         * Create a TLS context.
         *
         * @param cert_path Path to the cert file.
         * @param key_path Path to he key file.
         * @param cipher_list The ciphers to use. Will use `default_ciphers` if empty.
         * @return The context (if any)
         */
        [[nodiscard]]
        static
        std::shared_ptr<boost::asio::ssl::context>
        make_context(
            const std::filesystem::path& cert_path,
            const std::filesystem::path& key_path,
            std::string_view cipher_list = { }
        );
    };

}
