#include "http.hpp"
#include "config.hpp"


namespace http {
    using namespace std::chrono;

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

        request::Parser parser;
        asyncio::Socket sock { conn };
        request::Handler handler { sock };
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
            SPDLOG_DEBUG("recv {} bytes data", nbytes);
            co_await parser.process(buffer, nbytes, handler);
        }
    }
}
