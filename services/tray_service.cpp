#include "services/tray_service.hpp"

#include <shellapi.h>
#include <spdlog/spdlog.h>

namespace {

constexpr UINT WM_TRAY = WM_USER + 1;
constexpr UINT ID_TRAY = 1;
constexpr UINT ID_OPEN = 1001;
constexpr UINT ID_QUIT = 1002;

} // namespace

TrayService::TrayService() { instance_ = this; }

TrayService::~TrayService() { remove(); }

void TrayService::init(HWND main_window, const TrayConfig &cfg, QuitCallback on_quit) {
    main_window_ = main_window;
    on_quit_ = std::move(on_quit);
    hook_window_ = main_window;

    WNDCLASSW wc{};
    wc.lpfnWndProc = message_proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"WebView2AppTrayHost";
    RegisterClassW(&wc);

    message_window_ = CreateWindowExW(
        0, wc.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, nullptr);

    icon_ = {};
    icon_.cbSize = sizeof(icon_);
    icon_.hWnd = message_window_;
    icon_.uID = ID_TRAY;
    icon_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    icon_.uCallbackMessage = WM_TRAY;
    icon_.hIcon = LoadIconW(nullptr, reinterpret_cast<LPCWSTR>(32512)); // IDI_APPLICATION
    MultiByteToWideChar(CP_UTF8, 0, cfg.tooltip.c_str(), -1, icon_.szTip, ARRAYSIZE(icon_.szTip));

    Shell_NotifyIconW(NIM_ADD, &icon_);
    installed_ = true;
    spdlog::info("system tray initialized");
}

void TrayService::install_minimize_hook(bool enabled) {
    if (!enabled || !main_window_ || original_proc_) {
        return;
    }
    original_proc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(
        hook_window_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(window_hook)));
    spdlog::debug("minimize-to-tray hook installed");
}

void TrayService::show_main_window() {
    if (!main_window_) {
        return;
    }
    ShowWindow(main_window_, SW_SHOW);
    SetForegroundWindow(main_window_);
}

void TrayService::remove() {
    if (installed_) {
        Shell_NotifyIconW(NIM_DELETE, &icon_);
        installed_ = false;
    }
    if (original_proc_ && hook_window_) {
        SetWindowLongPtrW(hook_window_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_proc_));
        original_proc_ = nullptr;
    }
    if (message_window_) {
        DestroyWindow(message_window_);
        message_window_ = nullptr;
    }
}

void TrayService::show_context_menu() {
    POINT pt{};
    GetCursorPos(&pt);

    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, ID_OPEN, L"Open");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_QUIT, L"Quit");

    SetForegroundWindow(message_window_);
    TrackPopupMenu(menu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, message_window_, nullptr);
    DestroyMenu(menu);
}

LRESULT CALLBACK TrayService::message_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto *self = instance_;
    if (!self) {
        return DefWindowProcW(hwnd, msg, wp, lp);
    }

    if (msg == WM_TRAY) {
        if (lp == WM_LBUTTONDBLCLK) {
            self->show_main_window();
            return 0;
        }
        if (lp == WM_RBUTTONUP) {
            self->show_context_menu();
            return 0;
        }
    }

    if (msg == WM_COMMAND) {
        switch (LOWORD(wp)) {
        case ID_OPEN:
            self->show_main_window();
            return 0;
        case ID_QUIT:
            spdlog::info("quit requested from tray");
            if (self->on_quit_) {
                self->on_quit_();
            }
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT CALLBACK TrayService::window_hook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto *self = instance_;
    if (!self || !self->original_proc_) {
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    if (msg == WM_SIZE && wp == SIZE_MINIMIZED) {
        ShowWindow(hwnd, SW_HIDE);
        spdlog::debug("window minimized to tray");
        return 0;
    }
    return CallWindowProcW(self->original_proc_, hwnd, msg, wp, lp);
}
