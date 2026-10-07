#pragma once

#include <memory>
#include <format>

namespace rm {
    // Memory

    template <typename T>
    using Ref = std::shared_ptr<T>;

    template <typename T, typename... Args>
    constexpr Ref<T> CreateRef(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }

    template <typename T>
    using Scope = std::unique_ptr<T>;

    template <typename T, typename... Args>
    constexpr Scope<T> CreateScope(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }

    // Debugging

    #define RM_ASSERT(condition, ...)                         \
    do {                                                      \
        if (!(condition)) {                                   \
            RM_LOG_ERROR(                                    \
                "Assertion failed!\n"                         \
                "  Expression: {}\n"                          \
                "  Message: {}\n"                             \
                "  File: {}:{}\n"                             \
                "  Function: {}",                             \
                #condition,                                   \
                std::format(__VA_ARGS__),                     \
                __FILE__,                                     \
                __LINE__,                                     \
                __func__                                      \
            );                                                \
            __builtin_trap();                                 \
        }                                                     \
    } while (false)
}
