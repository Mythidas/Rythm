#pragma once

#include "graphics/shader.h"

#include <vulkan/vulkan_core.h>

namespace rm {
    struct ShaderBytecode;
}

namespace rm::vk {
    class VKRenderDevice;

    class VKShader : public rm::Shader {
    public:
        VKShader(VKRenderDevice& device, const ShaderBytecode& bytecode, const ShaderSpec& spec);
        ~VKShader();

        VkShaderModule GetHandle() const { return shader; }

    private:
        VKRenderDevice& device;
        ShaderSpec spec;

        VkShaderModule shader;
    };
}
