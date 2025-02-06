#pragma once
#include <list>

#include "asyncio.hpp"

#include "http.hpp"
using namespace kwa;


class WebServer {
    private:
        asyncio::Socket _sock {};
        std::list<asyncio::Task<>> _conns {};

        void _handle_connection(int conn) noexcept;
    public:
        WebServer() noexcept = default;
        WebServer(WebServer&) = delete;
        WebServer(WebServer&&) = delete;
        WebServer& operator=(WebServer&) = delete;
        WebServer& operator=(WebServer&&) = delete;
        asyncio::Result<> init(const char* host, short port, int max_listen_num) noexcept;
        asyncio::Task<> run() noexcept;
};
