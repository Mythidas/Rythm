#pragma once

#include "vk_swapchain.h"
#include "graphics/render_device.h"

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace rm {
    class VKRenderContext;

    class VKRenderDevice : public RenderDevice {
    public:
        VKRenderDevice(VKRenderContext& context, const RenderDeviceSpec& spec);
        ~VKRenderDevice();

        Scope<Swapchain> CreateSwapchain() override;

        VkDevice GetDevice() const { return device; }
        VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
    private:
        VKRenderContext& context;
        RenderDeviceSpec spec;

        VkDevice device{ VK_NULL_HANDLE };
        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
        VkPhysicalDeviceProperties2 physicalDeviceProperties{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
        };
        VkQueue queue{ VK_NULL_HANDLE };
        VmaAllocator allocator{ VK_NULL_HANDLE };
    };
}
