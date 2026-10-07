#pragma once

#include "render_types.h"

namespace rm {
    struct ImageSpec {
        ImageType type;
        RenderFormat format;
        ImageRect rect;
        ImageSamples samples;
        ImageTiling tiling;
        ImageUsage usage;
        ImageAspectMask aspectMask;

        unsigned int mipLevels{ 1 }, imgLayers{ 1 };
    };

    class Image {
    public:
        virtual ~Image() = default;
    };
}
