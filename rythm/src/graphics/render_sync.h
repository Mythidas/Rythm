#pragma once

namespace rm {
    class RenderSync {
    public:
        virtual ~RenderSync() = default;

        virtual void Reset() const = 0;
        virtual void Wait() const = 0;
    };
}
