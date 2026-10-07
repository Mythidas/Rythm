#pragma once

#include <vector>

namespace rm {
    class ResourceLayout;
    class Shader;
    enum class RenderFormat;

    struct ShaderAttribute {
        unsigned int location{ 0 }, binding{ 0 };
        RenderFormat format;
    };

    struct GraphicsPipelineSpec {
        ResourceLayout& layout;
        Shader& shader;

        RenderFormat depthFormat;

        const std::vector<ShaderAttribute>& shaderAttributes;
    };

    class GraphicsPipeline {
    public:
        virtual ~GraphicsPipeline() = default;
    };
}
