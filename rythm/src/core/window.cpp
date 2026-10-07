#include "rmpch.h"
#include "window.h"

#include <SDL3/SDL.h>

namespace rm {
    Window::Window(const WindowSpec& spec): title(spec.title), width(spec.width), height(spec.height) {
        native = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
        assert(native);

        SDL_ShowWindow(native);
        RM_LOG_INFO("Created Window: {} ({}x{})", title, width, height);
    }

    Window::~Window() {
	    SDL_DestroyWindow(native);
        RM_LOG_INFO("Destroyed Window: {}", title);
    }

    Scope<Window> Window::Create(const WindowSpec &spec) {
        return CreateScope<Window>(spec);
    }

    void Window::Update() {
        SDL_Delay(1);

        for (SDL_Event event; SDL_PollEvent(&event);) {
            if (event.type == SDL_EVENT_QUIT) {
                S_WindowClose.Dispatch();
                break;
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                S_WindowResize.Dispatch(event.window.data1, event.window.data2);
                width = event.window.data1;
                height = event.window.data2;
            }
        }
    }
}
