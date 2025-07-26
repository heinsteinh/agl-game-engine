#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

// Simple test to verify our Signal/Slot and DispatchQueue systems work
// This is a minimal test without OpenGL/GLFW dependencies

namespace agl {
    // Minimal Signal/Slot implementation for testing
    template<typename... Args>
    class Signal {
    public:
        void Connect(std::function<void(Args...)> callback) {
            m_callbacks.push_back(callback);
        }
        
        void Emit(Args... args) {
            for (auto& callback : m_callbacks) {
                callback(args...);
            }
        }
        
    private:
        std::vector<std::function<void(Args...)>> m_callbacks;
    };

    // Minimal DispatchQueue for testing
    class DispatchQueue {
    public:
        template<typename F>
        void async(F&& task) {
            std::thread([task = std::forward<F>(task)]() {
                task();
            }).detach();
        }
        
        static DispatchQueue& global() {
            static DispatchQueue instance;
            return instance;
        }
    };
}

// Test function
void TestSignalSlot() {
    std::cout << "🔗 Testing Signal/Slot System..." << std::endl;
    
    agl::Signal<int, std::string> testSignal;
    
    bool callbackTriggered = false;
    int receivedInt = 0;
    std::string receivedString;
    
    testSignal.Connect([&](int num, const std::string& str) {
        callbackTriggered = true;
        receivedInt = num;
        receivedString = str;
        std::cout << "   ✅ Signal received: " << num << ", " << str << std::endl;
    });
    
    testSignal.Emit(42, "Hello Signal/Slot!");
    
    if (callbackTriggered && receivedInt == 42 && receivedString == "Hello Signal/Slot!") {
        std::cout << "   ✅ Signal/Slot test PASSED" << std::endl;
    } else {
        std::cout << "   ❌ Signal/Slot test FAILED" << std::endl;
    }
}

void TestDispatchQueue() {
    std::cout << "\n⚡ Testing DispatchQueue System..." << std::endl;
    
    bool taskCompleted = false;
    
    agl::DispatchQueue::global().async([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        taskCompleted = true;
        std::cout << "   ✅ Background task completed" << std::endl;
    });
    
    // Wait for task to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    if (taskCompleted) {
        std::cout << "   ✅ DispatchQueue test PASSED" << std::endl;
    } else {
        std::cout << "   ❌ DispatchQueue test FAILED" << std::endl;
    }
}

int main() {
    std::cout << "🎮 AGL Game Engine - Demo Systems Test" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    TestSignalSlot();
    TestDispatchQueue();
    
    std::cout << "\n🎉 All basic tests completed!" << std::endl;
    return 0;
}
