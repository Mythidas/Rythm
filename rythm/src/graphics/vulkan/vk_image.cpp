#include "rmpch.h"
#include "vk_image.h"
#include "vk_render_device.h"
#include "vk_helpers.h"
#include "graphics/render_types.h"
#include "vulkan/vulkan_core.h"

#include <volk.h>


namespace rm::vk {
    VKImage::VKImage(VKRenderDevice& device, const ImageSpec& spec): device(device), spec(spec) {
        RM_ASSERT(spec.rect.depth > 0, "Image depth must be > 0, got {}", spec.rect.depth);

        VkImageCreateInfo imageCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VKHelpers::ToVkImageType(spec.type),
            .format = VKHelpers::ToVkFormat(spec.format),
            .extent{
                .width = spec.rect.width,
                .height = spec.rect.height,
                .depth = spec.rect.depth
            },
            .mipLevels = spec.mipLevels,
            .arrayLayers = spec.imgLayers,
            .samples = VKHelpers::ToVkSampleCount(spec.samples),
            .tiling = VKHelpers::ToVkImageTiling(spec.tiling),
            .usage = VKHelpers::ToVkImageUsage(spec.usage),
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VmaAllocationCreateInfo allocCI{ 
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, 
            .usage = VMA_MEMORY_USAGE_AUTO 
        };

        VK_CHECK(vmaCreateImage(device.GetAllocator(), &imageCI, &allocCI, &image, &allocation, nullptr));

        VkImageViewCreateInfo imageViewCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VKHelpers::ToVkImageViewType(spec.type),
            .format = VKHelpers::ToVkFormat(spec.format),
            .subresourceRange{
                .aspectMask = VKHelpers::ToVkImageAspect(spec.aspectMask),
                .levelCount = spec.mipLevels,
                .layerCount = spec.imgLayers
            }
        };

        VK_CHECK(vkCreateImageView(device.GetDevice(), &imageViewCI, nullptr, &view));

        Logger::Info("Created VKImage");
    }

    VKImage::~VKImage() {
        vkDestroyImageView(device.GetDevice(), view, nullptr);
        vmaDestroyImage(device.GetAllocator(), image, allocation);
    }
}
