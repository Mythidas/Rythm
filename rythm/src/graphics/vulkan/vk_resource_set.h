#pragma once

#include "graphics/resource_set.h"

namespace rm::vk {
    class VKRenderDevice;

    class VKResourceSet : public rm::ResourceSet {
    public:
        VKResourceSet(VKRenderDevice& device);
        ~VKResourceSet();

    private:
        VKRenderDevice& device;
    };
}
