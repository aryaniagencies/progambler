#pragma once

#include "utils/config.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <functional>

class TrayService {
public:
    using QuitCallback = std::function<void()>;

    TrayService();
    ~TrayService();

    void init(HWND main_window, const TrayConfig &cfg, QuitCallback on_quit);
    void install_minimize_hook(bool enabled);
    void show_main_window();
    void remove();

private:
    static LRESULT CALLBACK message_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT CALLBACK window_hook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void show_context_menu();

    inline static TrayService *instance_{nullptr};

    HWND main_window_{nullptr};
    HWND message_window_{nullptr};
    HWND hook_window_{nullptr};
    WNDPROC original_proc_{nullptr};
    QuitCallback on_quit_;
    bool installed_{false};
    NOTIFYICONDATAW icon_{};
};
