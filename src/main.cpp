#include <getopt.h>

#include "webserver.hpp"


int main(int argc, char** argv) noexcept {
    int c;
    int max_listen_num = 1024;
    short port = 12345;
    while ((c = getopt(argc, argv, ":l:p:h")) != -1) {
        switch (c) {
            case 'l': {
                max_listen_num = std::stoi(optarg);
                break;
            }
            case 'p': {
                port = std::stoi(optarg);
                break;
            }
            case 'h':
            case '?': {
                std::fprintf(stderr, "%s [-p <port>] [-l <max listen num>]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    auto server = WebServer();
    server.init("0.0.0.0", port, max_listen_num);
    asyncio::run(server.run());
}
