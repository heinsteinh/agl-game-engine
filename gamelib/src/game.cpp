#include "game.h"
#include <chrono>
#include <algorithm>
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
    : m_running(false), 
      m_lastFrameTime(0.0f), 
      m_deltaTime(0.0f),
      m_fps(0.0f),
      m_averageDeltaTime(0.0f),
      m_frameTimeAccumulator(0.0f),
      m_frameCount(0) {
}

Game::~Game() {
    Shutdown();
}

bool Game::Initialize(int width, int height, const char* title) {
    // Initialize Logger first
    Logger::Initialize();
    
    AGL_CORE_INFO("Initializing AGL Game Engine...");
    
    // Initialize GLFW
    Window::Initialize();

    // Create window
    m_window = std::make_unique<Window>();
    if (!m_window->Create(width, height, title)) {
        AGL_CORE_ERROR("Failed to create window");
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

    AGL_CORE_INFO("Game initialized successfully!");
    return true;
}

void Game::Run() {
    if (!m_window || !m_input) {
        AGL_CORE_ERROR("Game not properly initialized!");
        return;
    }

    m_running = true;
    m_lastFrameTime = static_cast<float>(glfwGetTime());

    AGL_CORE_INFO("Starting game loop...");

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
        OnUpdate(m_deltaTime);

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
            
            // Enhanced timing information
            ImGui::Separator();
            ImGui::Text("Timing Information:");
            ImGui::Text("Current FPS: %.1f", m_fps);
            ImGui::Text("Current Delta Time: %.3f ms", m_deltaTime * 1000.0f);
            ImGui::Text("Average Delta Time: %.3f ms", m_averageDeltaTime * 1000.0f);
            ImGui::Text("ImGui FPS: %.1f", ImGui::GetIO().Framerate);
            
            // Frame time graph
            static float frameTimeHistory[100] = {0};
            static int frameTimeOffset = 0;
            frameTimeHistory[frameTimeOffset] = m_deltaTime * 1000.0f;
            frameTimeOffset = (frameTimeOffset + 1) % 100;
            
            ImGui::PlotLines("Frame Time (ms)", frameTimeHistory, 100, frameTimeOffset, 
                            nullptr, 0.0f, MAX_DELTA_TIME * 1000.0f, ImVec2(0, 80));

            ImGui::Separator();
            ImGui::Text("Input Information:");

            ImGui::Separator();
            ImGui::Text("Input Information:");

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

    AGL_CORE_INFO("Game loop ended.");
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

        AGL_CORE_INFO("Game shutdown complete.");
        
        // Shutdown logger last
        Logger::Shutdown();
    }
}

void Game::CalculateDeltaTime() {
    float currentTime = static_cast<float>(glfwGetTime());
    
    // Calculate raw delta time
    float rawDeltaTime = currentTime - m_lastFrameTime;
    m_lastFrameTime = currentTime;
    
    // Clamp delta time to prevent large spikes (spiral of death)
    m_deltaTime = std::min(rawDeltaTime, MAX_DELTA_TIME);
    
    // Accumulate frame time for FPS calculation
    m_frameTimeAccumulator += rawDeltaTime;
    m_frameCount++;
    
    // Update FPS and average delta time every second
    if (m_frameTimeAccumulator >= FPS_UPDATE_INTERVAL) {
        m_averageDeltaTime = m_frameTimeAccumulator / m_frameCount;
        m_fps = static_cast<float>(m_frameCount) / m_frameTimeAccumulator;
        
        // Reset accumulator
        m_frameTimeAccumulator = 0.0f;
        m_frameCount = 0;
        
        // Log FPS info occasionally (every 5 seconds)
        static float fpsLogTimer = 0.0f;
        fpsLogTimer += FPS_UPDATE_INTERVAL;
        if (fpsLogTimer >= 5.0f) {
            AGL_CORE_TRACE("FPS: {:.1f}, Avg Delta: {:.3f}ms, Current Delta: {:.3f}ms", 
                          m_fps, m_averageDeltaTime * 1000.0f, m_deltaTime * 1000.0f);
            fpsLogTimer = 0.0f;
        }
    }
}

}
