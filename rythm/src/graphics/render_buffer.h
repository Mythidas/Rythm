#pragma once

#include "render_types.h"

namespace rm {
    struct RenderBufferSpec {
        unsigned int size;
        BufferUsage usage;
        BufferAllocation allocation;
    };

    class RenderBuffer {
    public:
        virtual ~RenderBuffer() = default;

        virtual void SetData(void* data, unsigned long long size) = 0;
    };
}
