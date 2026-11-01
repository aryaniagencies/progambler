#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <exception>

namespace utils {

inline int run_safe(int (*main_fn)()) {
    try {
        return main_fn();
    } catch (const std::exception &e) {
        MessageBoxA(nullptr, e.what(), "Application Error", MB_ICONERROR | MB_OK);
        return 1;
    } catch (...) {
        MessageBoxA(nullptr, "Unknown error", "Application Error", MB_ICONERROR | MB_OK);
        return 1;
    }
}

} // namespace utils
