#pragma once

namespace rm {
    class Window;
}

namespace rm::gfx {
    class Renderer {
    public:
        Renderer(Window& window);
        ~Renderer();

    private:
        Window& window;

        void CreateInstance();
        void CreateDevice();
        void CreateSwapchain();
        void CreateBuffers();
    };
}
