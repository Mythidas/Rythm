#include "rmpch.h"
#include "vk_swapchain.h"
#include "vk_render_device.h"
#include "vk_helpers.h"

#include <SDL3/SDL_vulkan.h>
#include <volk.h>

namespace rm {
    VKSwapchain::VKSwapchain(const VKRenderDevice& device, VkSurfaceKHR surface, const SwapchainSpec& spec) {
        VkSurfaceCapabilitiesKHR surfaceCaps{};
        VKHelpers::Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhysicalDevice(), surface, &surfaceCaps));

        VkExtent2D swapChainExtent{ surfaceCaps.currentExtent };
        if (surfaceCaps.currentExtent.width == 0xFFFFFFFF) {
            swapChainExtent = {
                .width = spec.width,
                .height = spec.height
            };
        }

        // Swap Chain
        const VkFormat imageFormat{ VK_FORMAT_B8G8R8A8_SRGB };
        VkSwapchainCreateInfoKHR swapChainCI{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = surfaceCaps.minImageCount,
            .imageFormat = imageFormat,
            .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = { .width = swapChainExtent.width, .height = swapChainExtent.height },
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR
        };

        VKHelpers::Check(vkCreateSwapchainKHR(device.GetDevice(), &swapChainCI, nullptr, &swapchain));

        uint32_t imageCount{ 0 };
        VKHelpers::Check(vkGetSwapchainImagesKHR(device.GetDevice(), swapchain, &imageCount, nullptr));

        swapchainImages.resize(imageCount);
        swapchainImageViews.resize(imageCount);
        VKHelpers::Check(vkGetSwapchainImagesKHR(device.GetDevice(), swapchain, &imageCount, swapchainImages.data()));

        for (auto i = 0; i < imageCount; i++) {
            VkImageViewCreateInfo viewCI{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapchainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = imageFormat,
                .subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = 1,
                    .layerCount = 1
                }
            };
            VKHelpers::Check(vkCreateImageView(device.GetDevice(), &viewCI, nullptr, &swapchainImageViews[i]));
        }
    }
}
