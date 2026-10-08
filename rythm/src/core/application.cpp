#include "rmpch.h"
#include "application.h"
#include "window.h"
#include "graphics/renderer.h"

#include <SDL3/SDL.h>
#include <stdexcept>

namespace rm {
    Application::Application(const ApplicationSpec& spec): spec(spec) {
        Logger::Init();
        Logger::Info("Initialized Logger");

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(SDL_GetError());
        }

        window = Window::Create({ .title = "Rythm", .width = 1280, .height = 720 });

        window->S_WindowClose.Register([this](){
            running = false;
            return false;
        });

        renderer = CreateScope<gfx::Renderer>(*window);
    }

    Application::~Application() {
        window.reset();
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
	    SDL_Quit();
    }

    void Application::Run() {
        running = true;
        Logger::Info("Running Application");

        while (running) {
            window->Update();
            if (running) {
            }
        }
    }
}
