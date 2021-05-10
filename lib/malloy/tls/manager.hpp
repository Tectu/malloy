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
    /**
     * Manager class for TLS.
     *
     * ToDo: Allow specifying the ciperlists (TLS 1.2 and below) / ciphersuites (TLS 1.3+)
     *       See https://www.openssl.org/docs/manmaster/man3/SSL_CTX_set_ciphersuites.html
     */
    class manager
    {
    public:
        /**
         * Create a TLS context.
         *
         * @param cert_path Path to the cert file.
         * @param key_path Path to he key file.
         * @return The context (if any)
         */
        [[nodiscard]]
        static
        std::shared_ptr<boost::asio::ssl::context>
        make_context(
            const std::filesystem::path& cert_path,
            const std::filesystem::path& key_path
        );
    };

}
