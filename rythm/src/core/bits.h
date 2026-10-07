#pragma once

#include <type_traits>

namespace rm {
    template <typename E>
    constexpr bool HasFlag(E value, E flag) noexcept {
        using T = std::underlying_type_t<E>;
        return (static_cast<T>(value) & static_cast<T>(flag)) != 0;
    }

    #define RM_BIT_ENUM(EnumType)                                            \
    constexpr EnumType operator|(EnumType lhs, EnumType rhs) noexcept {      \
        using T = std::underlying_type_t<EnumType>;                          \
        return static_cast<EnumType>(                                        \
            static_cast<T>(lhs) | static_cast<T>(rhs)                        \
        );                                                                   \
    }                                                                        \
                                                                             \
    constexpr EnumType operator&(EnumType lhs, EnumType rhs) noexcept {      \
        using T = std::underlying_type_t<EnumType>;                          \
        return static_cast<EnumType>(                                        \
            static_cast<T>(lhs) & static_cast<T>(rhs)                        \
        );                                                                   \
    }                                                                        \
                                                                             \
    constexpr EnumType operator^(EnumType lhs, EnumType rhs) noexcept {      \
        using T = std::underlying_type_t<EnumType>;                          \
        return static_cast<EnumType>(                                        \
            static_cast<T>(lhs) ^ static_cast<T>(rhs)                        \
        );                                                                   \
    }                                                                        \
                                                                             \
    constexpr EnumType operator~(EnumType value) noexcept {                  \
        using T = std::underlying_type_t<EnumType>;                          \
        return static_cast<EnumType>(~static_cast<T>(value));                \
    }                                                                        \
                                                                             \
    constexpr EnumType& operator|=(EnumType& lhs, EnumType rhs) noexcept {   \
        lhs = lhs | rhs;                                                     \
        return lhs;                                                          \
    }                                                                        \
                                                                             \
    constexpr EnumType& operator&=(EnumType& lhs, EnumType rhs) noexcept {   \
        lhs = lhs & rhs;                                                     \
        return lhs;                                                          \
    }                                                                        \
                                                                             \
    constexpr EnumType& operator^=(EnumType& lhs, EnumType rhs) noexcept {   \
        lhs = lhs ^ rhs;                                                     \
        return lhs;                                                          \
    }
}
