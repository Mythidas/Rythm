#include "rmpch.h"
#include "window.h"

#include <SDL3/SDL.h>

namespace rm {
    Window::Window(const WindowSpec& spec): spec(spec) {
        native = SDL_CreateWindow(spec.title, spec.width, spec.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
        assert(native);

        SDL_ShowWindow(native);
        RM_LOG_INFO("Created Window: {} ({}x{})", spec.title, spec.width, spec.height);
    }

    Window::~Window() {
	    SDL_DestroyWindow(native);
        RM_LOG_INFO("Destroyed Window: {}", spec.title);
    }

    Scope<Window> Window::Create(const WindowSpec &spec) {
        return CreateScope<Window>(spec);
    }

    void Window::Update() const {
        SDL_Delay(1);

        for (SDL_Event event; SDL_PollEvent(&event);) {
            if (event.type == SDL_EVENT_QUIT) {
                S_WindowClose.Dispatch();
                break;
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                S_WindowResize.Dispatch(event.window.data1, event.window.data2);
            }
        }
    }
}
