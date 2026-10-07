#pragma once

#include "graphics/render_types.h"

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include <source_location>

namespace rm::vk {
    class VKHelpers {
    public:
        static void Check(VkResult result, const char* expression, std::source_location location = std::source_location::current());
        static void Check(bool result, const char* expression, std::source_location location = std::source_location::current());

        static VkFormat ToVkFormat(RenderFormat format);
        static VkImageType ToVkImageType(ImageType type);
        static VkImageViewType ToVkImageViewType(ImageType type);
        static VkSampleCountFlagBits ToVkSampleCount(ImageSamples sample);
        static VkImageUsageFlags ToVkImageUsage(ImageUsage usage);
        static VkImageTiling ToVkImageTiling(ImageTiling tiling);
        static VkImageAspectFlags ToVkImageAspect(ImageAspectMask aspectMask);

        static VkBufferUsageFlags ToVkBufferUsage(BufferUsage usage);
        static VmaAllocationCreateFlags ToVmaAllocation(BufferAllocation allocation);

        static unsigned int GetFormatSizeBytes(RenderFormat format);
    };

    #define VK_CHECK(expression) \
    ::rm::vk::VKHelpers::Check(  \
        (expression),        \
        #expression,         \
        std::source_location::current())
}
