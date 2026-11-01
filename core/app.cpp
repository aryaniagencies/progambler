#include "core/app.hpp"

#include "services/storage_service.hpp"
#include "services/tray_service.hpp"
#include "services/Image.hpp"
#include "ui/main_window.hpp"
#include "utils/config.hpp"
#include "utils/logger.hpp"
#include "utils/paths.hpp"

#include <memory>
#include <spdlog/spdlog.h>

#include "core/desktop++.hpp"

struct App::State {
    Config config;
    std::unique_ptr<StorageService> storage;
    std::unique_ptr<MainWindow> window;
    std::unique_ptr<TrayService> tray;
    std::unique_ptr<MainModule> mod; // change this
};

int App::run() {
    startup();

    state_->window->navigate_frontend();

    if (state_->config.tray.enabled) {
        const HWND hwnd = state_->window->native_handle();
        state_->tray->init(hwnd, state_->config.tray, [this] {
            state_->window->terminate();
        });
        state_->tray->install_minimize_hook(state_->config.tray.minimize_to_tray);
    }

    spdlog::info("entering main loop");
    state_->window->run();

    shutdown();
    return 0;
}

void App::startup() {
    state_ = new State{};
    state_->config = Config::load();

    utils::init_logger(state_->config.logging);
    spdlog::info("starting {} [{}]", state_->config.app.name, state_->config.environment);

    const auto storage_path = utils::resolve_data_path(state_->config.storage.path);
    state_->storage = std::make_unique<StorageService>(storage_path);
    state_->window = std::make_unique<MainWindow>(state_->config.app, *state_->storage);
    state_->tray = std::make_unique<TrayService>();

    state_->mod->start(); // change this
}

void App::shutdown() {
    if (!state_) {
        return;
    }

    spdlog::info("shutting down");
    if (state_->tray) {
        state_->tray->remove();
    }
    state_->tray.reset();
    state_->window.reset();
    state_->storage.reset();
    utils::shutdown_logger();

    state_->mod->stop(); // change this

    delete state_;
    state_ = nullptr;
}
