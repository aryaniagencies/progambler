#pragma once

#include <nlohmann/json.hpp>
#include <string>

struct AppConfig {
    std::string name{"WebView2 App"};
    int window_width{960};
    int window_height{640};
};

struct LoggingConfig {
    std::string level{"info"};
    std::string file{"logs/app.log"};
};

struct StorageConfig {
    std::string path{"data/app.json"};
};

struct TrayConfig {
    bool enabled{true};
    bool minimize_to_tray{true};
    std::string tooltip{"WebView2 App"};
};

struct Config {
    std::string environment{"dev"};
    AppConfig app;
    LoggingConfig logging;
    StorageConfig storage;
    TrayConfig tray;

    static Config load();
};

void from_json(const nlohmann::json &j, AppConfig &c);
void from_json(const nlohmann::json &j, LoggingConfig &c);
void from_json(const nlohmann::json &j, StorageConfig &c);
void from_json(const nlohmann::json &j, TrayConfig &c);
