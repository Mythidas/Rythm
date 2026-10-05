#include "rmpch.h"
#include "logger.h"

#include <spdlog/spdlog.h>

namespace rm {
    void Logger::Init() {
        spdlog::set_pattern("[%H:%M:%S][%^%L%$] %v");
    }

    void Logger::Info(std::string_view message) {
        spdlog::info(message);
    }

    void Logger::Warn(std::string_view message) {
        spdlog::warn(message);
    }

    void Logger::Error(std::string_view message) {
        spdlog::error(message);
    }
}
