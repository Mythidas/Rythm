#pragma once

#include "graphics/render_device.h"

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace rm::vk {
    class VKRenderContext;

    class VKRenderDevice : public RenderDevice {
    public:
        VKRenderDevice(VKRenderContext& context, const RenderDeviceSpec& spec);
        ~VKRenderDevice();

        Scope<Swapchain> CreateSwapchain() override;
        Scope<rm::Image> CreateImage(const ImageSpec& spec) override;
        Scope<RenderBuffer> CreateBuffer(const RenderBufferSpec& spec) override;
        Scope<RenderSync> CreateSync() override;
        Scope<CommandBuffer> CreateCommandBuffer(const CommandBufferSpec& spec) override;
        Scope<Shader> CreateShader(const ShaderSpec& spec) override;
        Scope<ResourceLayout> CreateResourceLayout(const ResourceLayoutSpec& spec) override;
        Scope<ResourceSet> CreateResourceSet() override;
        Scope<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineSpec& spec) override;

        VkDevice GetDevice() const { return device; }
        VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
        VmaAllocator GetAllocator() const { return allocator; }
        unsigned int GetQueueFamily() const { return queueFamily; }

    private:
        VKRenderContext& context;
        RenderDeviceSpec spec;

        VkDevice device{ VK_NULL_HANDLE };
        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
        VkPhysicalDeviceProperties2 physicalDeviceProperties{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
        };
        unsigned int queueFamily{ 0 };

        VkQueue queue{ VK_NULL_HANDLE };
        VmaAllocator allocator{ VK_NULL_HANDLE };
    };
}
