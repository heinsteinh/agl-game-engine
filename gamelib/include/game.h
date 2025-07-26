#ifndef GAME_H
#define GAME_H

#include "DispatchQueue.h"
#include "Logger.h"
#include "input.h"
#include "window.h"
#include <memory>

namespace agl {
class Game {
public:
    Game();
    ~Game();

    bool Initialize(int width = 800, int height = 600, const char *title = "AGT Game Engine");
    void Run();
    void Shutdown();

    // Getters
    Window *GetWindow() const {
        return m_window.get();
    }
    Input *GetInput() const {
        return m_input.get();
    }
    DispatchQueue &GetMainQueue() const {
        return DispatchQueue::main();
    }

    // Queue operations for convenience
    template <typename F>
    void RunOnMainThread(F &&func) {
        DispatchQueue::main().async(std::forward<F>(func));
    }

    template <typename F>
    void RunOnMainThreadSync(F &&func) {
        DispatchQueue::main().sync(std::forward<F>(func));
    }

    // Game loop callbacks
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnRender() {}
    virtual void OnImGuiRender() {}

    // Timing utilities
    float GetDeltaTime() const {
        return m_deltaTime;
    }
    float GetFPS() const {
        return m_fps;
    }
    float GetAverageDeltaTime() const {
        return m_averageDeltaTime;
    }

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Input> m_input;
    bool m_running;

    // Timing variables
    float m_lastFrameTime;
    float m_deltaTime;
    float m_fps;
    float m_averageDeltaTime;
    float m_frameTimeAccumulator;
    int m_frameCount;

    // Timing configuration
    static constexpr float MAX_DELTA_TIME = 1.0f / 20.0f; // Cap at 20 FPS minimum
    static constexpr float FPS_UPDATE_INTERVAL = 1.0f;    // Update FPS every second

    void CalculateDeltaTime();
};
} // namespace agl

#endif // GAME_H
