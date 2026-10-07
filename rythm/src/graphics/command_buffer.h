#pragma once

#include "graphics/color.h"

namespace rm {
    class Swapchain;
    class GraphicsPipeline;

    struct CommandBufferSpec {
        unsigned int buffers{ 1 };
    };

    class CommandBuffer {
    public:
        virtual ~CommandBuffer() = default;

        virtual void Reset(unsigned int index) = 0;
        virtual void Begin() const = 0;
        virtual void End() const = 0;

        virtual void BeginRendering(const Swapchain& swapchain, Color clear) const = 0;
        virtual void EndRendering(const Swapchain& swapchain) const = 0;
        virtual void Draw(unsigned int count) const = 0;

        virtual void BindPipeline(const GraphicsPipeline& pipeline) const = 0;
        virtual void BindBuffers() const = 0;
    };
}
