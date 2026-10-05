#pragma once

#include "graphics/swapchain.h"

#include <vulkan/vulkan_core.h>
#include <vector>

namespace rm {
    class VKRenderDevice;

    class VKSwapchain : public Swapchain {
    public:
        VKSwapchain(const VKRenderDevice& device, VkSurfaceKHR surface, const SwapchainSpec& spec);
        ~VKSwapchain();

    private:
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;

        std::vector<VkImage> swapchainImages{};
        std::vector<VkImageView> swapchainImageViews{};
    };
}
