#pragma once

namespace rm {
    struct SwapchainSpec {
        unsigned int width, height;
    };

    class Swapchain {
    public:
        virtual ~Swapchain() = default;
    };
}
