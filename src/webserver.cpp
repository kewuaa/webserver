#include "webserver.hpp"
#include "http.hpp"


asyncio::Result<> WebServer::init(
    const char* host,
    short port,
    int max_listen_num
) noexcept {
    auto res = _sock.bind(host, port);
    if (!res) {
        return asyncio::Error(std::move(res.error()));
    }
    res = _sock.listen(max_listen_num);
    if (!res) {
        return asyncio::Error(std::move(res.error()));
    }
    return {};
}


asyncio::Task<> WebServer::run() noexcept {
    while (true) {
        auto conn = co_await _sock.accept();
        SPDLOG_INFO("accept connection to socket fd {}", conn);
        http::init_connection(conn);
    }
}
