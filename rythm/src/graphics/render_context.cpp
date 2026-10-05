#include "rmpch.h"
#include "render_context.h"
#include "vulkan/vk_render_context.h"

namespace rm {
    Scope<RenderContext> RenderContext::Create(const RenderContextSpec &spec) {
        if (spec.backend == VULKAN) {
            return CreateScope<VKRenderContext>();
        }

        return nullptr;
    }
}
