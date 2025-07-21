#pragma once

#include "window.h"
#include "input.h"
#include <memory>

namespace agl {
    class Game {
    public:
        Game();
        ~Game();

        bool Initialize(int width = 800, int height = 600, const char* title = "AGT Game Engine");
        void Run();
        void Shutdown();

        // Getters
        Window* GetWindow() const { return m_window.get(); }
        Input* GetInput() const { return m_input.get(); }

        // Game loop callbacks
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnRender() {}
        virtual void OnImGuiRender() {}

    private:
        std::unique_ptr<Window> m_window;
        std::unique_ptr<Input> m_input;
        bool m_running;
        float m_lastFrameTime;

        void CalculateDeltaTime();
    };
}
