#include "agl.h"
#include <thread>
#include <chrono>

class DeltaTimeBenchmark : public agl::Game {
private:
    float m_benchmarkTimer = 0.0f;
    int m_frameStutterTest = 0;
    bool m_simulateStutter = false;

public:
    void OnUpdate(float deltaTime) override {
        m_benchmarkTimer += deltaTime;

        // Simulate frame stutters every 3 seconds
        if (m_benchmarkTimer >= 3.0f && m_simulateStutter) {
            m_frameStutterTest++;
            if (m_frameStutterTest % 10 == 0) { // Every 10th frame during stutter period
                // Simulate a 100ms frame stutter
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                AGL_WARN("Simulated frame stutter: 100ms delay");
            }

            if (m_frameStutterTest >= 30) { // Reset after 30 frames
                m_frameStutterTest = 0;
                m_benchmarkTimer = 0.0f;
                AGL_INFO("Stutter simulation cycle complete");
            }
        }

        // Log frame time statistics
        static float logTimer = 0.0f;
        logTimer += deltaTime;
        if (logTimer >= 2.0f) {
            AGL_INFO("Current Performance - FPS: {:.1f}, Delta: {:.3f}ms, Avg Delta: {:.3f}ms",
                    GetFPS(), deltaTime * 1000.0f, GetAverageDeltaTime() * 1000.0f);
            logTimer = 0.0f;
        }
    }

    void OnImGuiRender() override {
        ImGui::Begin("DeltaTime Benchmark");

        ImGui::Text("DeltaTime Performance Test");
        ImGui::Separator();

        // Current stats
        ImGui::Text("Current FPS: %.1f", GetFPS());
        ImGui::Text("Current Delta: %.3f ms", GetDeltaTime() * 1000.0f);
        ImGui::Text("Average Delta: %.3f ms", GetAverageDeltaTime() * 1000.0f);
        ImGui::Text("Max Delta Cap: %.1f ms", 50.0f); // MAX_DELTA_TIME * 1000

        ImGui::Separator();

        // Controls
        ImGui::Checkbox("Simulate Frame Stutters", &m_simulateStutter);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Enable to test deltaTime clamping with artificial frame drops");
        }

        if (ImGui::Button("Reset Timer")) {
            m_benchmarkTimer = 0.0f;
            m_frameStutterTest = 0;
        }

        ImGui::Separator();

        // Frame time history graph
        static float frameHistory[120] = {0}; // 2 seconds at 60fps
        static int historyIndex = 0;
        frameHistory[historyIndex] = GetDeltaTime() * 1000.0f;
        historyIndex = (historyIndex + 1) % 120;

        ImGui::PlotLines("Frame Times (ms)", frameHistory, 120, historyIndex,
                        nullptr, 0.0f, 50.0f, ImVec2(0, 100));

        // Color-coded performance indicator
        float currentDelta = GetDeltaTime() * 1000.0f;
        if (currentDelta < 16.67f) { // > 60 FPS
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Performance: EXCELLENT");
        } else if (currentDelta < 33.33f) { // > 30 FPS
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Performance: GOOD");
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Performance: POOR");
        }

        ImGui::End();
    }
};

int main() {
    DeltaTimeBenchmark benchmark;

    if (benchmark.Initialize(1200, 800, "DeltaTime Benchmark - AGL Game Engine")) {
        AGL_INFO("Starting DeltaTime Benchmark Application");
        AGL_INFO("This demo shows the improved deltaTime system with clamping and monitoring");
        benchmark.Run();
        AGL_INFO("Benchmark complete");
    } else {
        AGL_ERROR("Failed to initialize benchmark!");
        return -1;
    }

    return 0;
}
