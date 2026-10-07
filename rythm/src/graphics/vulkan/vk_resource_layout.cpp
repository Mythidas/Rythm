#include "rmpch.h"
#include "vk_resource_layout.h"
#include "vk_render_device.h"
#include "vk_helpers.h"

#include <volk.h>

namespace rm::vk {
    VKResourceLayout::VKResourceLayout(VKRenderDevice& device, const ResourceLayoutSpec& spec): device(device), spec(spec) {
        VkDescriptorBindingFlags descVariableFlag{ VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };
        VkDescriptorSetLayoutBindingFlagsCreateInfo descBindingFlags{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = 1,
            .pBindingFlags = &descVariableFlag
        };
        VkDescriptorSetLayoutBinding descLayoutBinding{
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = spec.count,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        };
        VkDescriptorSetLayoutCreateInfo descLayoutCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &descBindingFlags,
            .bindingCount = 1,
            .pBindings = &descLayoutBinding
        };

        VK_CHECK(vkCreateDescriptorSetLayout(device.GetDevice(), &descLayoutCI, nullptr, &layout));

        VkDescriptorPoolSize poolSize{
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = spec.count
        };
        VkDescriptorPoolCreateInfo descPoolCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize
        };

        VK_CHECK(vkCreateDescriptorPool(device.GetDevice(), &descPoolCI, nullptr, &descriptorPool));
    }

    VKResourceLayout::~VKResourceLayout() {
        vkDestroyDescriptorSetLayout(device.GetDevice(), layout, nullptr);
        vkDestroyDescriptorPool(device.GetDevice(), descriptorPool, nullptr);
    }
}
