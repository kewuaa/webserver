#include "webserver.hpp"


int main() noexcept {
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    auto server = WebServer();
    auto res = server.init("0.0.0.0", 12345, 10);
    if (!res) {
        SPDLOG_ERROR(res.error().message());
        exit(EXIT_FAILURE);
    }
    res = asyncio::run(server.run());
    if (!res) {
        SPDLOG_ERROR(res.error().message());
        exit(EXIT_FAILURE);
    }
}
