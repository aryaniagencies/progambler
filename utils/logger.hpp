#pragma once

#include "utils/config.hpp"

namespace utils {

void init_logger(const LoggingConfig &cfg);
void shutdown_logger();

} // namespace utils
