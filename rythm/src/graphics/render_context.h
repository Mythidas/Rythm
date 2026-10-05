#pragma once

#include "render_types.h"
#include "render_device.h"
#include "core/defines.h"

namespace rm {
    struct RenderContextSpec {
        RenderBackend backend;
    };

    class RenderContext {
    public:
        RenderContext() = default;
        virtual ~RenderContext() = default;

        virtual Scope<RenderDevice> CreateDevice(const RenderDeviceSpec& spec) = 0;

        static Scope<RenderContext> Create(const RenderContextSpec& spec);
    };
}
