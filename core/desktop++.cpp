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
#include <unordered_set>
#include "spdlog/spdlog.h"

class MainModule {
public:
    // Using explicit constructor and setting up appropriate initializer list
    MainModule()
        : folders_(std::move(folders)),
          interval_(interval_seconds),
          paused_(false),
          running_(false),
          current_index_(0) {}
    
    MainModule() {}

    ~MainModule() {
        stop();
    }

    // Starts the slideshow (looping, non-blocking)
    void start() {
        if (running_) return;
        load_images();
        
        std::lock_guard<std::mutex> lock(mutex_);
        if (image_files_.empty()) {
            std::cerr << "Cannot start: Image list is empty.\n";
            return;
        }
        
        running_ = true;
        paused_ = false;
        worker_ = std::thread([this] { slideshow_loop(); });
    }

    void pause() { 
        paused_ = true;
    }

    void resume() {
        paused_ = false;
        cv_.notify_one();
    }

    void stop() {
        if (!running_) return;
        running_ = false;
        paused_ = false;
        cv_.notify_one();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    void next() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!image_files_.empty()) {
            current_index_ = randnum_locked();
            set_wallpaper_internal(image_files_[current_index_]);
        }
    }

    void previous() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!image_files_.empty()) {
            // Guarding against underflow safely
            current_index_ = (current_index_ == 0) ? image_files_.size() - 1 : current_index_ - 1;
            set_wallpaper_internal(image_files_[current_index_]);
        }
    }

    void set_interval(int seconds) {
        std::lock_guard<std::mutex> lock(mutex_);
        interval_ = seconds;
        cv_.notify_one();
    }

    void set_wallpaper(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(mutex_);
        set_wallpaper_internal(filepath);
    }

    bool is_paused() const { return paused_; }
    bool is_running() const { return running_; }

    void toggle_orientation() {
        DISPLAY_DEVICEA dd;
        dd.cb = sizeof(dd);
        
        if (EnumDisplayDevicesA(nullptr, 0, &dd, 0)) {
            DEVMODEA dm; 
            ZeroMemory(&dm, sizeof(dm));
            dm.dmSize = sizeof(dm);

            if (EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
                
                if (dm.dmDisplayOrientation == DMDO_DEFAULT) {
                    dm.dmPelsWidth = 1080; 
                    dm.dmPelsHeight = 1920;
                    dm.dmDisplayOrientation = DMDO_90;
                    std::cout << ">> Switching to Portrait\n";
                } else {
                    dm.dmPelsWidth = 1920;
                    dm.dmPelsHeight = 1080;
                    dm.dmDisplayOrientation = DMDO_DEFAULT;
                    std::cout << ">> Switching to Landscape\n";
                }
                
                dm.dmDisplayFrequency = 60;
                dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_ORIENTATION | DM_DISPLAYFREQUENCY; 
            
                LONG result = ChangeDisplaySettingsExA(dd.DeviceName, &dm, nullptr, CDS_FULLSCREEN, nullptr); 
                if (result != DISP_CHANGE_SUCCESSFUL) {
                    std::cerr << "Error: ChangeDisplaySettings failed with code " << result << "\n";
                }
            }
        }
    }

    void set_wallpaper_style(int style) {
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            const wchar_t* styleStr = L"0";
            const wchar_t* tileStr = L"0";

            switch (style) {
                case 0:  styleStr = L"0";  break; // Center
                case 2:  styleStr = L"2";  break; // Stretch
                case 6:  styleStr = L"6";  break; // Fit
                case 10: styleStr = L"10"; break; // Fill
                case 22: styleStr = L"22"; tileStr = L"1"; break; // Span
                default: styleStr = L"2";  break; // Default to Stretch
            }

            RegSetValueExW(hKey, L"WallpaperStyle", 0, REG_SZ, (const BYTE*)styleStr, (DWORD)((wcslen(styleStr) + 1) * sizeof(wchar_t)));
            RegSetValueExW(hKey, L"TileWallpaper", 0, REG_SZ, (const BYTE*)tileStr, (DWORD)((wcslen(tileStr) + 1) * sizeof(wchar_t)));
            RegCloseKey(hKey);
            
            // Forces Windows to refresh registry styles changes instantly
            SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        }
    }

private:
    // Internal helper assuming mutex lock is already acquired
    void set_wallpaper_internal(const std::string& filepath) {
        SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (PVOID)filepath.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    }

    // Modern C++ Thread-safe random range calculation
    size_t randnum_locked() {
        if (image_files_.empty()) return 0;
        std::uniform_int_distribution<size_t> dist(0, image_files_.size() - 1);
        return dist(rng_);
    }

    void load_images() {
        std::lock_guard<std::mutex> lock(mutex_);
        image_files_.clear();

        try {
            auto& config = Config::get();
            bool indexed = config.app.images_indexed;
            std::time_t last_index = config.app.last_indexed_time;
            std::time_t now = std::time(nullptr);

            // 30 days check: 60s * 60m * 24h * 30d
            if (!indexed || difftime(now, last_index) > (60 * 60 * 24 * 30)) {
                for (const auto& folder : folders_) {
                    try {
                        if (!std::filesystem::exists(folder)) continue;
                        
                        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
                            if (entry.is_regular_file()) {
                                std::string ext = entry.path().extension().string();
                                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                                if (allowed_exts.count(ext)) {
                                    image_files_.push_back(entry.path().string());
                                }
                            }
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Failed to parse folder " << folder << ": " << e.what() << "\n";
                    }
                }
                
                // Write out file caching map
                std::ofstream out("images.json");
                if (out.is_open()) {
                    out << "[\n";
                    for (size_t i = 0; i < image_files_.size(); ++i) {
                        // Escaping backslashes for valid JSON formatting outputs
                        std::string path = image_files_[i];
                        size_t pos = 0;
                        while ((pos = path.find('\\', pos)) != std::string::npos) {
                            path.replace(pos, 1, "\\\\");
                            pos += 2;
                        }
                        out << "  \"" << path << "\"";
                        if (i + 1 < image_files_.size()) out << ",";
                        out << "\n";
                    }
                    out << "]\n";
                }
                
                config.app.images_indexed = true;
                config.app.last_indexed_time = now;
                Config::save(config);
            } else {
                // Read from cache fallback configuration
                std::ifstream in("images.json");
                if (in.is_open()) {
                    std::string line;
                    while (std::getline(in, line)) {
                        auto l = line.find('\"');
                        auto r = line.rfind('\"');
                        if (l != std::string::npos && r != std::string::npos && r > l) {
                            std::string path = line.substr(l + 1, r - l - 1);
                            // Unescaping JSON text
                            size_t pos = 0;
                            while ((pos = path.find("\\\\", pos)) != std::string::npos) {
                                path.replace(pos, 2, "\\");
                                pos += 1;
                            }
                            image_files_.push_back(path);
                        }
                    }
                }
            }
            
            if (image_files_.empty()) {
                std::cerr << "Warning: No images loaded.\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error handling image serialization: " << e.what() << "\n";
        }
    }

    void slideshow_loop() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            // Wait while running but paused
            cv_.wait(lock, [this] { return !paused_ || !running_; });
            if (!running_) break;

            if (image_files_.empty()) {
                std::cerr << "Loop Error: Image array pool empty.\n";
                break;
            }

            current_index_ = randnum_locked();
            set_wallpaper_internal(image_files_[current_index_]);

            // Safely capture configuration updates during timed cv wait variables
            int current_interval = interval_;
            if (cv_.wait_for(lock, std::chrono::seconds(current_interval), [this] { return paused_ || !running_; })) {
                continue; // Triggered wakeup by interrupt logic changes
            }
        }
    }

    // Fixed path string initializations down below using forward slashes safely
    std::unordered_set<std::string> allowed_exts = {".jpg", ".jpeg", ".png", ".bmp"};
    std::vector<std::string> folders_ =  {"f:/Pictures/babes/sanskari", "f:/pictures/babes/modern"};
    std::vector<std::string> image_files_;
    int interval_ = 60;
    std::atomic<bool> paused_;
    std::atomic<bool> running_;
    size_t current_index_;

    std::mt19937 rng_; // Safe standard engine for random generations
    std::thread worker_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

/*
How to integrate SlideshowWallManager into your App:

1. Include its header at the top of app.cpp:
    #include "core/desktop++.hpp"

2. Add a SlideshowWallManager instance to your App::State struct, for example:
    std::unique_ptr<SlideshowWallManager> slideshow;

3. In App::startup(), create and start the slideshow manager with desired folders and interval:
    std::vector<std::string> folders = {"f:/Pictures/babes/sanskari", "f:/pictures/babes/modern"};
    state_->slideshow = std::make_unique<SlideshowWallManager>(folders, 60); // change interval as needed
    state_->slideshow->start();

4. To control the slideshow externally, call pause(), resume(), next(), or stop(), for example:
    state_->slideshow->pause();
    state_->slideshow->resume();
    state_->slideshow->next();
    state_->slideshow->stop();

5. In App::shutdown(), ensure to stop and destroy slideshow cleanly:
    if (state_->slideshow) {
        state_->slideshow->stop();
    }
    state_->slideshow.reset();

6. For best practice, expose user configuration (folders, interval) via your existing config mechanism.
*/

/*
int main() {
    // Explicitly initializing structural instances safely 
    std::vector<std::string> target_folders = {"f:/Pictures/babes/sanskari", "f:/pictures/babes/modern"};
    SlideshowWallManager manager(target_folders, 5);
    
    std::cout << "Starting slideshow daemon...\n";
    manager.start();
    
    std::this_thread::sleep_for(std::chrono::seconds(12));
    
    std::cout << "Pausing...\n";
    manager.pause();
    
    std::this_thread::sleep_for(std::chrono::seconds(4));
    
    std::cout << "Resuming and forcing next...\n";
    manager.resume();
    manager.next();
    
    std::this_thread::sleep_for(std::chrono::seconds(10));
    manager.stop();
    return 0;
}
*/