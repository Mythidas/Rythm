#pragma once

#include "graphics/render_sync.h"

#include <vulkan/vulkan_core.h>

namespace rm::vk {
    class VKRenderDevice;

    class VKRenderSync : public rm::RenderSync {
    public:
        VKRenderSync(VKRenderDevice& device);
        ~VKRenderSync();

        void Reset() const override;
        void Wait() const override;

        VkFence GetFence() const { return fence; }
        VkSemaphore GetSemaphore() const { return semaphore; }
    private:
        VKRenderDevice& device;

        VkFence fence;
        VkSemaphore semaphore;
    };
}
