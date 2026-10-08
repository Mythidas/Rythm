#pragma once
#include "signal.h"
#include "defines.h"

struct SDL_Window;

namespace rm {
    struct WindowSpec {
        const char* title;
        unsigned int width, height;
    };

    class Window {
    public:
        explicit Window(const WindowSpec& spec);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        void Update();

        SDL_Window* GetNative() const { return native; }
        unsigned int GetWidth() const { return width; }
        unsigned int GetHeight() const { return height; }
        const char* GetTitle() const { return title; }

        Signal<> S_WindowClose;
        Signal<unsigned int, unsigned int> S_WindowResize;

    private:
        SDL_Window* native{ nullptr };

        const char* title;
        unsigned int width, height;
    };
}
