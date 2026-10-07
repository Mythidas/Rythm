#pragma once

#include "graphics/resource_layout.h"

#include <vulkan/vulkan_core.h>

namespace rm::vk {
    class VKRenderDevice;

    class VKResourceLayout : public rm::ResourceLayout {
    public:
        VKResourceLayout(VKRenderDevice& device, const ResourceLayoutSpec& spec);
        ~VKResourceLayout();

        VkDescriptorSetLayout GetHandle() const { return layout; }

    private:
        VKRenderDevice& device;
        ResourceLayoutSpec spec;

        VkDescriptorSetLayout layout;
        VkDescriptorPool descriptorPool;
    };
}
