#include <fstream>
#include <filesystem>
#include <boost/ut.hpp>

#include "growable_buffer.hpp"
using namespace boost::ut;


int main() {
    "load_404_page"_test = [] {
        GrowableBuffer buf;
        buf.write("HTTP/1.1 200 OK\r\n");
        auto path = std::filesystem::path(__FILE__)
            .parent_path()
            .parent_path()
            .concat("/resources/404.html");
        auto file_size = std::filesystem::file_size(path);
        auto view = buf.malloc(file_size);
        std::ifstream f;
        f.open(path, std::ios::binary);
        f.read(view.data(), view.size());
    };
}
