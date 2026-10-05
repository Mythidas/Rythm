#include "rmpch.h"
#include "application.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace rm {
    Application::Application(const ApplicationSpec& spec): spec(spec) {
        Logger::Init();
        Logger::Info("Initialized Logger");

        bool sdlInit = SDL_Init(SDL_INIT_VIDEO);
        assert(sdlInit);
        Logger::Info("Initialized SDL");

        bool sdlVulkan = SDL_Vulkan_LoadLibrary(NULL);
        assert(sdlVulkan);
        Logger::Info("Initialized SDL Vulkan");

        window = Window::Create({ .title = "Rythm", .width = 1280, .height = 720 });

        window->S_WindowClose.Register([this](){
            running = false;
            return false;
        });

        renderer = Renderer::Create({ .backend = VULKAN, .window = *window });
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
