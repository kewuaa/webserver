#include "webserver.hpp"
#include "http.hpp"


void WebServer::init(
    const char* host,
    short port,
    int max_listen_num
) noexcept {
    auto res = _sock.bind(host, port);
    if (res == -1) {
        SPDLOG_ERROR("failed to bind to {}:{}", host, port);
        exit(EXIT_FAILURE);
    }
    SPDLOG_INFO("successfully bind to {}:{}", host, port);
    res = _sock.listen(max_listen_num);
    if (res == -1) {
        SPDLOG_ERROR("failed to starting listening");
        exit(EXIT_FAILURE);
    }
    SPDLOG_INFO("start listening, max listen number: {}", max_listen_num);
}


asyncio::Task<> WebServer::run() noexcept {
    while (true) {
        auto conn = co_await _sock.accept();
        SPDLOG_INFO("accept connection to socket fd {}", conn);
        http::init_connection(conn);
    }
}
