#pragma once

#include "graphics/render_buffer.h"

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

namespace rm::vk {
    class VKRenderDevice;

    class VKRenderBuffer : public RenderBuffer {
    public:
        VKRenderBuffer(VKRenderDevice& device, const RenderBufferSpec& spec);
        ~VKRenderBuffer();

        virtual void SetData(void* data, unsigned long long size) = 0;

    private:
        VKRenderDevice& device;

        VkBuffer buffer{ VK_NULL_HANDLE };
        VkDeviceAddress address{};
        VmaAllocation allocation{ VK_NULL_HANDLE };
        VmaAllocationInfo allocationInfo{};
    };
}
