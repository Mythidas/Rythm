#pragma once

#include "core/bits.h"

namespace rm {
    // Renderer

    enum class RenderBackend {
        VULKAN,
    };

    enum class RenderFormat {
        UNDEFINED,

        // Color
        R8_UNORM,
        RG8_UNORM,
        RGBA8_UNORM,
        RGBA8_SRGB,
        BGRA8_UNORM,
        BGRA8_SRGB,

        // HDR
        RGBA16_FLOAT,
        RGBA32_FLOAT,

        // Depth / Stencil
        D32_FLOAT,
        D24_UNORM_S8_UINT,
        D32_FLOAT_S8_UINT,

        // Common
        RGB32_FLOAT,
        RG32_FLOAT,
        R32_FLOAT
    };

    // Image

    enum class ImageType {
        E2D, E3D
    };

    struct ImageRect {
        unsigned int width, height, depth;
    };

    enum class ImageSamples {
        S1 =    1 << 0, 
        S2 =    1 << 1, 
        S4 =    1 << 2, 
        S8 =    1 << 3, 
        S16 =   1 << 4, 
        S32 =   1 << 5, 
        S64 =   1 << 6
    };
    RM_BIT_ENUM(ImageSamples)

    enum class ImageTiling {
        NEAREST, LINEAR
    };

    enum class ImageUsage : unsigned int {
        NONE            = 0,
        DEPTH_STENCIL   = 1 << 0,
        COLOR           = 1 << 1
    };
    RM_BIT_ENUM(ImageUsage);

    enum class ImageAspectMask : unsigned int {
        NONE        = 0,
        COLOR       = 1 << 0,
        DEPTH       = 1 << 1,
        STENCIL     = 1 << 2,
        METADATA    = 1 << 3
    };
    RM_BIT_ENUM(ImageAspectMask)

    // Buffer

    enum class BufferUsage {
        SHADER = 1 << 0
    };
    RM_BIT_ENUM(BufferUsage)

    enum class BufferAllocation {
        SEQUENTIAL_WRITE = 1 << 0,
        TRANSFER_INSTEAD = 1 << 1,
        MAPPED           = 1 << 2
    };
    RM_BIT_ENUM(BufferAllocation)
}
