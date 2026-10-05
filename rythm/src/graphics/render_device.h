#pragma once

#include "swapchain.h"

#include "core/window.h"
#include "core/defines.h"

namespace rm {
    struct RenderDeviceSpec {
        unsigned int deviceIndex{ 0 };
        Window& window;
    };

    class RenderDevice {
    public:
        virtual ~RenderDevice() = default;

        virtual Scope<Swapchain> CreateSwapchain() = 0;
    };
}
