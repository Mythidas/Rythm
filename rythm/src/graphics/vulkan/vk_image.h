#pragma once

#include "graphics/image.h"

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

namespace rm::vk {
    class VKRenderDevice;

    class VKImage : public rm::Image {
    public:
        VKImage(VKRenderDevice& device, const ImageSpec& spec);
        ~VKImage();

        VkImage GetImage() const { return image; }
        VkImageView GetView() const { return view; }

    private:
        VKRenderDevice& device;
        ImageSpec spec;

        VkImage image{ VK_NULL_HANDLE };
        VkImageView view{ VK_NULL_HANDLE };
        VmaAllocation allocation{ VK_NULL_HANDLE };
    };
}
