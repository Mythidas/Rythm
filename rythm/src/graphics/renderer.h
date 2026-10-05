#pragma once

#include "render_types.h"
#include "render_context.h"
#include "swapchain.h"
#include "core/defines.h"

namespace rm {
    struct RendererSpec {
        RenderBackend backend;
        Window& window;
    };

    class Renderer {
    public:
        explicit Renderer(const RendererSpec& spec);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        static Scope<Renderer> Create(const RendererSpec& spec);

    private:
        RendererSpec spec;

        Scope<RenderContext> context;
        Scope<RenderDevice> device;
        Scope<Swapchain> swapchain;
    };
}
