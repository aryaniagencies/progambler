#include "utils/logger.hpp"

#include "utils/paths.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace {

spdlog::level::level_enum parse_level(const std::string &level) {
    if (level == "trace") return spdlog::level::trace;
    if (level == "debug") return spdlog::level::debug;
    if (level == "warn") return spdlog::level::warn;
    if (level == "error") return spdlog::level::err;
    if (level == "critical") return spdlog::level::critical;
    return spdlog::level::info;
}

} // namespace

namespace utils {

void init_logger(const LoggingConfig &cfg) {
    const auto log_path = resolve_data_path(cfg.file);
    ensure_parent_dir(log_path);

    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path.string(), true);

    auto logger = std::make_shared<spdlog::logger>(
        "app", spdlog::sinks_init_list{console, file});
    logger->set_level(parse_level(cfg.level));
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    logger->flush_on(spdlog::level::warn);

    spdlog::set_default_logger(logger);
    spdlog::info("logging initialized -> {}", log_path.string());
}

void shutdown_logger() {
    spdlog::info("shutting down logger");
    spdlog::shutdown();
}

} // namespace utils
