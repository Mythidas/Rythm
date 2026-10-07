#pragma once

#include "graphics/command_buffer.h"

#include <vulkan/vulkan_core.h>
#include <vector>

namespace rm::vk {
    class VKRenderDevice;

    class VKCommandBuffer : public rm::CommandBuffer {
    public:
        VKCommandBuffer(VKRenderDevice& device, const CommandBufferSpec& spec);
        ~VKCommandBuffer();

        void Reset(unsigned int index) override;
        void Begin() const override;
        void End() const override;

        virtual void BeginRendering(const Swapchain& swapchain, Color clear) const override;
        virtual void EndRendering(const Swapchain& swapchain) const override;
        virtual void Draw(unsigned int count) const override;

        virtual void BindPipeline(const GraphicsPipeline& pipeline) const override;
        virtual void BindBuffers() const override;

    private:
        VKRenderDevice& device;

        VkCommandPool pool;
        std::vector<VkCommandBuffer> buffers;
        unsigned int currentIndex;
    };
}
