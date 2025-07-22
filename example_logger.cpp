#include "agl.h"

class MyGame : public agl::Game {
public:
    void OnUpdate(float deltaTime) override {
        // Example of using application logger
        static float timer = 0.0f;
        timer += deltaTime;
        
        // Log every 5 seconds
        if (timer >= 5.0f) {
            AGL_INFO("Game has been running for {:.2f} seconds", timer);
            timer = 0.0f;
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
