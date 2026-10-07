#include "rmpch.h"
#include "vk_shader.h"
#include "vk_render_device.h"
#include "vk_helpers.h"

#include "graphics/shader_compiler.h"

#include <volk.h>

namespace rm::vk {
    VKShader::VKShader(VKRenderDevice& device, const ShaderBytecode& bytecode, const ShaderSpec& spec): device(device), spec(spec) {
        VkShaderModuleCreateInfo shaderModuleCI{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = bytecode.Size(),
            .pCode = bytecode.Data()
        };

        VK_CHECK(vkCreateShaderModule(device.GetDevice(), &shaderModuleCI, nullptr, &shader));

        RM_LOG_INFO("Created Shader: {}", spec.path);
    }

    VKShader::~VKShader() {
        vkDestroyShaderModule(device.GetDevice(), shader, nullptr);
    }
}
