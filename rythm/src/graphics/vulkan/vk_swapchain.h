#pragma once

#include "graphics/swapchain.h"
#include "core/defines.h"

#include <vulkan/vulkan_core.h>
#include <vector>

namespace rm::vk {
    class VKRenderDevice;
    class VKRenderContext;

    class VKSwapchain : public Swapchain {
    public:
        VKSwapchain(VKRenderDevice& device, VKRenderContext& context, VkSurfaceKHR surface, const SwapchainSpec& spec);
        ~VKSwapchain();

        void AquireNext(const RenderSync& sync) override;

        RenderFormat GetDepthFormat() const override { return depthFormat; }
        Image& GetDepthImage() const override { return *depthImage; }

        VkImage GetCurrentImage() const { return swapchainImages.at(imageIndex); }
        VkImageView GetCurrentImageView() const { return swapchainImageViews.at(imageIndex); }
        VkExtent2D GetExtent() const { return extent; }

    private:
        VKRenderContext& context;
        VKRenderDevice& device;

        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        Scope<rm::Image> depthImage;
        RenderFormat depthFormat;
        VkExtent2D extent;

        unsigned int imageIndex{ 0 };
        std::vector<VkImage> swapchainImages{};
        std::vector<VkImageView> swapchainImageViews{};
    };
}
