#include "game.h"
#include <iostream>
#include <chrono>
#include <GLFW/glfw3.h>

#if defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace agl {

Game::Game()
    : m_running(false), m_lastFrameTime(0.0f) {
}

Game::~Game() {
    Shutdown();
}

bool Game::Initialize(int width, int height, const char* title) {
    // Initialize GLFW
    Window::Initialize();

    // Create window
    m_window = std::make_unique<Window>();
    if (!m_window->Create(width, height, title)) {
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }

    // Initialize input
    m_input = std::make_unique<Input>();
    m_input->Initialize(m_window->GetNativeWindow());

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    const char* glsl_version = "#version 330";
    ImGui_ImplGlfw_InitForOpenGL(m_window->GetNativeWindow(), true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Set up basic OpenGL state
    glEnable(GL_DEPTH_TEST);

    // Set window callbacks
    m_window->SetResizeCallback([this](int width, int height) {
        // Handle window resize
        glViewport(0, 0, width, height);
    });

    m_window->SetCloseCallback([this]() {
        m_running = false;
    });

    // Set input callbacks
    m_input->SetKeyCallback([this](int key, KeyState state) {
        if (key == GLFW_KEY_ESCAPE && state == KeyState::Pressed) {
            m_running = false;
        }
    });

    std::cout << "Game initialized successfully!" << std::endl;
    return true;
}

void Game::Run() {
    if (!m_window || !m_input) {
        std::cerr << "Game not properly initialized!" << std::endl;
        return;
    }

    m_running = true;
    m_lastFrameTime = static_cast<float>(glfwGetTime());

    std::cout << "Starting game loop..." << std::endl;

    while (m_running && !m_window->ShouldClose()) {
        // Calculate delta time
        CalculateDeltaTime();

        // Poll events and update input
        m_window->PollEvents();
        m_input->Update();

        // Clear the screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update game logic
        OnUpdate(m_lastFrameTime);

        // Render game
        OnRender();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render ImGui
        OnImGuiRender();

        // Default ImGui demo window (remove in actual games)
        {
            ImGui::Begin("AGT Game Engine");
            ImGui::Text("Game Engine is running!");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                       1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            // Display input info
            double mouseX, mouseY;
            m_input->GetMousePosition(mouseX, mouseY);
            ImGui::Text("Mouse Position: (%.1f, %.1f)", mouseX, mouseY);

            double deltaX, deltaY;
            m_input->GetMouseDelta(deltaX, deltaY);
            ImGui::Text("Mouse Delta: (%.1f, %.1f)", deltaX, deltaY);

            // Key states
            ImGui::Text("Key States:");
            if (m_input->IsKeyHeld(GLFW_KEY_W)) ImGui::Text("W is held");
            if (m_input->IsKeyHeld(GLFW_KEY_A)) ImGui::Text("A is held");
            if (m_input->IsKeyHeld(GLFW_KEY_S)) ImGui::Text("S is held");
            if (m_input->IsKeyHeld(GLFW_KEY_D)) ImGui::Text("D is held");

            if (m_input->IsMouseButtonHeld(MouseButton::Left)) ImGui::Text("Left mouse button held");
            if (m_input->IsMouseButtonHeld(MouseButton::Right)) ImGui::Text("Right mouse button held");

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        m_window->SwapBuffers();
    }

    std::cout << "Game loop ended." << std::endl;
}

void Game::Shutdown() {
    if (m_window) {
        // Cleanup ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // Cleanup game resources
        m_input.reset();
        m_window.reset();

        // Terminate GLFW
        Window::Terminate();

        std::cout << "Game shutdown complete." << std::endl;
    }
}

void Game::CalculateDeltaTime() {
    float currentTime = static_cast<float>(glfwGetTime());
    m_lastFrameTime = currentTime - m_lastFrameTime;
    m_lastFrameTime = currentTime;
}

}
