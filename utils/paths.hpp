#pragma once

#include <filesystem>

namespace utils {

std::filesystem::path exe_dir();
std::filesystem::path resolve_data_path(const std::string &relative);
std::filesystem::path config_file(const std::string &env);
void ensure_parent_dir(const std::filesystem::path &file);
std::string file_url(const std::filesystem::path &path);

} // namespace utils
