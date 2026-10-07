#pragma once

#include "core/window.h"
#include "core/defines.h"

namespace rm {
    class RenderBuffer;
    class RenderSync;
    class Swapchain;
    class Image;
    class CommandBuffer;
    class ResourceSet;
    class ResourceLayout;
    class Shader;
    class ShaderCompiler;
    class GraphicsPipeline;

    struct ImageSpec;
    struct RenderBufferSpec;
    struct CommandBufferSpec;
    struct ResourceLayoutSpec;
    struct ShaderSpec;
    struct GraphicsPipelineSpec;

    struct RenderDeviceSpec {
        unsigned int deviceIndex{ 0 };
        Window& window;
        ShaderCompiler& shaderCompiler;
    };

    class RenderDevice {
    public:
        virtual ~RenderDevice() = default;

        virtual Scope<Swapchain> CreateSwapchain() = 0;
        virtual Scope<Image> CreateImage(const ImageSpec& spec) = 0;
        virtual Scope<RenderBuffer> CreateBuffer(const RenderBufferSpec& spec) = 0;
        virtual Scope<RenderSync> CreateSync() = 0;
        virtual Scope<CommandBuffer> CreateCommandBuffer(const CommandBufferSpec& spec) = 0;
        virtual Scope<Shader> CreateShader(const ShaderSpec& spec) = 0;
        virtual Scope<ResourceLayout> CreateResourceLayout(const ResourceLayoutSpec& spec) = 0;
        virtual Scope<ResourceSet> CreateResourceSet() = 0;
        virtual Scope<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineSpec& spec) = 0;
    };
}
