#include "rmpch.h"
#include "application.h"

#include <SDL3/SDL.h>

namespace rm {
    Application::Application(const ApplicationSpec& spec): spec(spec) {
        Logger::Init();
        Logger::Info("Initialized Logger");

        window = Window::Create({ .title = "Rythm", .width = 1280, .height = 720 });

        window->S_WindowClose.Register([this](){
            running = false;
            return false;
        });

        RendererSpec rSpec{
            .backend = RenderBackend::VULKAN,
            .window = *window
        };
        renderer = CreateScope<Renderer>(rSpec);
    }

    Application::~Application() {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
	    SDL_Quit();
    }

    void Application::Run() {
        running = true;
        Logger::Info("Running Application");

        while (running) {
            window->Update();
        }
    }
}
