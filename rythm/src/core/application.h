#pragma once

#include "defines.h"

namespace rm {
    namespace gfx {
        class Renderer;
    }
    class Window;

    struct ApplicationSpec {

    };

    class Application {
    public:
        explicit Application(const ApplicationSpec& spec);
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void Run();

        Window& GetWindow() const { return *window; }
    private:
        ApplicationSpec spec;
        Scope<Window> window;
        Scope<gfx::Renderer> renderer;

        bool running;
    };
}
