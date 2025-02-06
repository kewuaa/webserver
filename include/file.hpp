#pragma once
#include <vector>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;


inline std::vector<std::string> iter_dirs(std::string_view cwd) noexcept {
    fs::path path { cwd };
}
