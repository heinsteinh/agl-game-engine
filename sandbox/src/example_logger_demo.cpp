#include "agl.h"

class MyGame : public agl::Game {
public:
    void OnUpdate(float deltaTime) override {
        // Example of using application logger with improved deltaTime
        static float timer = 0.0f;
        timer += deltaTime;

        // Log every 5 seconds with timing information
        if (timer >= 5.0f) {
            AGL_INFO("Game running for {:.2f}s | Current FPS: {:.1f} | Delta: {:.3f}ms",
                    timer, GetFPS(), deltaTime * 1000.0f);
            timer = 0.0f;
        }

        // Example: Simple movement that's frame-rate independent
        static float position = 0.0f;
        float speed = 100.0f; // units per second
        position += speed * deltaTime;

        // Wrap position
        if (position > 400.0f) {
            position = -400.0f;
            AGL_INFO("Object wrapped around! New position: {:.1f}", position);
        }
    }

    void OnRender() override {
        // Your rendering code here
    }

    void OnImGuiRender() override {
        // Custom ImGui windows here
        ImGui::Begin("Logger Example");

        if (ImGui::Button("Log Info")) {
            AGL_INFO("User clicked the Info button!");
        }

        if (ImGui::Button("Log Warning")) {
            AGL_WARN("This is a warning message");
        }

        if (ImGui::Button("Log Error")) {
            AGL_ERROR("This is an error message");
        }

        ImGui::Separator();
        ImGui::Text("Timing Information:");
        ImGui::Text("FPS: %.1f", GetFPS());
        ImGui::Text("Delta Time: %.3f ms", GetDeltaTime() * 1000.0f);
        ImGui::Text("Average Delta: %.3f ms", GetAverageDeltaTime() * 1000.0f);

        ImGui::Separator();
        ImGui::Text("Check the console and agl-engine.log file for output");
        ImGui::End();
    }
};

int main() {
    MyGame game;

    if (game.Initialize(1024, 768, "Logger Demo - AGL Game Engine")) {
        AGL_INFO("Starting Logger Demo Application");
        game.Run();
        AGL_INFO("Application shutting down");
    } else {
        AGL_ERROR("Failed to initialize game!");
        return -1;
    }

    return 0;
}
