#pragma once

#include <filesystem>
#include <fstream>

#include <boost/asio/buffer.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

namespace malloy
{
    using boost::asio::buffer;
    using boost::beast::buffers_to_string;
    using boost::beast::bind_front_handler;
    
    /**
     * Returns an std::string which represents the raw bytes of the file.
     *
     * @param path The path to the file.
     * @return The content of the file as it resides on the disk - byte by byte.
     */
    [[nodiscard]]
    static inline std::string file_contents(const std::filesystem::path& path)
    {
        // Sanity check
        if (!std::filesystem::is_regular_file(path))
            return { };

        // Open the file
        // Note that we have to use binary mode as we want to return a string
        // representing matching the bytes of the file on the file system.
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open())
            return { };

        // Read contents
        const std::size_t& size = std::filesystem::file_size(path);
        std::string content(size, '\0');
        file.read(content.data(), size);

        // Close the file
        file.close();

        return content;
    }

    /**
     * Attempts to return a suitable MIME-TYPE for a provided file path
     *
     * @param path The file path.
     * @return The corresponding MIME-TYPE.
     */
    static inline std::string_view mime_type(const std::filesystem::path& path)
    {
        // Extract file extension
        const std::filesystem::path& ext = path.extension();

        if (ext == ".htm")  return "text/html";
        if (ext == ".html") return "text/html";
        if (ext == ".php")  return "text/html";
        if (ext == ".css")  return "text/css";
        if (ext == ".txt")  return "text/plain";
        if (ext == ".js")   return "application/javascript";
        if (ext == ".json") return "application/json";
        if (ext == ".xml")  return "application/xml";
        if (ext == ".swf")  return "application/x-shockwave-flash";
        if (ext == ".flv")  return "video/x-flv";
        if (ext == ".png")  return "image/png";
        if (ext == ".jpe")  return "image/jpeg";
        if (ext == ".jpeg") return "image/jpeg";
        if (ext == ".jpg")  return "image/jpeg";
        if (ext == ".gif")  return "image/gif";
        if (ext == ".bmp")  return "image/bmp";
        if (ext == ".ico")  return "image/vnd.microsoft.icon";
        if (ext == ".tiff") return "image/tiff";
        if (ext == ".tif")  return "image/tiff";
        if (ext == ".svg")  return "image/svg+xml";
        if (ext == ".svgz") return "image/svg+xml";

        return "application/text";
    }

}
