#pragma once

#include "graphics/graphics_pipeline.h"

#include <vulkan/vulkan_core.h>

namespace rm::vk {
    class VKRenderDevice;

    class VKGraphicsPipeline : public rm::GraphicsPipeline {
    public:
        VKGraphicsPipeline(VKRenderDevice& device, const GraphicsPipelineSpec& spec);
        ~VKGraphicsPipeline();

        VkPipeline GetHandle() const { return pipeline; }
        VkPipelineLayout GetLayout() const { return pipelineLayout; }

    private:
        VKRenderDevice& device;
        GraphicsPipelineSpec spec;

        VkPipeline pipeline{ VK_NULL_HANDLE };
        VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
    };
}
