#include "utils/paths.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <shlwapi.h>

namespace utils {

std::filesystem::path exe_dir() {
    wchar_t buf[MAX_PATH]{};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf).parent_path();
}

std::filesystem::path resolve_data_path(const std::string &relative) {
    return exe_dir() / relative;
}

std::filesystem::path config_file(const std::string &env) {
    return exe_dir() / "config" / ("app." + env + ".json");
}

void ensure_parent_dir(const std::filesystem::path &file) {
    if (file.has_parent_path()) {
        std::filesystem::create_directories(file.parent_path());
    }
}

std::string file_url(const std::filesystem::path &path) {
    auto url = std::filesystem::absolute(path).string();
    for (auto &c : url) {
        if (c == '\\') {
            c = '/';
        }
    }
    return "file:///" + url;
}

} // namespace utils
