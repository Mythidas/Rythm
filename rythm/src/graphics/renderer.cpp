#include "rmpch.h"
#include "renderer.h"

namespace rm {
    Renderer::Renderer(const RendererSpec& spec): spec(spec) {
        context = RenderContext::Create({ .backend = spec.backend });
        device = context->CreateDevice({ .window = spec.window });
        swapchain = device->CreateSwapchain();

        Logger::Info("Created Renderer");
    }

    Renderer::~Renderer() {
        Logger::Info("Destroyed Renderer");
    }

    Scope<Renderer> Renderer::Create(const RendererSpec &spec) {
        return CreateScope<Renderer>(spec);
    }
}
