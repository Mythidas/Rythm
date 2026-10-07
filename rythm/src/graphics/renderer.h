#pragma once

#include "render_types.h"
#include "core/defines.h"

#include <vector>

namespace rm {
    class RenderContext;
    class RenderDevice;
    class RenderBuffer;
    class RenderSync;
    class CommandBuffer;
    class Shader;
    class ShaderCompiler;
    class ResourceLayout;
    class ResourceSet;
    class Swapchain;
    class GraphicsPipeline;
    class Window;

    struct RendererSpec {
        RenderBackend backend;
        Window& window;

        unsigned int framesInFlight{ 2 };
    };

    class Renderer {
    public:
        explicit Renderer(const RendererSpec& spec);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

    private:
        RendererSpec spec;

        Scope<RenderContext> context;
        Scope<RenderDevice> device;
        Scope<Swapchain> swapchain;
        Scope<CommandBuffer> commandBuffer;
        Scope<Shader> shader;
        Scope<ShaderCompiler> shaderCompiler;
        Scope<ResourceLayout> resourceLayout;
        Scope<ResourceSet> resourceSet;
        Scope<GraphicsPipeline> pipeline;

        std::vector<Scope<RenderBuffer>> shaderBuffers;
        std::vector<Scope<RenderSync>> frameSyncs;
    };
}
