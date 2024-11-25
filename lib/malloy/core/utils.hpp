#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

#include <filesystem>
#include <fstream>

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
    static
    inline
    std::string
    file_contents(const std::filesystem::path& path)
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
        std::string content{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

        // Close the file
        file.close();

        return content;
    }

    /**
     * Convert a hex value to a decimal value.
     *
     * @param c The hexadecimal input.
     * @return The decimal output.
     */
    [[nodiscard]]
    static
    inline
    uint8_t
    hex2dec(uint8_t c)
    {
        if (c >= '0' && c <= '9')
            c -= '0';

        else if (c >= 'a' && c <= 'f')
            c -= 'a' - 10;

        else if (c >= 'A' && c <= 'F')
            c -= 'A' - 10;

        return c;
    }

    /**
     * Splits a string.
     *
     * @note Passing an empty string will result in the return container containing an empty string.
     * @note Passing an empty string as the delimiter returns { str }.
     * @note Leading and trailing delimiters will be ignored.
     *
     * @param str The string to split.
     * @param delimiter The delimiter.
     * @return The string split into the corresponding parts.
     */
    [[nodiscard]]
    inline
    std::vector<std::string_view>
    split(std::string_view str, std::string_view delimiter)
    {
        // Sanity check str
        if (str.empty())
            return { };

        // Sanity check delimiter
        if (delimiter.empty())
            return { str };

        // Split
        std::vector<std::string_view> parts;
        std::string_view::size_type pos = 0;
        while (pos != std::string_view::npos) {
            // Look for substring
            const auto pos_found = str.find(delimiter, pos);

            // Drop leading delimiters
            if (pos_found == 0) {
                pos += delimiter.size();
                continue;
            }

            // Capture string
            parts.emplace_back(str.substr(pos, pos_found-pos));

            // Drop trailing delimiters
            if (pos_found + delimiter.size() >= str.size())
                break;

            // Move on
            if (pos_found == std::string_view::npos)
                break;
            pos = pos_found + delimiter.size();
        }

        return parts;
    }

    /**
     * Decodes an URL.
     *
     * @details This function replaces %<hex> with the corresponding characters.
     *          See https://en.wikipedia.org/wiki/Percent-encoding
     *
     * @note As the replaced characters are "shorter" than the original input we can perform the
     *       replacement in-place as long as we're somewhat careful not to fuck up.
     *
     * @param str The string to decode.
     */
    static
    inline
    void
    url_decode(std::string& str)
    {
        size_t w = 0;
        for (size_t r = 0 ; r < str.size() ; ++r) {
            uint8_t v = str[r];
            if (str[r] == '%') {
                v = hex2dec(str[++r]) << 4;
                v |= hex2dec(str[++r]);
            }
            str[w++] = v;
        }
        str.resize(w);
    }

    /**
     * Attempts to return a suitable MIME-TYPE for a provided file path
     *
     * @param path The file path.
     * @return The corresponding MIME-TYPE.
     */
    // ToDo: This is still a left-over from the very first day that malloy was started as a PoC. This needs some
    //       serious overhaul. See issue #4.
    [[nodiscard]]
    static
    inline
    std::string_view
    mime_type(const std::filesystem::path& path)
    {
        // Extract file extension
        const std::filesystem::path& ext = path.extension();

        if (ext == ".7z")       return "application/x-7z-compressed";
        if (ext == ".bin")      return "application/octet-stream";
        if (ext == ".bmp")      return "image/bmp";
        if (ext == ".bz")       return "application/x-bzip";
        if (ext == ".bz2")      return "application/x-bzip2";
        if (ext == ".css")      return "text/css";
        if (ext == ".css.min")  return "text/css";
        if (ext == ".csv")      return "text/csv";
        if (ext == ".gif")      return "image/gif";
        if (ext == ".gz")       return "application/gzip";
        if (ext == ".ico")      return "image/vnd.microsoft.icon";
        if (ext == ".htm")      return "text/html";
        if (ext == ".html")     return "text/html";
        if (ext == ".mp3")      return "audio/mpeg";
        if (ext == ".mp4")      return "video/mp4";
        if (ext == ".mpeg")     return "video/mpeg";
        if (ext == ".rar")      return "application/vnd.rar";
        if (ext == ".php")      return "text/html";
        if (ext == ".txt")      return "text/plain";
        if (ext == ".js")       return "application/javascript";
        if (ext == ".json")     return "application/json";
        if (ext == ".xml")      return "application/xml";
        if (ext == ".swf")      return "application/x-shockwave-flash";
        if (ext == ".flv")      return "video/x-flv";
        if (ext == ".png")      return "image/png";
        if (ext == ".jpe")      return "image/jpeg";
        if (ext == ".jpeg")     return "image/jpeg";
        if (ext == ".jpg")      return "image/jpeg";
        if (ext == ".tiff")     return "image/tiff";
        if (ext == ".tif")      return "image/tiff";
        if (ext == ".svg")      return "image/svg+xml";
        if (ext == ".svgz")     return "image/svg+xml";
        if (ext == ".zip")      return "application/zip";

        // Recommended fall-back
        return "application/octet-stream";
    }

}
