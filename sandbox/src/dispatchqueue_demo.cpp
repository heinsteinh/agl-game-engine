#include "agl.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

class DispatchQueueAdvancedDemo : public agl::Game {
private:
    std::unique_ptr<agl::DispatchQueue> m_backgroundQueue;
    std::unique_ptr<agl::DispatchQueue> m_uiQueue;

    // Demo state
    std::atomic<int> m_completedTasks{0};
    std::atomic<int> m_backgroundTasks{0};
    std::vector<std::string> m_taskResults;
    std::mutex m_resultsMutex;

    // Performance metrics
    std::chrono::high_resolution_clock::time_point m_lastUpdate;
    std::atomic<float> m_backgroundLoad{0.0f};

    bool m_showDemo = true;

public:
    bool Initialize(int width = 1024, int height = 768, const char* title = "Advanced DispatchQueue Demo") {
        if (!agl::Game::Initialize(width, height, title)) {
            return false;
        }

        // Create additional dispatch queues for different purposes
        m_backgroundQueue = std::make_unique<agl::DispatchQueue>();
        m_uiQueue = std::make_unique<agl::DispatchQueue>();

        m_lastUpdate = std::chrono::high_resolution_clock::now();

        AGL_INFO("Advanced DispatchQueue demo initialized");

        // Start some background processing
        StartBackgroundProcessing();

        return true;
    }

private:
    void StartBackgroundProcessing() {
        // Start background thread for heavy processing
        std::thread backgroundThread([this]() {
            m_backgroundQueue->run();
        });
        backgroundThread.detach();

        // Queue some initial background tasks
        for (int i = 0; i < 3; ++i) {
            QueueBackgroundTask(i);
        }
    }

    void QueueBackgroundTask(int taskId) {
        m_backgroundQueue->async([this, taskId]() {
            // Simulate heavy computation
            auto start = std::chrono::high_resolution_clock::now();

            std::this_thread::sleep_for(std::chrono::milliseconds(100 + (taskId * 50)));

            // Simulate some work with random results
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 1000);
            int result = dis(gen);

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            // Update results on main thread
            RunOnMainThread([this, taskId, result, duration]() {
                std::lock_guard<std::mutex> lock(m_resultsMutex);
                m_taskResults.push_back(
                    "Task " + std::to_string(taskId) +
                    " completed: " + std::to_string(result) +
                    " (took " + std::to_string(duration.count()) + "ms)"
                );

                // Keep only last 10 results
                if (m_taskResults.size() > 10) {
                    m_taskResults.erase(m_taskResults.begin());
                }

                m_completedTasks++;
                AGL_TRACE("Background task {} completed with result {}", taskId, result);
            });
        });

        m_backgroundTasks++;
    }

    void UpdatePerformanceMetrics() {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUpdate);

        if (elapsed.count() > 1000) { // Update every second
            // Calculate background load (tasks per second)
            m_backgroundLoad = m_backgroundTasks.load() / (elapsed.count() / 1000.0f);
            m_backgroundTasks = 0;
            m_lastUpdate = now;
        }
    }

public:
    void OnUpdate(float deltaTime) override {
        UpdatePerformanceMetrics();
    }

    void OnRender() override {
        glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OnImGuiRender() override {
        if (m_showDemo) {
            ImGui::Begin("Advanced DispatchQueue Demo", &m_showDemo);

            ImGui::Text("Multi-Queue Task Management System");
            ImGui::Separator();

            // Task statistics
            ImGui::Text("Task Statistics:");
            ImGui::Text("Completed Tasks: %d", m_completedTasks.load());
            ImGui::Text("Background Load: %.2f tasks/sec", m_backgroundLoad.load());

            ImGui::Separator();

            // Control buttons
            if (ImGui::Button("Queue Single Background Task")) {
                static int taskCounter = 1000;
                QueueBackgroundTask(taskCounter++);
            }

            ImGui::SameLine();
            if (ImGui::Button("Queue 5 Background Tasks")) {
                static int batchCounter = 2000;
                for (int i = 0; i < 5; ++i) {
                    QueueBackgroundTask(batchCounter++);
                }
            }

            if (ImGui::Button("Queue Priority Main Thread Task")) {
                RunOnMainThread([this]() {
                    AGL_INFO("Priority task executed on main thread!");
                    std::lock_guard<std::mutex> lock(m_resultsMutex);
                    m_taskResults.push_back("PRIORITY: Main thread task completed");
                });
            }

            ImGui::SameLine();
            if (ImGui::Button("Queue Delayed Task")) {
                // Demonstrate delayed execution
                RunOnMainThread([this]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    AGL_INFO("Delayed task completed");
                    std::lock_guard<std::mutex> lock(m_resultsMutex);
                    m_taskResults.push_back("DELAYED: Task completed after 100ms");
                });
            }

            if (ImGui::Button("Stress Test (10 tasks)")) {
                static int stressCounter = 3000;
                for (int i = 0; i < 10; ++i) {
                    QueueBackgroundTask(stressCounter++);
                }
            }

            ImGui::Separator();

            // Task results
            ImGui::Text("Recent Task Results:");
            ImGui::BeginChild("Results", ImVec2(0, 200), true);
            {
                std::lock_guard<std::mutex> lock(m_resultsMutex);
                for (const auto& result : m_taskResults) {
                    ImGui::TextWrapped("%s", result.c_str());
                }
            }
            ImGui::EndChild();

            ImGui::Separator();
            ImGui::Text("Queue Usage Examples:");
            ImGui::BulletText("Background Queue: Heavy computations, file I/O");
            ImGui::BulletText("Main Queue: UI updates, rendering commands");
            ImGui::BulletText("UI Queue: User interface event processing");

            ImGui::Separator();
            ImGui::Text("Features Demonstrated:");
            ImGui::BulletText("Multiple independent dispatch queues");
            ImGui::BulletText("Async task execution with results");
            ImGui::BulletText("Thread-safe communication between queues");
            ImGui::BulletText("Performance monitoring");
            ImGui::BulletText("Priority task scheduling");

            ImGui::End();
        }

        // Performance window
        ImGui::Begin("System Performance");
        ImGui::Text("Frame Rate: %.1f FPS", GetFPS());
        ImGui::Text("Frame Time: %.3f ms", GetDeltaTime() * 1000.0f);
        ImGui::Text("Main Queue Tasks: Processing in game loop");
        ImGui::Text("Background Queue: %.1f tasks/sec", m_backgroundLoad.load());

        // Real-time performance graph
        static float frameTimeHistory[100] = {0};
        static int frameTimeOffset = 0;
        frameTimeHistory[frameTimeOffset] = GetDeltaTime() * 1000.0f;
        frameTimeOffset = (frameTimeOffset + 1) % 100;

        ImGui::PlotLines("Frame Time (ms)", frameTimeHistory, 100, frameTimeOffset,
                        nullptr, 0.0f, 50.0f, ImVec2(0, 80));

        ImGui::End();
    }

    void Shutdown() override {
        // Stop background queue
        if (m_backgroundQueue) {
            m_backgroundQueue->stop();
        }

        agl::Game::Shutdown();
    }
};

int main() {
    DispatchQueueAdvancedDemo game;

    if (game.Initialize(1024, 768, "AGL Engine - Advanced DispatchQueue Demo")) {
        game.Run();
    }

    game.Shutdown();
    return 0;
}
