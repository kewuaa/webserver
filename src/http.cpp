#include <fstream>
#include <filesystem>

#include "http.hpp"
#include "config.hpp"
namespace fs = std::filesystem;;


namespace http {
    using namespace std::chrono;

    static asyncio::Task<> handle_GET(asyncio::Socket& sock, Buffer& buf, Request::Data& data) noexcept {
        if (data.path == "/") {
            co_await response::make_response_list_dir(
                sock,
                buf,
                fs::current_path().c_str()
            );
        } else if (data.path == "/list") {
            if (!data.params.contains("cwd") or !fs::is_directory(data.params["cwd"])) {
                co_await response::make_response_get_file(
                    sock,
                    buf,
                    RESOURCE_ROOT_DIR"/404.html",
                    StatusCode::NOT_FOUND
                );
            } else {
                co_await response::make_response_list_dir(
                    sock,
                    buf,
                    data.params["cwd"]
                );
            }
        } else if (data.path == "/view" or data.path == "/download") {
            if (!data.params.contains("file") or !fs::exists(data.params["file"])) {
                co_await response::make_response_get_file(
                    sock,
                    buf,
                    RESOURCE_ROOT_DIR"/404.html",
                    StatusCode::NOT_FOUND
                );
            } else {
                if (data.path == "/download") {
                    co_await response::make_response_download_file(
                        sock,
                        buf,
                        data.params["file"]
                    );
                } else {
                    co_await response::make_response_get_file(
                        sock,
                        buf,
                        data.params["file"],
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
                sock,
                buf,
                file,
                code
            );
        }
    }

    asyncio::Task<> init_connection(int conn) noexcept {
        bool closed = false;
        auto close_connection = [&closed] {
            SPDLOG_INFO("connection timeout, cloesd");
            closed = true;
        };
        auto& loop = asyncio::EventLoop::get();
        // initialize timer
        auto timer = loop.call_later(
            milliseconds(CONNECTION_TIMEOUT),
            close_connection
        );

        Request req;
        Buffer buf;
        asyncio::Socket sock { conn };
        char buffer[BUFFER_SIZE];
        while (!closed) {
            auto res = co_await sock.read(buffer, BUFFER_SIZE);
            if (!res) {
                timer->cancel();
               break;
            }
            if (closed) {
                close(conn);
                break;
            }
            // reset timer
            timer->cancel();
            timer = loop.call_later(
                milliseconds(CONNECTION_TIMEOUT),
                close_connection
            );

            auto nbytes = *res;
            buffer[nbytes] = '\0';
            SPDLOG_DEBUG("recv {} bytes data:\n{}", nbytes, buffer);
            req.parse(buffer, nbytes);
            while (req.has_data()) {
                if (auto data = req.get(); data) {
                    if (!data->valid) {
                        co_await response::make_response_get_file(
                            sock,
                            buf,
                            RESOURCE_ROOT_DIR"/400.html",
                            StatusCode::BAD_REQUEST
                        );
                        continue;
                    }
                    SPDLOG_DEBUG("method: {}", data->method_str());
                    SPDLOG_DEBUG("path: {}", data->path);
                    for (auto& [key, value] : data->params) {
                        SPDLOG_DEBUG("{} -> {}", key, value);
                    }
                    SPDLOG_DEBUG("version: {}", data->version);
                    for (auto& [key, value] : data->headers) {
                        SPDLOG_DEBUG("{} -> {}", key, value);
                    }

                    if (data->method == Method::GET) {
                        co_await handle_GET(sock, buf, *data);
                    }
                }
            }
        }
    }
}
