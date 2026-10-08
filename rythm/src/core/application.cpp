#include "rmpch.h"
#include "application.h"
#include "window.h"
#include "graphics/renderer.h"
#include "graphics/color.h"


namespace rm {
    Application::Application(const ApplicationSpec& spec): spec(spec) {
        Logger::Init();
        Logger::Info("Initialized Logger");

        window = CreateScope<Window>(WindowSpec({ .title = "Rythm", .width = 1280, .height = 720 }));

        window->S_WindowClose.Register([this](){
            running = false;
            return false;
        });

        renderer = CreateScope<gfx::Renderer>(*window);
    }

    Application::~Application() {
        window.reset();
        renderer.reset();

        Logger::Info("Application Destroyed");
    }

    void Application::Run() {
        running = true;
        Logger::Info("Running Application");

        while (running) {
            window->Update();

            renderer->BeginFrame();
            renderer->DrawQuad({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
            renderer->EndFrame();
        }
    }
}
