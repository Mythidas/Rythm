#include "rmpch.h"
#include "vk_graphics_pipeline.h"
#include "vk_render_device.h"
#include "vk_resource_layout.h"
#include "vk_shader.h"
#include "vk_helpers.h"

#include <volk.h>

namespace rm::vk {
    VKGraphicsPipeline::VKGraphicsPipeline(VKRenderDevice& device, const GraphicsPipelineSpec& spec): device(device), spec(spec) {
        VkPushConstantRange pushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .size = sizeof(VkDeviceAddress)
        };

        const VkDescriptorSetLayout setLayout = static_cast<const VKResourceLayout&>(spec.layout).GetHandle();
        VkPipelineLayoutCreateInfo pipelineLayoutCI{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &setLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstantRange
        };

        VK_CHECK(vkCreatePipelineLayout(device.GetDevice(), &pipelineLayoutCI, nullptr, &pipelineLayout));

        const VkShaderModule shaderModule = static_cast<const VKShader&>(spec.shader).GetHandle();
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
            { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = shaderModule, .pName = "main" },
            { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = shaderModule, .pName = "main" },
        };


        std::vector<VkVertexInputAttributeDescription> shaderAttributes;
        shaderAttributes.resize(spec.shaderAttributes.size());

        unsigned int offset = 0;
        for (auto i = 0; i < shaderAttributes.size(); i++) {
            shaderAttributes[i] = {
                .location = spec.shaderAttributes[i].location,
                .binding = spec.shaderAttributes[i].binding,
                .format = VKHelpers::ToVkFormat(spec.shaderAttributes[i].format),
                .offset = offset
            };

            offset += VKHelpers::GetFormatSizeBytes(spec.shaderAttributes[i].format);
        }

        VkVertexInputBindingDescription vertexBinding{
            .binding = 0,
            .stride = offset,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        VkPipelineVertexInputStateCreateInfo vertexInputState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &vertexBinding,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(shaderAttributes.size()),
            .pVertexAttributeDescriptions = shaderAttributes.data()
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
        };

        std::vector<VkDynamicState> dynamicStates{
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = dynamicStates.data()
        };

        VkPipelineViewportStateCreateInfo viewportState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1
        };
        VkPipelineRasterizationStateCreateInfo rasterizationState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .lineWidth = 1.0f
        };
        VkPipelineMultisampleStateCreateInfo multisampleState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        };
        VkPipelineDepthStencilStateCreateInfo depthStencilState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL
        };
        VkPipelineColorBlendAttachmentState blendAttachment{ .colorWriteMask = 0xF };
        VkPipelineColorBlendStateCreateInfo colorBlendState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &blendAttachment
        };

        const VkFormat imageFormat{ VK_FORMAT_B8G8R8A8_SRGB };
        VkPipelineRenderingCreateInfo renderingCI{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &imageFormat,
            .depthAttachmentFormat = VKHelpers::ToVkFormat(spec.depthFormat)
        };

        VkGraphicsPipelineCreateInfo pipelineCI{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderingCI,
            .stageCount = 2,
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputState,
            .pInputAssemblyState = &inputAssemblyState,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizationState,
            .pMultisampleState = &multisampleState,
            .pDepthStencilState = &depthStencilState,
            .pColorBlendState = &colorBlendState,
            .pDynamicState = &dynamicState,
            .layout = pipelineLayout
        };

        VK_CHECK(vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline));
    }

    VKGraphicsPipeline::~VKGraphicsPipeline() {
        vkDestroyPipeline(device.GetDevice(), pipeline, nullptr);
        vkDestroyPipelineLayout(device.GetDevice(), pipelineLayout, nullptr);
    }
}
