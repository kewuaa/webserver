#include <fstream>
#include <filesystem>

#include "http/response.hpp"
namespace fs = std::filesystem;


namespace http::response {
    static void make_status_line(Buffer& buffer, StatusCode code) noexcept {
        buffer.write(
            "HTTP/1.1 {} {}" CRLF,
            std::to_string(magic_enum::enum_integer(code)),
            status_message(code)
        );
    }

    void make_response_get_file(
        Buffer& buf,
        std::string_view file,
        StatusCode code
    ) noexcept {
        auto file_size = fs::file_size(file);
        make_status_line(buf, code);
        buf.write("Connection: keep-alive" CRLF);
        buf.write("Keep-Alive: timeout={}" CRLF, CONNECTION_TIMEOUT / 1000);
        buf.write(
            "Content-Type: {}; charset=utf-8" CRLF,
            get_content_type(file)
        );

        buf.write("content-length: {}" CRLF CRLF, file_size);
        {
            auto view = buf.malloc(file_size);
            std::ifstream f;
            f.open(file.data(), std::ios::binary);
            f.read(view.data(), view.size());
            f.close();
        }
    }

    void make_response_list_dir(Buffer &buf, std::string_view dir) noexcept {
        make_status_line(buf, StatusCode::OK);
        buf.write("Connection: keep-alive" CRLF);
        buf.write("Keep-Alive: timeout={}" CRLF, CONNECTION_TIMEOUT / 1000);
        buf.write("Content-Type: text/html; charset=utf-8" CRLF);

        {
            buf.write("Content-Length: ");
            auto length_view = buf.write("xxxxxxxxxx");
            buf.write(CRLF CRLF);
            auto size = buf.readable_bytes();

            buf.write(
                "<!DOCTYPE html>" CRLF
                "<html>" CRLF
                "<head lang=en>" CRLF
                "<meta charset=\"UTF-8\">" CRLF
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" CRLF
                "<title>Dirctory list for {}</title>" CRLF
                "<link rel=\"stylesheet\" href=\"/css/style.css\">" CRLF
                "</head>" CRLF
                "<body>" CRLF
                "<hl>Directory list for {}</hl>" CRLF
                "<hr>" CRLF
                "<ul>" CRLF, dir, dir
            );

            fs::path path { dir };
            buf.write(
                "<li><a href=\"/list?cwd={}\">..</a></li>" CRLF,
                path.parent_path().c_str()
            );
            for (auto& entry : fs::directory_iterator(path)) {
                if (entry.is_directory()) {
                    buf.write(
                        "<li><a href=\"/list?cwd={}\">{}</a></li>" CRLF,
                        entry.path().c_str(),
                        entry.path().c_str()
                    );
                } else {
                    buf.write(
                        "<li>" CRLF
                        "<a href=\"/view?file={}\">{}</a>" CRLF
                        "<button value=\"{}\" id=\"download_button\">download</button>" CRLF
                        "</li>" CRLF,
                        entry.path().c_str(),
                        entry.path().c_str(),
                        entry.path().c_str()
                    );
                }
            }

            buf.write(
                "</ul>" CRLF
                "<hr>" CRLF
                "<script src=\"/js/event.js\"></script>" CRLF
                "</body>" CRLF
                "</html>" CRLF
            );
            size = buf.readable_bytes() - size;
            auto content_length = std::format("{:10}", size);
            std::copy(
                content_length.begin(),
                content_length.end(),
                length_view.data()
            );
        }
    }

    void make_response_download_file(Buffer &buf, std::string_view file) noexcept {
        auto file_size = fs::file_size(file);
        make_status_line(buf, StatusCode::OK);
        buf.write("Connection: close" CRLF);
        buf.write("Content-Type: application/octet-stream" CRLF);

        buf.write("content-length: {}" CRLF CRLF, file_size);
        {
            auto view = buf.malloc(file_size);
            std::ifstream f;
            f.open(file.data(), std::ios::binary);
            f.read(view.data(), view.size());
            f.close();
        }
    }
}
