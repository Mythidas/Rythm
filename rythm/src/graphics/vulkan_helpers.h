#pragma once

#include <source_location>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.hpp>

namespace rm::gfx {
    class VulkanHelpers {
    public:
        static void Check(VkResult result, const char* expression, std::source_location location);
        static void Check(vk::Result result, const char* expression, std::source_location location);
    };

    #define RM_VK_CHECK(expression) ::rm::gfx::VulkanHelpers::Check((expression), #expression, std::source_location::current())
}
