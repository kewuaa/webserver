#pragma once
#include "asyncio.hpp"
using namespace kwa;


class WebServer {
    private:
        asyncio::Socket _sock {};
    public:
        WebServer() noexcept = default;
        WebServer(WebServer&) = delete;
        WebServer(WebServer&&) = delete;
        WebServer& operator=(WebServer&) = delete;
        WebServer& operator=(WebServer&&) = delete;
        asyncio::Result<> init(const char* host, short port, int max_listen_num) noexcept;
        asyncio::Task<> run() noexcept;
};
