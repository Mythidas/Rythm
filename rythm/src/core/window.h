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

        static Scope<Window> Create(const WindowSpec& spec);

        void Update() const;

        SDL_Window& GetNative() const { return *native; }
        unsigned int GetWidth() const { return spec.width; }
        unsigned int GetHeight() const { return spec.height; }

        Signal<> S_WindowClose;
        Signal<unsigned int, unsigned int> S_WindowResize;

    private:
        WindowSpec spec;
        SDL_Window* native{ nullptr };
    };
}
