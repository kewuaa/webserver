#include <filesystem>

#include "http/request.hpp"
#include "http/response.hpp"
namespace fs = std::filesystem;


namespace http::request {
    Handler::Handler(asyncio::Socket& sock) noexcept: _sock(sock) {}

    asyncio::Task<> Handler::_handle_get(const request::Request& data) noexcept {
        if (data.path == "/") {
            co_await response::make_response_list_dir(
                _sock,
                _buffer,
                fs::current_path().c_str()
            );
        } else if (data.path == "/list") {
            if (!data.params.contains("cwd") or !fs::is_directory(data.params.at("cwd"))) {
                co_await response::make_response_get_file(
                    _sock,
                    _buffer,
                    RESOURCE_ROOT_DIR"/404.html",
                    StatusCode::NOT_FOUND
                );
            } else {
                co_await response::make_response_list_dir(
                    _sock,
                    _buffer,
                    data.params.at("cwd")
                );
            }
        } else if (data.path == "/view" or data.path == "/download") {
            if (!data.params.contains("file") or !fs::exists(data.params.at("file"))) {
                co_await response::make_response_get_file(
                    _sock,
                    _buffer,
                    RESOURCE_ROOT_DIR"/404.html",
                    StatusCode::NOT_FOUND
                );
            } else {
                if (data.path == "/download") {
                    co_await response::make_response_download_file(
                        _sock,
                        _buffer,
                        data.params.at("file")
                    );
                } else {
                    co_await response::make_response_get_file(
                        _sock,
                        _buffer,
                        data.params.at("file"),
                        StatusCode::OK
                    );
                }
            }
        } else {
            StatusCode code { StatusCode::OK };
            std::string file = RESOURCE_ROOT_DIR;
            file.append(data.path);
            if (!fs::exists(file)) {
                if (fs::is_directory(file)) {
                    file = RESOURCE_ROOT_DIR"/400.html";
                    code = StatusCode::BAD_REQUEST;
                } else {
                    file = RESOURCE_ROOT_DIR"/404.html";
                    code = StatusCode::NOT_FOUND;
                }
            }
            co_await response::make_response_get_file(
                _sock,
                _buffer,
                file,
                code
            );
        }
    }

    asyncio::Task<> Handler::_handle_post(
        request::Request& data,
        std::string_view body
    ) noexcept {
        static std::unordered_map<std::string, FILE*> file_map;
        if (data.path == "/upload" || data.path == "/upload-done") {
            if (!data.headers.contains("X-file-name") || !data.headers.contains("X-cwd")) {
                co_await response::make_response_json(
                    _sock,
                    _buffer,
                    "`X-file-name` and `X-cwd` header is needed"
                );
                co_return;
            }
            std::string_view cwd = data.headers.at("X-cwd");
            std::string_view file_name = data.headers.at("X-file-name");
            auto path = std::format("{}/{}", cwd, file_name);

            if (data.path == "/upload-done") {
                if (!file_map.contains(path)) {
                    SPDLOG_WARN("`{}` not register in file ptr map", path);
                    co_return;
                }
                SPDLOG_DEBUG("file fp for `{}` closed", path);
                fclose(file_map[path]);
                file_map.erase(std::move(path));
                co_await response::make_response_json(_sock, _buffer, "");
                co_return;
            }

            if (!file_map.contains(path)) {
                if (!fs::is_directory(cwd)) {
                    co_await response::make_response_json(
                        _sock,
                        _buffer,
                        "invalid directory"
                    );
                    co_return;
                }
                FILE* f = fopen(path.c_str(), "wb");
                SPDLOG_DEBUG("open `{}`", path);
                file_map[path] = f;
            }

            SPDLOG_DEBUG("write {} bytes to `{}`", body.size(), path);
            fwrite(body.data(), sizeof(char), body.size(), file_map[std::move(path)]);
            if (data.content_length() == data.body_size) {
                co_await response::make_response_json(_sock, _buffer, "");
            }
        } else {
            co_await response::make_response_get_file(
                _sock,
                _buffer,
                RESOURCE_ROOT_DIR"/404.html",
                StatusCode::NOT_FOUND
            );
        }
    }

    asyncio::Task<> Handler::handle_request(request::Request& data, std::string_view body) noexcept {
        if (!data.valid) {
            co_await response::make_response_get_file(
                _sock,
                _buffer,
                RESOURCE_ROOT_DIR"/400.html",
                StatusCode::BAD_REQUEST
            );
        } else {
            switch (data.method) {
                case Method::GET:
                    co_await _handle_get(data);
                    break;
                case Method::POST:
                    co_await _handle_post(data, body);
                    break;
                default:
                    co_await response::make_response_error_text(
                        _sock, _buffer,
                        std::format("Unsupported method `{}`", data.method_str())
                    );
                    break;
            }
        }
    }
}
