#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <random>
#include <cstdlib>
#include <fstream>
#include <algorithm>

// NOTE: Config is assumed external
// struct Config { ... };
// Config Config::get(); Config::save();

class MainModule {
public:
    MainModule(const std::vector<std::string>& folders, int interval_seconds = 60);
    ~MainModule();

    void start();
    void pause();
    void resume();
    void stop();

    void next();
    void previous();

    void set_interval(int seconds);
    void set_wallpaper(const std::string& filepath);

    bool is_paused() const;
    bool is_running() const;

private:
    void slideshow_loop();
    void load_images();

    void toggleorientation();
    void SetWallpaperStyle(int style);
    void setdefaultwallpaper();
    int randnum();

private:
    std::vector<std::string> allowed_exts = {".jpg", ".jpeg", ".png", ".bmp"};
    std::vector<std::string> folders_;
    std::vector<std::string> image_files_;

    int interval_ = 60;

    std::atomic<bool> paused_{false};
    std::atomic<bool> running_{false};

    size_t current_index_ = 0;

    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;
};



// ================= IMPLEMENTATION =================
/*
inline MainModule::MainModule(
    const std::vector<std::string>& folders,
    int interval_seconds
) : folders_(folders), interval_(interval_seconds) {}

inline MainModule::~MainModule() {
    stop();
}

inline void MainModule::start() {
    if (running_) return;

    load_images();
    if (image_files_.empty()) return;

    running_ = true;
    paused_ = false;

    worker_ = std::thread([this] { slideshow_loop(); });
}

inline void MainModule::pause() {
    paused_ = true;
}

inline void MainModule::resume() {
    paused_ = false;
    cv_.notify_one();
}

inline void MainModule::stop() {
    running_ = false;
    cv_.notify_one();
    if (worker_.joinable()) worker_.join();
}

inline bool MainModule::is_paused() const {
    return paused_;
}

inline bool MainModule::is_running() const {
    return running_;
}

inline void MainModule::set_wallpaper(const std::string& filepath) {
    SystemParametersInfoA(
        SPI_SETDESKWALLPAPER,
        0,
        (PVOID)filepath.c_str(),
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
    );
}
*/