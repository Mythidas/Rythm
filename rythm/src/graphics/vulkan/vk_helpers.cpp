#include "rmpch.h"
#include "vk_helpers.h"

#include <string>
#include <cstdlib>

namespace rm {
    void VKHelpers::Check(VkResult result) {
        if (result != VK_SUCCESS) {
            std::string message = "Vulkan Call Failed: " + result; 
            Logger::Error(message);
            exit(result);
        }
    }

    void VKHelpers::Check(bool result) {
        if (!result) {
            Logger::Error("Vulkan Check failed");
            exit(result);
        }
    }
}
