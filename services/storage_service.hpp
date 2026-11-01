#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <string>

class StorageService {
public:
    explicit StorageService(std::filesystem::path path);

    nlohmann::json &data();
    void save();
    int increment_message_count();

private:
    std::filesystem::path path_;
    nlohmann::json data_;
};
