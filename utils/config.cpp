#include "utils/config.hpp"

#include "utils/paths.hpp"

#include <cstdlib>
#include <fstream>
#include <stdexcept>

// Helper: detect environment
static std::string detect_environment() {
    if (const char *env = std::getenv("APP_ENV")) {
        return env;
    }
#ifdef APP_ENV_DEFAULT
    return APP_ENV_DEFAULT;
#else
    return "dev";
#endif
}

// JSON adapters
void from_json(const nlohmann::json &j, AppConfig &c) {
    c.name = j.value("name", c.name);
    if (j.contains("window")) {
        c.window_width = j["window"].value("width", c.window_width);
        c.window_height = j["window"].value("height", c.window_height);
    }
}

void from_json(const nlohmann::json &j, LoggingConfig &c) {
    c.level = j.value("level", c.level);
    c.file = j.value("file", c.file);
}

void from_json(const nlohmann::json &j, StorageConfig &c) {
    c.path = j.value("path", c.path);
}

void from_json(const nlohmann::json &j, TrayConfig &c) {
    c.enabled = j.value("enabled", c.enabled);
    c.minimize_to_tray = j.value("minimize_to_tray", c.minimize_to_tray);
    c.tooltip = j.value("tooltip", c.tooltip);
}

Config Config::load() {
    Config conf;
    conf.environment = detect_environment();

    const auto path = utils::config_file(conf.environment);
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("config not found: " + path.string());
    }
    const auto j = nlohmann::json::parse(in);

    if (j.contains("app")) {
        conf.app = j["app"].get<AppConfig>();
    }
    if (j.contains("logging")) {
        conf.logging = j["logging"].get<LoggingConfig>();
    }
    if (j.contains("storage")) {
        conf.storage = j["storage"].get<StorageConfig>();
    }
    if (j.contains("tray")) {
        conf.tray = j["tray"].get<TrayConfig>();
    }
    return conf;
}
