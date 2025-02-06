#pragma once
#include <asyncio.hpp>

#include "request.hpp"
#include "buffer.hpp"
using namespace kwa;


namespace http::response {
    constexpr const char* PAGE_FMT = "";

    constexpr const char* get_content_type(std::string_view file) noexcept {
        const char* type = "text/plain";
        auto dot_pos = file.rfind('.');
        if (dot_pos == std::string_view::npos) {
            return type;
        }
        auto suffix = file.substr(dot_pos);
        if (suffix == ".txt") {
            // type = "text/plain";
        } else if (suffix == ".html") {
            type = "text/html";
        } else if (suffix == ".xml") {
            type = "text/xml";
        } else if (suffix == ".js") {
            type = "text/javascript";
        } else if (suffix == ".css") {
            type = "text/css";
        } else if (suffix == ".png") {
            type = "image/png";
        } else if (suffix == ".gif") {
            type = "image/gif";
        } else if (suffix == ".jpg" or suffix == ".jpeg") {
            type = "image/jpg";
        } else if (suffix == ".mpg" or suffix == ".mpeg") {
            type = "video/mpeg";
        } else if (suffix == ".avi") {
            type = "video/x-msvideo";
        } else if (suffix == ".au") {
            type = "audio/basic";
        } else if (suffix == ".xhtml") {
            type = "application/xhtml+xml";
        } else if (suffix == ".rtf") {
            type = "application/rtf";
        } else if (suffix == ".word") {
            type = "application/nsword";
        } else if (suffix == ".pdf") {
            type = "application/pdf";
        } else if (suffix == ".gz") {
            type = "application/x-gzip";
        } else if (suffix == ".tar") {
            type = "application/x-tar";
        }
        return type;
    }

    void make_response_get_file(Buffer& buf, std::string_view file, StatusCode code) noexcept;
    void make_response_list_dir(Buffer& buf, std::string_view dir) noexcept;
    void make_response_download_file(Buffer& buf, std::string_view file) noexcept;
}
