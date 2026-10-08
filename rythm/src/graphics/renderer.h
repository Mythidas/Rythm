#pragma once

#include <glm/glm.hpp>

namespace rm {
    class Window;
    struct Color;
}

namespace rm::gfx {
    class Renderer {
    public:
        Renderer(Window& window);
        ~Renderer();

        void BeginFrame();
        void EndFrame();

        void DrawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, rm::Color color);

    private:
        Window& window;
        uint32_t currentFrame{ 0 };
        uint32_t currentImage{ 0 };

        void FlushFrame();

        void CreateInstance();
        void CreateDevice();
        void CreateSwapchain();
        void CreateBuffers();
        void CreateShaders();
        void CreatePipeline();
        void CreateCameraBuffer();
        void CreateSyncObjects();
    };
}
