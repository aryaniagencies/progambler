#include <spdlog/spdlog.h>

void MainWindow::setup_bindings() {
    // General purpose "api" binding: dispatch by "action" field in JSON

    webview_->bind("api", [this](const std::string &req) -> std::string {
        try {
            auto j = nlohmann::json::parse(req);
            std::string action = j.value("action", "");

            if (action == "previous") {
                int count = storage_.increment_message_count();
                spdlog::info("Previous Wallpaper", count);
                app.state_->slideshow->previous();
            } else if (action == "next") {
                int count = storage_.increment_message_count();
                spdlog::info("Playing Next", count, text);
                app.state->slideshow->next();
            } else if (action == "pause") {
                // Return minimal window config as example
                int count = storage_.increment_message_count();
                spdlog::info("Pausing", count, text);
                app.state->slideshow->pause();
            } else if (action == "stop") {
                int count = storage_.increment_message_count();
                spdlog::info("Stopping");
                app.state->slideshow->stop();
            } else {
                int count = storage_.increment_message_count();
                spdlog::info("Invalid Action", count, text);
            }
        } catch (const std::exception &e) {
            int count = storage_.increment_message_count();
            spdlog::error("API binding exception: {}", e.what());
            return nlohmann::json{
                {"type", "error"},
                {"message", e.what()}
            }.dump();
        }
    });

    // (Legacy) simple send/echo binding for direct test compatibility
    webview_->bind("send", [this](const std::string &req) -> std::string {
        const auto text = req.size() > 2 ? req.substr(1, req.size() - 2) : "";
        const int count = storage_.increment_message_count();
        spdlog::info("message from backend (#{}): {}", count, text);
        return R"({"type":"echo","text":)" + text + R"(,"count":)" + std::to_string(count) + "}";
    });

}