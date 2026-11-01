#include "services/storage_service.hpp"

#include "utils/paths.hpp"

#include <fstream>
#include <spdlog/spdlog.h>

StorageService::StorageService(std::filesystem::path path) : path_(std::move(path)) {
    utils::ensure_parent_dir(path_);
    if (std::filesystem::exists(path_)) {
        std::ifstream in(path_);
        data_ = nlohmann::json::parse(in, nullptr, false);
        if (data_.is_discarded()) {
            spdlog::warn("storage file corrupt, resetting: {}", path_.string());
            data_ = nlohmann::json::object();
        }
    } else {
        data_ = nlohmann::json::object();
        data_["message_count"] = 0;
        save();
    }
    spdlog::debug("storage loaded: {}", path_.string());
}

nlohmann::json &StorageService::data() { return data_; }

void StorageService::save() {
    std::ofstream out(path_);
    out << data_.dump(2);
    spdlog::debug("storage saved: {}", path_.string());
}

int StorageService::increment_message_count() {
    const int count = data_.value("message_count", 0) + 1;
    data_["message_count"] = count;
    save();
    return count;
}
