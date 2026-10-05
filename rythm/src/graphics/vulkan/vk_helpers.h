#pragma once

#include <vulkan/vulkan.h>

namespace rm {
    class VKHelpers {
    public:
        static void Check(VkResult result);
        static void Check(bool result);
    };
}
