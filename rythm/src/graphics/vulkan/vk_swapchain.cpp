#include "rmpch.h"
#include "vk_swapchain.h"
#include "vk_render_device.h"
#include "vk_render_context.h"
#include "vk_image.h"
#include "vk_render_sync.h"
#include "vk_helpers.h"
#include "graphics/render_types.h"

#include <SDL3/SDL_vulkan.h>
#include <volk.h>
#include <vulkan/vulkan.h>

namespace rm::vk {
    VKSwapchain::VKSwapchain(VKRenderDevice& device, VKRenderContext& context, VkSurfaceKHR surface, const SwapchainSpec& spec): device(device), context(context) {
        assert(surface != VK_NULL_HANDLE);

        VkSurfaceCapabilitiesKHR surfaceCaps{};
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhysicalDevice(), surface, &surfaceCaps));

        extent = { surfaceCaps.currentExtent };
        if (surfaceCaps.currentExtent.width == 0xFFFFFFFF) {
            extent = {
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
            .imageExtent = { .width = extent.width, .height = extent.height },
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR
        };

        VK_CHECK(vkCreateSwapchainKHR(device.GetDevice(), &swapChainCI, nullptr, &swapchain));

        uint32_t imageCount{ 0 };
        VK_CHECK(vkGetSwapchainImagesKHR(device.GetDevice(), swapchain, &imageCount, nullptr));

        swapchainImages.resize(imageCount);
        swapchainImageViews.resize(imageCount);

        VK_CHECK(vkGetSwapchainImagesKHR(device.GetDevice(), swapchain, &imageCount, swapchainImages.data()));

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
            VK_CHECK(vkCreateImageView(device.GetDevice(), &viewCI, nullptr, &swapchainImageViews[i]));
        }

        std::vector<VkFormat> depthFormatList{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
        depthFormat = RenderFormat::UNDEFINED;

        for (VkFormat& format : depthFormatList) {
            VkFormatProperties2 formatProperties{ .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
            vkGetPhysicalDeviceFormatProperties2(device.GetPhysicalDevice(), format, &formatProperties);
            if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                if (format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
                    depthFormat = RenderFormat::D32_FLOAT_S8_UINT;
                } else { depthFormat = RenderFormat::D24_UNORM_S8_UINT; }
                break;
            }
        }

        VK_CHECK(depthFormat != RenderFormat::UNDEFINED);

        ImageSpec imageSpec{
                    .type = ImageType::E2D,
                    .format = depthFormat,
                    .rect = { .width = extent.width, .height = extent.height, .depth = 1 },
                    .samples = ImageSamples::S1,
                    .tiling = ImageTiling::NEAREST,
                    .usage = ImageUsage::DEPTH_STENCIL,
                    .aspectMask = ImageAspectMask::DEPTH
        };
        depthImage = device.CreateImage(imageSpec);

        Logger::Info("Created VKSwapchain");
    }

    VKSwapchain::~VKSwapchain() {
        for (auto i = 0; i < swapchainImages.size(); i++) {
            vkDestroyImageView(device.GetDevice(), swapchainImageViews[i], nullptr);
            vkDestroyImage(device.GetDevice(), swapchainImages[i], nullptr);
        } 

        vkDestroySwapchainKHR(device.GetDevice(), swapchain, nullptr);
        vkDestroySurfaceKHR(context.GetInstance(), surface, nullptr);

        Logger::Info("Destroyed VKSwapchain");
    }

    void VKSwapchain::AquireNext(const RenderSync &sync) {
        // Need to check for VK_ERROR_OUT_OF_DATE_KHR and rebuilt swapchain

        VkSemaphore semaphore = static_cast<const VKRenderSync&>(sync).GetSemaphore();
        VK_CHECK(vkAcquireNextImageKHR(device.GetDevice(), swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageIndex));
    }
}
