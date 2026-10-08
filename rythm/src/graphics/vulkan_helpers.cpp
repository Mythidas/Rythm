#include "rmpch.h"
#include "vulkan_helpers.h"

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_enums.hpp>

namespace rm::gfx {
    void VulkanHelpers::Check(VkResult result, const char *expression, std::source_location location) {
        if (result == VK_SUCCESS) return;

        std::string message = std::format("Vulkan call failed!: \n\t{} \n\t{} \n\t{} \n\t{}", expression, string_VkResult(result), location.line(), location.function_name());
        throw std::runtime_error(message);
    }

    void VulkanHelpers::Check(vk::Result result, const char *expression, std::source_location location) {
        Check(static_cast<VkResult>(result), expression, location);
    }
}
