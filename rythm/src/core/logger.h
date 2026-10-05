#pragma once
#include <string_view>

namespace rm {
    class Logger {
    public:
        static void Init();

        static void Info(std::string_view message);
        static void Warn(std::string_view message);
        static void Error(std::string_view message);
    };

#define RM_LOG_INFO(...) ::rm::Logger::Info(std::format(__VA_ARGS__))
#define RM_LOG_WARN(...) ::rm::Logger::Warn(std::format(__VA_ARGS__))
#define RM_LOG_ERROR(...) ::rm::Logger::Error(std::format(__VA_ARGS__))
}
