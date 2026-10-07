#pragma once


namespace rm {
    class Image;
    class RenderSync;
    enum class RenderFormat;

    struct SwapchainSpec {
        unsigned int width, height;
    };

    class Swapchain {
    public:
        virtual ~Swapchain() = default;

        virtual void AquireNext(const RenderSync& sync) = 0;

        virtual RenderFormat GetDepthFormat() const = 0;
        virtual Image& GetDepthImage() const = 0;
    };
}
