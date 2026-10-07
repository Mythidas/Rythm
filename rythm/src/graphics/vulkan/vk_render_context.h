#pragma once

#include "graphics/render_context.h"

#include <vulkan/vulkan_core.h>

namespace rm::vk {
    class VKRenderContext : public RenderContext {
    public:
        VKRenderContext();
        ~VKRenderContext();

        Scope<RenderDevice> CreateDevice(const RenderDeviceSpec& spec) override;

        VkInstance GetInstance() const { return instance; }

    private:
        VkInstance instance{ VK_NULL_HANDLE };
    };
}
