#include "rmpch.h"
#include "renderer.h"
#include "render_buffer.h"
#include "render_device.h"
#include "render_context.h"
#include "render_sync.h"
#include "command_buffer.h"
#include "shader.h"
#include "shader_compiler.h"
#include "resource_layout.h"
#include "resource_set.h"
#include "swapchain.h"
#include "graphics_pipeline.h"

#include <glm/glm.hpp>

namespace rm {
    struct ShaderData {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 model[3];
        glm::vec4 lightPos{ 0.0f, -10.0f, 10.0f, 0.0f };
        uint32_t selected{ 1 };
    };

    Renderer::Renderer(const RendererSpec& spec): spec(spec) {
        shaderCompiler = CreateScope<ShaderCompiler>();
        context = RenderContext::Create({ .backend = spec.backend });
        device = context->CreateDevice({ .window = spec.window, .shaderCompiler = *shaderCompiler });
        swapchain = device->CreateSwapchain();

        shaderBuffers.resize(spec.framesInFlight);
        frameSyncs.resize(spec.framesInFlight);

        RenderBufferSpec rbSpec{
            .size = sizeof(ShaderData),
            .usage = BufferUsage::SHADER,
            .allocation = BufferAllocation::TRANSFER_INSTEAD | BufferAllocation::SEQUENTIAL_WRITE | BufferAllocation::MAPPED
        };

        for (auto i = 0; i < spec.framesInFlight; i++) {
            shaderBuffers[i] = device->CreateBuffer(rbSpec);
            frameSyncs[i] = device->CreateSync();
        }

        CommandBufferSpec cbSpec{
            .buffers = spec.framesInFlight
        };
        commandBuffer = device->CreateCommandBuffer(cbSpec);

        ShaderSpec shaderSpec{
            .path = std::string("assets/shader.slang")
        };
        shader = device->CreateShader(shaderSpec);

        ResourceLayoutSpec rlSpec{ .count = 1 };
        resourceLayout = device->CreateResourceLayout(rlSpec);

        std::vector<ShaderAttribute> shaderAttributes{
            { .location = 0, .binding = 0, .format = RenderFormat::RGB32_FLOAT },
            { .location = 1, .binding = 0, .format = RenderFormat::RGB32_FLOAT },
            { .location = 2, .binding = 0, .format = RenderFormat::RG32_FLOAT },
        };
        GraphicsPipelineSpec pipelineSpec{
            .layout = *resourceLayout,
            .shader = *shader,
            .depthFormat = swapchain->GetDepthFormat(),
            .shaderAttributes = shaderAttributes,
        };
        pipeline = device->CreateGraphicsPipeline(pipelineSpec);

        Logger::Info("Created Renderer");
    }

    Renderer::~Renderer() {
        Logger::Info("Destroyed Renderer");
    }
}
