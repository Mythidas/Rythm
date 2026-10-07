#pragma once

namespace rm {
    struct ResourceLayoutSpec {
        unsigned int count;
    };

    class ResourceLayout {
    public:
        virtual ~ResourceLayout() = default;
    };
}
