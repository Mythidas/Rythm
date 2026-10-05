#pragma once

#include "defines.h"
#include "window.h"
#include "graphics/renderer.h"

namespace rm {
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
        Scope<Renderer> renderer;

        bool running;
    };
}
