
#pragma once

#include "../response.hpp"
#include "../request.hpp"
#include "../../error.hpp"

#include <filesystem>
#include <variant>

/**
 * @namespace malloy::http::filters
 * @brief Contains the filter types bundled with malloy
 * @details See response_filter (@ref client_concepts) and request_filter (@ref route_concepts) for more info. 
 * All types in this namespace satisfy both concepts.
 */
namespace malloy::http::filters {

    /** 
     * @brief Writes the contents of a message to disk
     * @details This filter is used to write a request/response directly to disk
     * rather than storing it in-memory.
     */
    template<bool isRequest>
    struct basic_file {
        using response_type = malloy::http::response<boost::beast::http::file_body>;
        using request_type = malloy::http::request<boost::beast::http::file_body>;
        using value_type = boost::beast::http::file_body::value_type;
        using header_type = boost::beast::http::header<isRequest>;

        using setup_handler_t = std::function<void(const header_type&, value_type&)>;

        setup_handler_t setup;

        /**
         * @brief Default ctor 
         * @details Calls to setup_body will do nothing until setup is set to a
         * valid function
         */
        basic_file() = default;
        /**
         * @brief Construct with a setup handler
         */
        explicit basic_file(setup_handler_t setup_) : setup{ std::move(setup_) } {}

        basic_file(basic_file&&) noexcept = default;
        basic_file& operator=(basic_file&&) noexcept = default;

        /**
         * @brief Create a version of the filter that writes/reads the specified
         * file
         * @param location Path to the file on the local filesystem
         * @param on_error Callback invoked on an error during handling of the
         * request 
         * @param mode Mode to use when opening the file. Should not be a
         * readonly mode (e.g. NOT boost::beast::file_mode::scan)
         */
        static auto open(
            const std::filesystem::path& location, 
            std::function<void(malloy::error_code)> on_error, 
            boost::beast::file_mode mode) -> basic_file {
            return basic_file{ [location, mode, on_error = std::move(on_error)](auto&&, auto& body) {
                boost::beast::error_code ec;
                body.open(location.string().c_str(), mode, ec);
                if (ec && on_error) {
                    on_error(ec);
                }
            } };

        }

        auto body_for(const header_type&) const -> std::variant<boost::beast::http::file_body> {
            return {};
        }

        void setup_body(const header_type& h, value_type& body) const {
            if (setup) {
                setup(h, body);
            }
        }
    };

    using file_request = basic_file<true>;
    using file_response = basic_file<false>;

}
